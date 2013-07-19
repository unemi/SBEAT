#include  <string.h>
#include	"decl.h"
#define	MutationRate	0.05
extern	WindowPtr	TopWindow;

void pop_init(SBField *sf) {
	short	i = 0, j, k;
	if (TopWindow) switch (GetWRefCon(TopWindow)) {
		case WIDgedit: {
			SBgedit	*sg = FindGedit(TopWindow);
			if (sg) {
				sf->pop[0] = sg->ind;
				sf->b = sg->b; sf->v = sg->v; i = 1;
			}
		} break;
		case WIDIntegrator: {
			SBIntegrator *si = FindIntegrator(TopWindow);
			if (si && si->sel >= 0) {
				HLock((Handle)si->scoreHandle);
				sf->pop[0] = (*si->scoreHandle)[si->sel].i;
				sf->b = (*si->scoreHandle)[si->sel].b;
				sf->v = si->vInfo; i = 1;
				HUnlock((Handle)si->scoreHandle);
			}
		} break;
	}
	for ( ; i < PopulationSize; i ++)
		for (j = 0; j < BytesParNote; j ++)
			for (k = 0; k < NShortBeats; k ++)
				sf->pop[i].gene[j][k] = Random();
	for (i = 0; i < PopulationSize; i ++) {
		sf->sel[i] = 0;
		develop_score(&sf->pop[i], &sf->b);
	}
}
#define byte2part(i)	(i%NTotalParts)
void check_disable_DoItAgain(SBField *sf) {
	short	i;
	if (sf->prevOp != GOpNothing && sf->win == TopWindow) {
		for (i = 0; i < PopulationSize; i ++)
			if (sf->sel[i] & HadSelectedFlag) break;
		if (i >= PopulationSize) {
			sf->prevOp = GOpNothing;
			DisableMenuItem(GetMenuHandle(mBreed), iDoItAgain);
		}
	}
}
void reset_pop(SBField *sf) {
	short	i, j, p;
	unsigned char	mask, cmask;
	Boolean	selChanged = false;
	if (!sf) return;
	enque_field_pop_history(sf);
	for (i = 0; i < PopulationSize; i ++) if (!ProtectedP(sf,i)) {
		for (p = 0; p < BytesParNote; p ++) {
			if (p < GnFraction) switch (sf->b.protect[byte2part(p)] & 0x03) {
				case 0: mask = 0xff; break;	// reset all
				case 1: mask = 0xf0; break; // reset rhythm
				case 2: mask = 0x0f; break; // reset melody
				default: mask = 0;	// no reset
			} else switch ((sf->b.protect[byte2part(p)] >> 2) & 0x03) {
				case 0: mask = 0xff; break; // reset all
				case 1: mask = 0xe0; break; // reset velocity
				default: mask = 0;	// no reset
			}
			if (mask) for (j = 0, cmask = ~mask; j < NShortBeats; j ++)
				sf->pop[i].gene[p][j] = (sf->pop[i].gene[p][j] & cmask)
					| (Random() & mask);
		}
		if (SelectedP(sf, i)) { flip_sel_mode(sf, i); selChanged = true; }
		sf->sel[i] &= ~ HadSelectedFlag;
		develop_score(&sf->pop[i], &sf->b);
		revise_tune_queue(sf->win, i);
	}
	check_disable_DoItAgain(sf);
	field_modified(sf);
	draw_window(sf);
	if (selChanged && sf->win == TopWindow) setup_field_menu(sf);
}
static void mutate_part(unsigned char *p, unsigned char protect, short boundary) {
	short	i, j, jf, jt;
	double	r;
	switch (protect & 3) {
		case 0: jf = 0x80; jt = 0; break;	/* no protection */
		case 1: jf = 0x80; jt = boundary; break;	/* melody protection */
		case 2: jf = boundary; jt = 0; break;	/* rhythm protection */
		default: return;
	}
	for (i = 0; i < NShortBeats; i ++) {
		for (j = jf; j != jt; j >>= 1) {
			r = (Random() + 32767.) / 65534.;
			if (r < MutationRate) p[i] ^= j;
		}
	}
}
static void set_mutant(SBField *sf_p, short parent, SBField *sf_c, short child) {
	short	p;
	if (child != parent || sf_c != sf_p) BlockMove(
		&sf_p->pop[parent].gene[0][0], &sf_c->pop[child].gene[0][0],
		sizeof(Gene)*BytesParNote*NShortBeats);
	for (p = 0; p < GnFraction; p ++)
		mutate_part(sf_c->pop[child].gene[p],
			sf_c->b.protect[byte2part(p)], 0x08);
	for (; p < BytesParNote; p ++)
		mutate_part(sf_c->pop[child].gene[p],
			sf_c->b.protect[byte2part(p)] >> 2, 0x10);
	develop_score(&sf_c->pop[child], &sf_c->b);
	draw_field_ind(sf_c, child);
}
void MutateCB(void) {
	short	id;
	SBField *sf = FindField(TopWindow);
	if (!sf) return;
	for (id = 0; id < PopulationSize; id ++) if (SelectedP(sf, id)) break;
	if (id >= PopulationSize) return;
	enque_field_pop_history(sf);
	SetPort(myGetWPort(TopWindow));
	for ( ; id < PopulationSize; id ++)
	if ((sf->sel[id] & (SelectedFlag|ProtectedFlag)) == SelectedFlag) {
		set_mutant(sf, id, sf, id);
		revise_tune_queue(sf->win, id);
	}
	field_modified(sf);
}
static void set_crossover(SBField *sf, short child,
	Gene father[BytesParNote][NShortBeats], Gene mother[BytesParNote][NShortBeats]) {
	unsigned short	i, m, p;
	for (p = 0; p < BytesParNote; p ++) {
		m = ((unsigned short)Random() %
			((NShortBeats >> sf->b.iteration[p]) - 1)) + 1;
		switch (sf->b.protect[byte2part(p)]) {
			case 0:	/* no protection */
			for (i = 0; i < m; i ++) sf->pop[child].gene[p][i] = father[p][i];
			for (; i < NShortBeats; i ++) sf->pop[child].gene[p][i] = mother[p][i];
			break;
			case 1:	/* melody protection */
			for (i = 0; i < m; i ++) sf->pop[child].gene[p][i] =
				(sf->pop[child].gene[p][i] & 0x0f) | (father[p][i] & 0xf0);
			for (; i < NShortBeats; i ++) sf->pop[child].gene[p][i] =
				(sf->pop[child].gene[p][i] & 0x0f) | (mother[p][i] & 0xf0);
			break;
			case 2:	/* rhythm protection */
			for (i = 0; i < m; i ++) sf->pop[child].gene[p][i] =
				(sf->pop[child].gene[p][i] & 0xf0) | (father[p][i] & 0x0f);
			for (; i < NShortBeats; i ++) sf->pop[child].gene[p][i] =
				(sf->pop[child].gene[p][i] & 0xf0) | (mother[p][i] & 0x0f);
		}
	}
	develop_score(&sf->pop[child], &sf->b);
	draw_field_ind(sf, child);
}
static void crossover(SBField *sf_p, SBField *sf_c) {
	short	i, j, k, n;
	Gene	pgene[PopulationSize][BytesParNote][NShortBeats];
	for (i = n = 0; i < PopulationSize; i ++) if (SelectedP(sf_p, i))
		BlockMove(sf_p->pop[i].gene[0], pgene[n ++][0], BytesParNote*NShortBeats);
	for (i = j = 0; i < PopulationSize; i ++, j = (j + 1) % n)
	if (!(sf_c->sel[i] & (SelectedFlag|ProtectedFlag))) {
		k = (unsigned short)Random() % (n - 1);
		if (k >= j) k ++;
		set_crossover(sf_c, i, pgene[j], pgene[k]);
	}
}
void next_generation(SBField *sf_p, SBField *sf_c) {
	short	i, id, np, ns;
	np = ns = 0;
	if (!sf_p) return;
	if (!sf_c) sf_c = sf_p;
	for (i = 0; i < PopulationSize; i ++) {
		if (SelectedP(sf_p, i)) { ns ++; id = i; }
		if (!(sf_c->sel[i] & (SelectedFlag|ProtectedFlag))) np ++;
	}
	if (ns <= 0 || np <= 0) return;
	if (sf_p == sf_c) enque_field_pop_history(sf_c);
	SetPort(myGetWPort(sf_c->win));
	if (ns == 1) {
		for (i = 0; i < PopulationSize; i ++)
			if (!(sf_c->sel[i] & (SelectedFlag|ProtectedFlag)))
				set_mutant(sf_p, id, sf_c, i);
	} else crossover(sf_p, sf_c);
	for (i = 0; i < PopulationSize; i ++) {
		if (SelectedP(sf_p, i)) {
			flip_sel_mode(sf_p, i);
			sf_p->sel[i] |= HadSelectedFlag;
		} else sf_p->sel[i] &= ~ HadSelectedFlag;
	}
	sf_p->prevOp = (sf_p == sf_c)? GOpNextInThis : GOpNextInNew;
	if (sf_p == sf_c) field_modified(sf_c);
	setup_field_menu(sf_p);
}
void DoItAgainCB(void) {
	short	i;
	SBField	*sf = FindField(TopWindow);
	if (!sf) return;
	if (sf->prevOp == GOpNothing) return;
	for (i = 0; i < PopulationSize; i ++)
		if (sf->sel[i] & HadSelectedFlag) sf->sel[i] |= SelectedFlag;
	switch (sf->prevOp) {
		case GOpNextInThis: next_generation(sf, NULL); break;
		case GOpNextInNew: next_in_new(sf); break;
		case GOpMutateIt: MutateCB();
	}
}
