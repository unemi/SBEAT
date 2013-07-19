#include  <stdio.h>
#include	<string.h>
#include	"decl.h"
#define	msgIdPasteOption	11
typedef	struct {
	Individual	i;
	BarInfo		b;
}	ScrapBar;
static	ViewInfo	ScrapViewInfo;
static	ScrapBar	**ScrapMemHdl = NULL;
static	short		ScrapN = 0;
#define	SizeOfScore	(sizeof(Score) * NTotalParts * NShortBeats)
#define	NHistories	10
typedef	struct {
	HistEventType	type;
	union { SBField *sf; SBIntegrator *si; SBgedit *sg; } sb;
	short	id1, id2;
	FileMode	fmode;
	unsigned char	flag;
	long	memSize;
	Handle	memHdl;
}	HistoryElm;
static	HistoryElm	History[NHistories] = {{ HistEmpty }};
static	short	HistoryIdx = 0, HistorySize = 0;
extern	WindowPtr	TopWindow, PlayWindow;
extern	DialogPtr	playerDlg;

static void check_disable_undo(void) {
	if (HistorySize <= 0) {
		MenuHandle	mh = GetMenuHandle(mEdit);
		DisableMenuItem(mh, iUndo);
//		if (!((*mh)->enableFlags & mEditEnableMask))
//			{ DisableMenuItem(mh, 0); DrawMenuBar(); }
	}
}
void rm_history(WindowPtr win) {
	short	i, j, k, a, b, c;
	if (HistorySize <= 0) return;
	for (i = HistoryIdx, k = HistorySize; k > 0; i = j, k --) {
		if (History[i].sb.si->win == win) {
			if (History[i].memSize >= 0) DisposeHandle(History[i].memHdl);
			if (k > 1) for (a = i, c = k - 1; c > 0; c --, a = b) {
				b = (a + NHistories - 1) % NHistories;
				History[a] = History[b];
			} else b = i;
			History[b].memSize = -1;
			HistorySize --;
			j = i;
		} else j = (i + NHistories - 1) % NHistories;
	}
	check_disable_undo();
}
static HistoryElm *next_history(HistEventType type, void *info, long size) {
	HistoryElm *hist;
	short	idx;
	if (History[0].type == HistEmpty && HistorySize == 0) {
		short	i;
		for (i = 0; i < NHistories; i ++) History[i].memSize = -1;
		idx = 0;
	} else idx = (HistoryIdx + 1) % NHistories;
	hist = History + idx;
	if (size != hist->memSize) {
		OSErr	err;
		if (hist->memSize < 0) hist->memHdl = NewHandle(size);
		else SetHandleSize(hist->memHdl, size);
		if ((err = MemError()) != noErr) {
			error_msg("\pMemory allocation to save history failed.", err);
			return NULL;
		} else hist->memSize = size;
	}
	if (HistorySize < NHistories) if ((++ HistorySize) == 1) {
		MenuHandle	mh = GetMenuHandle(mEdit);
//		if (!((*mh)->enableFlags & 1))
//			{ EnableMenuItem(mh, 0); DrawMenuBar(); }
		EnableMenuItem(mh, iUndo);
	}
	HistoryIdx = idx;
	hist->type = type;
	if (info) {
		HLock(hist->memHdl);
		BlockMove(info, *hist->memHdl, size);
		HUnlock(hist->memHdl);
	}
	return hist;
}
void enque_field_pop_history(SBField *sf) {
	HistoryElm	*h =
	next_history(HistFPop, sf->pop, sizeof(Individual) * PopulationSize);
	if (!h) return;
	h->sb.sf = sf;
	h->fmode = sf->fmode;
}
void enque_field_ind_history(SBField *sf, short id) {
	HistoryElm	*h;
	if (id < 0) return;
	h = next_history(HistFInd, &sf->pop[id], sizeof(Individual));
	if (!h) return;
	h->sb.sf = sf;
	h->id1 = id;
	h->fmode = sf->fmode;
}
void enque_gedit_history(SBgedit *sg) {
	HistoryElm	*h;
	h = next_history(HistGInd, &sg->ind, sizeof(Individual));
	if (!h) return;
	h->sb.sg = sg;
	h->fmode = sg->fmode;
}
void enque_note_shift_history(SBIntegrator *si, integScorePtr scorep) {
	short	i, k, n, *ip;
	HistoryElm	*h;
	Ptr	p;
	if (HistorySize > 0 && History[HistoryIdx].type == HistNoteShift) {
		h = &History[HistoryIdx];
		HLock(h->memHdl); p = *h->memHdl;
		for (n = i = 0; i < si->nBars; i ++) {
			k = (n < h->id1)? ((short *)p)[0] : si->nBars;
			if (k < i) break;
			else if (!ItgSelectedOnP(scorep, i)) { if (k == i) break; }
			else if (k != i) break;
			else { n ++; p += sizeof(short) + SizeOfScore; }
		}
		HUnlock(h->memHdl);
		if (i >= si->nBars || n != h->id1) return;
	} else for (n = i = 0; i < si->nBars; i ++) if (ItgSelectedOnP(scorep, i)) n ++;
	if (n <= 0) return;
	h = next_history(HistNoteShift, NULL, n * (SizeOfScore + sizeof(short)));
	if (!h) return;
	h->sb.si = si;
	h->fmode = si->fmode;
	h->id1 = n;
	HLock(h->memHdl); p = *h->memHdl;
	for (i = 0; n > 0 && i < si->nBars; i ++) if (ItgSelectedOnP(scorep, i)) {
		ip = (short *)p; ip[0] = i; p += sizeof(short);
		BlockMove(scorep[i].i.score[0], p, SizeOfScore);
		p += SizeOfScore;
		n --;
	}
	HUnlock(h->memHdl);
}
static void enque_integ_edit_history(HistEventType type, SBIntegrator *si) {
	integScorePtr	scorep;
	HistoryElm	*h;
	short	i, n, size, *ip;
	Ptr	p;
	HLock((Handle)si->scoreHandle);
	scorep = *(si->scoreHandle);
	for (i = n = 0; i < si->nBars; i ++) if (ItgSelectedP(scorep,i)) n ++;
	switch (type) {
		case HistIBars: size = sizeof(integScoreRec); break;
		case HistIBarInfo: size = sizeof(BarInfo); break;
		default: HUnlock((Handle)si->scoreHandle); return;
	}
	h = next_history(type, NULL, n * (long)(size + sizeof(short)));
	if (!h) { HUnlock((Handle)si->scoreHandle); return; }
	h->type = type;
	h->sb.si = si;
	h->fmode = si->fmode;
	h->id1 = n;
	HLock(h->memHdl); p = *h->memHdl;
	for (i = 0; n > 0 && i < si->nBars; i ++) if (ItgSelectedP(scorep, i)) {
		ip = (short *)p; ip[0] = i; p += sizeof(short);
		switch (type) {
			case HistIBars: BlockMove(&scorep[i], p, size); break;
			case HistIBarInfo: BlockMove(&scorep[i].b, p, size);
		}
		p += size;
		n --;
	}
	HUnlock(h->memHdl);
	HUnlock((Handle)si->scoreHandle);
}
void enque_binfo_history(void) {
	SBField *sf; SBgedit *sg;
	SBIntegrator	*si;
	HistoryElm	*h;
	switch (GetWRefCon(TopWindow)) {
		case WIDField: sf = FindField(TopWindow);
		if (!sf || (HistorySize > 0
		 && History[HistoryIdx].type == HistFBarInfo
		 && History[HistoryIdx].sb.sf == sf)) return;
		h = next_history(HistFBarInfo, &sf->b, sizeof(BarInfo));
		if (!h) return;
		h->sb.sf = sf;
		h->fmode = sf->fmode;
		break;
		case WIDgedit: sg = FindGedit(TopWindow);
		if (!sg || (HistorySize > 0
		 && History[HistoryIdx].type == HistGBarInfo
		 && History[HistoryIdx].sb.sg == sg)) return;
		h = next_history(HistGBarInfo, &sg->b, sizeof(BarInfo));
		if (!h) return;
		h->sb.sg = sg;
		h->fmode = sg->fmode;
		break;
		case WIDIntegrator: si = FindIntegrator(TopWindow);
		if (!si) return;
		h = &History[HistoryIdx];
		if (HistorySize > 0 && h->type == HistIBarInfo && h->sb.si == si) {
			integScorePtr	scorep;
			short	i, n, *ip;
			Ptr	p;
			HLock((Handle)si->scoreHandle);
			scorep = *(si->scoreHandle);
			HLock(h->memHdl); p = *h->memHdl;
			for (i = 0, n = h->id1; i < si->nBars; i ++) if (ItgSelectedP(scorep,i)) {
				if ((-- n) < 0) break;
				ip = (short *)p;
				if (ip[0] != i) break;
				p += sizeof(short) + sizeof(BarInfo);
			}
			HUnlock(h->memHdl);
			HUnlock((Handle)si->scoreHandle);
			if (i >= si->nBars && n == 0) return;
		}
		enque_integ_edit_history(HistIBarInfo, si);
	}
}
static void restore_integ_edit_history(HistoryElm *h) {
	SBIntegrator	*si = h->sb.si;
	integScorePtr	scorep;
	Ptr	p;
	short	i, k;
	SetPort(myGetWPort(si->win));
	HLock((Handle)si->scoreHandle);
	scorep = *(si->scoreHandle);
	HLock(h->memHdl);
	p = *h->memHdl;
	for (i = 0; i < h->id1; i ++) {
		unsigned char	oldFlag;
		k = *((short *)p); p += sizeof(short);
		switch (h->type) {
			case HistIBars: oldFlag = scorep[k].flag;
			BlockMove(p, &scorep[k], sizeof(integScoreRec));
			scorep[k].flag = (scorep[k].flag & ~ItgSelectedFlag) |
				(oldFlag & ItgSelectedFlag);
			p += sizeof(integScoreRec); break;
			case HistIBarInfo: BlockMove(p, &scorep[k].b, sizeof(BarInfo));
			p += sizeof(BarInfo);
		}
		draw_integ_ind(si, k, true);
	}
	HUnlock(h->memHdl);
	HUnlock((Handle)si->scoreHandle);
	si->fmode = h->fmode;
}
void enque_integ_history(HistEventType type, SBIntegrator *si,
	short id1, short id2) {
	integScorePtr	scorep;
	HistoryElm	*h;
	short	size;
	void	*mem;
	HLock((Handle)si->scoreHandle);
	scorep = *(si->scoreHandle);
	switch (type) {
		case HistIShrink: size = sizeof(integScoreRec) * si->nSectionsW;
		mem = &scorep[(si->nSectionsH - 1) * si->nSectionsW]; break;
		default: size = sizeof(integScoreRec);
		mem = &scorep[id1];
	}
	h = next_history(type, mem, size);
	if (!h) { HUnlock((Handle)si->scoreHandle); return; }
	h->sb.si = si;
	h->fmode = si->fmode;
	h->id1 = id1;
	if (type != HistIShrink) {
		h->id2 = id2;
		h->flag = scorep[id1].flag;
	}
	HUnlock((Handle)si->scoreHandle);
}
static void restore_integ_section(HistoryElm *h) {
	SBIntegrator	*si = h->sb.si;
	integScorePtr	scorep;
	Boolean	selp;
	HLock((Handle)si->scoreHandle);
	scorep = *(si->scoreHandle);
	BlockMove(*h->memHdl, &scorep[h->id1], sizeof(integScoreRec));
	scorep[h->id1].flag = h->flag;
	SetPort(myGetWPort(si->win));
	draw_integ_ind(si, h->id1, true);
	selp = ItgSelectedP(scorep, h->id1);
	HUnlock((Handle)si->scoreHandle);
	if (selp) set_current_integ_pi(si);
	si->fmode = h->fmode;
}
static void reexpand_integ(HistoryElm *h) {
	SBIntegrator	*si = h->sb.si;
	integScorePtr	scorep;
	short	i, k;
	if (!expand_integ(si, 1)) return;
	SetPort(myGetWPort(si->win));
	HLock((Handle)si->scoreHandle);
	scorep = *(si->scoreHandle);
	k = (si->nSectionsH - 1) * si->nSectionsW;
	BlockMove(*h->memHdl, &scorep[k], sizeof(integScoreRec) * si->nSectionsW);
	for (i = 0; i < si->nSectionsW; i ++, k ++)
		draw_integ_ind(si, k, true);
	HUnlock((Handle)si->scoreHandle);
	si->fmode = h->fmode;
}
static void restore_integ_scores(HistoryElm *h) {
	SBIntegrator	*si = h->sb.si;
	integScorePtr	scorep;
	Ptr	p;
	short	i, k;
	HLock((Handle)si->scoreHandle);
	scorep = *(si->scoreHandle);
	HLock(h->memHdl);
	p = *h->memHdl;
	SetPort(myGetWPort(si->win));
	for (i = 0; i < h->id1; i ++) {
		k = ((short *)p)[0]; p += sizeof(short);
		if (0 <= k && k < si->nSectionsH * si->nSectionsW)
			BlockMove(p, scorep[k].i.score[0], SizeOfScore);
		p += SizeOfScore;
		draw_integ_ind(si, k, true);
	}
	HUnlock(h->memHdl);
	HUnlock((Handle)si->scoreHandle);
	si->fmode = h->fmode;
}
void UndoCB(void) {
	HistoryElm	*h;
	SBField	*sf; SBgedit *sg;
	SBIntegrator	*si = NULL;
	integScorePtr	scorep;
	WindowRef	win;
	short	i, id = -1;
	if (HistorySize <= 0) return;
	h = History + HistoryIdx;
	switch (h->type) {
		case HistFInd: sf = h->sb.sf;
		SetPort(myGetWPort(sf->win));
		BlockMove(*h->memHdl, &sf->pop[h->id1], sizeof(Individual));
		draw_field_ind(sf, h->id1);
		break;
		case HistFPop: sf = h->sb.sf;
		BlockMove(*h->memHdl, sf->pop, sizeof(Individual)*PopulationSize);
		draw_window(sf);
		break;
		case HistFBarInfo: sf = h->sb.sf;
		BlockMove(*h->memHdl, &sf->b, sizeof(BarInfo));
		devlop_all_score(sf);
		if (sf->win == TopWindow)
			{ setup_partoption_cntrl(); setup_tempo_and_key(); }
		break;
		case HistGInd: sg = h->sb.sg;
		SetPort(myGetWPort(sg->win));
		BlockMove(*h->memHdl, &sg->ind, sizeof(Individual));
		draw_gedit(sg);
		if (sg->win == TopWindow) setup_gedit(sg);
		break;
		case HistGBarInfo: sg = h->sb.sg;
		BlockMove(*h->memHdl, &sg->b, sizeof(BarInfo));
		develop_score(&sg->ind, &sg->b);
		draw_gedit(sg);
		if (sg->win == TopWindow)
			{ setup_partoption_cntrl(); setup_tempo_and_key(); }
		break;
		case HistIBars: case HistIBarInfo:
		restore_integ_edit_history(h);
		si = h->sb.si; break;
		case HistILeft: si = h->sb.si;
		SetPort(myGetWPort(si->win));
		HLock((Handle)si->scoreHandle);
		scorep = *(si->scoreHandle);
		for (i = h->id2; i < h->id1; i ++) {
			scorep[i] = scorep[i + 1];
			draw_integ_ind(si, i, true);
		}
		HUnlock((Handle)si->scoreHandle);
		case HistIOneBar:
		restore_integ_section(h);
		break;
		case HistIRight: si = h->sb.si;
		SetPort(myGetWPort(si->win));
		HLock((Handle)si->scoreHandle);
		scorep = *(si->scoreHandle);
		for (i = h->id2; i > h->id1; i --) {
			scorep[i] = scorep[i - 1];
			draw_integ_ind(si, i, true);
		}
		HUnlock((Handle)si->scoreHandle);
		restore_integ_section(h);
		break;
		case HistIShrink: reexpand_integ(h); break;
		case HistNoteShift: restore_integ_scores(h);
	}
	if (si && si->sel >= 0)
		activate_integ_sel_ctrl(si, ItgOnP(*si->scoreHandle,si->sel), true);
	switch (h->type) {
		case HistFInd: case HistFPop: case HistFBarInfo: win = sf->win; break;
		case HistGInd: case HistGBarInfo: win = sg->win; break;
		default: win = NULL;
	}
	if (win && win == PlayWindow) play_restart();
	HistoryIdx = (HistoryIdx + NHistories - 1) % NHistories;
	-- HistorySize;
	check_disable_undo();
}
static void clear_sel_sec(SBIntegrator *si) {
	extern short	playID;
	integScorePtr	scorep;
	short	i, row, column;
	Boolean	g_modified, row_modified;
	GrafPtr	oldG;
	if (!si) return;
	if (!check_integ_selected_on(si, NULL)) return;
	enque_integ_edit_history(HistIBars, si);
	HLock((Handle)si->scoreHandle);
	scorep = *si->scoreHandle;
	if (si->win == PlayWindow && ItgSelectedOnP(scorep, playID)) stop_tune();
	g_modified = false;
	GetPort(&oldG); SetPort(myGetWPort(si->win));
	for (i = row = 0; row < si->nSectionsH; row ++) {
		row_modified = false;
		for (column = 0; column < si->nSectionsW; column ++, i ++)
		if (ItgSelectedOnP(scorep, i)) {
			scorep[i].flag &= ~ItgOnFlag;
			draw_integ_ind(si, i, true);
			row_modified = true;
		}
		if (row_modified) {
			check_integ_score_head(si, scorep, row);
			g_modified = true;
		}
	}
	HUnlock((Handle)si->scoreHandle);
	if (g_modified) {
		integ_modified(si);
		activate_integ_sel2_ctrl(si);
	}
	SetPort(oldG);
}
static OSErr setup_scrapmem(short n) {
	OSErr	err;
	short	size = sizeof(ScrapBar) * n;
	if (ScrapN == n) return noErr;
	if (ScrapMemHdl) SetHandleSize((Handle)ScrapMemHdl, size);
	else ScrapMemHdl = (ScrapBar **)NewHandle(size);
	if ((err = MemError()) == noErr) ScrapN = n;
	else error_msg("\pNo enough memory.", err);
	return err;
}
static Boolean integ_copy_scrap(SBIntegrator *si) {
	integScorePtr	p;
	ScrapBar	*q;
	short	i, j, nbar, nsel;
	nbar = si->nSectionsH * si->nSectionsW;
	HLock((Handle)si->scoreHandle);
	p = *si->scoreHandle;
	for (i = nsel = 0; i < nbar; i ++) if (ItgSelectedOnP(p,i)) nsel ++;
	HUnlock((Handle)si->scoreHandle);
	if (nsel == 0) return false;
	if (setup_scrapmem(nsel) != noErr) return false;
	HLock((Handle)si->scoreHandle);
	HLock((Handle)ScrapMemHdl);
	p = *si->scoreHandle;
	q = *ScrapMemHdl;
	for (i = j = 0; i < nbar && j < nsel; i ++) if (ItgSelectedOnP(p,i))
		{ q[j].i = p[i].i; q[j].b = p[i].b; j ++; }
	HUnlock((Handle)ScrapMemHdl);
	HUnlock((Handle)si->scoreHandle);
	ScrapViewInfo = si->vInfo;
	setup_edit_menu(true, true, WIDIntegrator);
	return true;
}
void CutCB(void) {
	SBIntegrator *si = FindIntegrator(TopWindow);
	if (si) if (integ_copy_scrap(si)) clear_sel_sec(si);
}
static WindowPtr current_window(void) {
	Point	where;
	short	part;
	WindowPtr	win;
	GetMouse(&where);
	part = FindWindow(where, &win);
	if (!win || part == inDesk || part == inSysWindow) return TopWindow;
	if (GetWRefCon(win) == WIDPlayer) return win;
	return TopWindow;
}
void CopyCB(void) {
	WindowPtr win = current_window();
	SBField	*sf; SBgedit *sg;
	SBIntegrator	*si;
	short	i, j, n;
	ScrapBar	*p;
	if (!win) return;
	switch (GetWRefCon(win)) {
		case WIDPlayer: DialogCopy(playerDlg); break;
		case WIDField: sf = FindField(win); if (!sf) return;
		for (i = n = 0; i < PopulationSize; i ++) if (SelectedP(sf,i)) n ++;
		if (setup_scrapmem(n) != noErr) return;
		HLock((Handle)ScrapMemHdl);
		p = *ScrapMemHdl;
		for (i = j = 0; i < PopulationSize; i ++) if (SelectedP(sf,i))
			{ p[j].i = sf->pop[i]; p[j].b = sf->b; j ++; }
		HUnlock((Handle)ScrapMemHdl);
		ScrapViewInfo = sf->v;
		setup_edit_menu(true, true, WIDField);
		break;
		case WIDgedit: sg = FindGedit(win); if (!sg) return;
		if (setup_scrapmem(1) != noErr) return;
		HLock((Handle)ScrapMemHdl);
		(*ScrapMemHdl)->i = sg->ind;
		(*ScrapMemHdl)->b = sg->b;
		HUnlock((Handle)ScrapMemHdl);
		ScrapViewInfo = sg->v;
		setup_edit_menu(true, true, WIDgedit);
		break;
		case WIDIntegrator: si = FindIntegrator(win);
		if (si) (void)integ_copy_scrap(si);
	}
}
static void paste_field(WindowRef win) {
	short	i, n = ScrapN;
	ScrapBar	*p;
	SBField	*sf = FindField(win);
	if (!sf || n <= 0) return;
	enque_field_pop_history(sf);
	HLock((Handle)ScrapMemHdl);
	p = *ScrapMemHdl;
	for (i = 0; i < PopulationSize && n > 0; i ++)
	if ((sf->sel[i] & (SelectedFlag|ProtectedFlag)) == SelectedFlag) {
		sf->pop[i] = p->i; p ++; n --;
		field_ind_changed(sf, i);
	}
	HUnlock((Handle)ScrapMemHdl);
}
static void paste_integ(WindowRef win) {
	short	i, j, n = ScrapN, row, column;
	Boolean	row_modified, g_modified;
	integScorePtr	scorep;
	ScrapBar	*p;
	SBIntegrator	*si = FindIntegrator(win);
	GrafPtr	oldPort;
	if (!si || n <= 0) return;
	enque_integ_edit_history(HistIBars, si);
	GetPort(&oldPort); SetPort(GetWindowPort(win));
	HLock((Handle)si->scoreHandle);
	scorep = *si->scoreHandle;
	HLock((Handle)ScrapMemHdl);
	p = *ScrapMemHdl;
	g_modified = false;
	for (i = j = row = 0; row < si->nSectionsH && j < ScrapN; row ++) {
		row_modified = false;
		for (column = 0; column < si->nSectionsW && j < ScrapN; column ++, i ++)
		if (ItgSelectedP(scorep, i)) {
			scorep[i].i = p[j].i;
			scorep[i].b = p[j].b;
			scorep[i].flag |= ItgOnFlag;
			row_modified = true;
			j ++;
		}
		if (row_modified) {
			check_integ_score_head(si, scorep, row);
			for (column = 0; column < si->nSectionsW; column ++)
				draw_integ_ind(si, row * si->nSectionsW + column, true);
			g_modified = true;
		}
	}
	if (g_modified) {
		integ_modified(si);
		activate_integ_sel_ctrl(si, true, true);
		activate_integ_sel2_ctrl(si);
		set_current_integ_pi(si);
		if (si->win == PlayWindow && ItgSelectedP(scorep, si->PlayID))
			revise_tune_queue(si->win, si->PlayID);
	}
	HUnlock((Handle)ScrapMemHdl);
	HUnlock((Handle)si->scoreHandle);
	SetPort(oldPort);
}
void PasteCB(void) {
	WindowPtr win = current_window();
	if (!win) return;
	switch (GetWRefCon(win)) {
		case WIDPlayer: DialogPaste(playerDlg);
		player_text_changed();
		break;
		case WIDField: paste_field(win); break;
		case WIDgedit:
		HLock((Handle)ScrapMemHdl);
		paste_gedit(win, &(*ScrapMemHdl)->i, &(*ScrapMemHdl)->b, &ScrapViewInfo);
		HUnlock((Handle)ScrapMemHdl);
		break;
		case WIDIntegrator: paste_integ(win);
	}
}
void ClearCB(void) { clear_sel_sec(FindIntegrator(TopWindow)); }
void SelectAllCB(void) {
	short	i, n;
	SBField	*sf;
	switch (GetWRefCon(TopWindow)) {
		case WIDField: sf = FindField(TopWindow); if (!sf) return;
		SetPort(myGetWPort(sf->win));
		for (i = n = 0; i < PopulationSize; i ++) if (!SelectedP(sf, i))
			{ n ++; flip_sel_mode(sf, i); }
		if (n == 0) for (i = 0; i < PopulationSize; i ++) flip_sel_mode(sf, i);
		setup_field_menu(sf);
		break;
		case WIDIntegrator: integ_select_all(FindIntegrator(TopWindow));
	}
}
enum {
	itemSaveThem = 1,
	itemDontSave,
	itemGroupBox,
	itemCheckBox
};
enum {
	cboxView, cboxPlay, cboxInst, cboxOctave, cboxControl,
	cboxUnitBeat, cboxIteration, cboxProtection, cboxGenePart,
	cboxTempo, cboxKeyScale,
	cpOptNcboxes
};
static pascal Boolean copy_option_filter(DialogPtr dlg, EventRecord *e, short *item) {
	if (StdFilterProc(dlg, e, item)) return true;
	switch (e->what) {
		case nullEvent: check_playing(); check_integ_sel(); break;
		case updateEvt:
		if ((WindowPtr)e->message != GetDialogWindow(dlg))
			DoUpdate(e);
		break;
		case activateEvt:
		if ((WindowPtr)e->message != GetDialogWindow(dlg)) {
			*item = 999; return true;
		} else if (e->modifiers & activeFlag) {
			SetDialogDefaultItem(dlg, itemSaveThem);
			SetDialogCancelItem(dlg, itemDontSave);
		}
	}
	return false;
}
static unsigned long copy_option(unsigned long flagIn, short msgID) {
	static ModalFilterUPP	MyFilter = NULL;
	DialogPtr	dlg;
	ControlHandle	okButton, cancelButton, groupBox;
	unsigned char	message[80], words[40];
	short	i, item;
	unsigned long	flagOut, m;
	ControlHandle	button;
	GrafPtr	oldGP;
	if (flagIn == 0) return 0;
	if (!MyFilter) MyFilter = NewModalFilterUPP(&copy_option_filter);
	GetIndString(message, 130, msgID);
	ParamText(message, NULL, NULL, NULL);
	dlg = GetNewDialog(132, NULL, kMoveToFront);
	SetDialogDefaultItem(dlg, itemSaveThem);
	GetDialogItemAsControl(dlg, itemSaveThem, &okButton);
	GetDialogItemAsControl(dlg, itemDontSave, &cancelButton);
	GetDialogItemAsControl(dlg, itemGroupBox, &groupBox);
	GetIndString(words, 130, msgID + 1);
	SetControlTitle(okButton, words);
	GetIndString(words, 130, msgID + 2);
	SetControlTitle(cancelButton, words);
	GetIndString(words, 130, msgID + 3);
	SetControlTitle(groupBox, words);
	for (i = 0, m = 1; i < cpOptNcboxes; i ++, m <<= 1) {
		GetDialogItemAsControl(dlg, itemCheckBox + i, &button);
		SetControlValue(button, ((flagIn & m) != 0));
		if (flagIn & m) ActivateControl(button);
		else DeactivateControl(button);
	}
	GetPort(&oldGP); SetPort(myGetDPort(dlg));
	for (flagOut = flagIn;;) {
		unsigned long	oldFlag;
		ModalDialog(MyFilter, &item);
		if (itemCheckBox <= item && item < cpOptNcboxes + itemCheckBox) {
			GetDialogItemAsControl(dlg, item, &button);
			item -= itemCheckBox;
			oldFlag = flagOut; m = 1 << item; flagOut ^= m;
			SetControlValue(button, ((flagOut & m) != 0));
			if (flagOut) { if (!oldFlag) ActivateControl(okButton); }
			else if (oldFlag) DeactivateControl(okButton);
		} else if (item < itemCheckBox) break;
	}
	SetPort(oldGP);
	DisposeDialog(dlg);
	return (item == itemSaveThem)? flagOut : 0;
}
static Boolean check_view_changed(ViewInfo *sv, ViewInfo *dv) {
	short	i;
	for (i = 0; i < NDispParts; i ++)
		if (sv->viewPartID[i] != dv->viewPartID[i]) return true;
	return false;
}
typedef	struct	{ void *ps, *pd; short size; }	GEoption;
PasteOptionRet copy_option_f(short msgIndex,
	ViewInfo *sv, BarInfo *sb, ViewInfo *dv, BarInfo *db) {
	short	i;
	unsigned long	flagIn, flagOut, m;
	PasteOptionRet	done = PstOptDone;
	GEoption	option[cpOptNcboxes], *op = option;
	op->ps = sv; op->pd = dv; op->size = sizeof(ViewInfo); op ++;
	op->ps = sb->playOn; op->pd = db->playOn;
		op->size = sizeof(Boolean) * NTotalParts; op ++;
	op->ps = sb->instrument; op->pd = db->instrument;
		op->size = sizeof(long) * (NNoteCh + NDsPercParts); op ++;
	op->ps = sb->octave; op->pd = db->octave; op->size = NNoteCh - 1; op ++;
	op->ps = sb->control[0]; op->pd = db->control[0];
		op->size = NControls * NNoteCh; op ++;
	op->ps = sb->unitBeat; op->pd = db->unitBeat;
		op->size = NTotalParts; op ++;
	op->ps = sb->iteration; op->pd = db->iteration;
		op->size = NTotalParts; op ++;
	op->ps = sb->protect; op->pd = db->protect;
		op->size = NTotalParts; op ++;
	op->ps = sb->genePart[0]; op->pd = db->genePart[0];
		op->size = 3 * NTotalParts; op ++;
	op->ps = &sb->tempo; op->pd = &db->tempo; op->size = 1; op ++;
	op->ps = &sb->keyScale; op->pd = &db->keyScale; op->size = 1; op ++;
	for (i = flagIn = 0, op = option, m = 1; i < cpOptNcboxes; i ++, op ++, m <<= 1)
		if (memcmp(op->ps, op->pd, op->size) != 0) flagIn |= m;
	if (sb->keyNote != db->keyNote) flagIn |= (1<<cboxKeyScale);
	if (flagIn == 0) return PstOptNoNeed;
	if ((flagOut = copy_option(flagIn, msgIndex)) != 0) {
		if (flagOut & (1<<cboxView))
			if (check_view_changed(&ScrapViewInfo, dv)) done = PstOptNeedRedraw;
		if (done != PstOptNeedRedraw && (flagOut & (1<<cboxOctave))) {
			for (i = 0; i < NDispParts; i ++) {
				if (dv->viewPartID[i] >= DsPercPart) break;
				else if (sb->octave[dv->viewPartID[i]] != db->octave[dv->viewPartID[i]])
					{ done = PstOptNeedRedraw; break; }
			}
		}
		if (msgIndex == msgIdPasteOption) enque_binfo_history();
		for (i = 0, m = 1; i < cpOptNcboxes; i ++, m <<= 1)
			if (flagOut & m)
				BlockMove(option[i].ps, option[i].pd, option[i].size);
		if (flagOut & (1<<cboxKeyScale)) db->keyNote = sb->keyNote;
		return done;
	} else return PstOptNothing;
}
static PasteOptionRet paste_option_fg(ViewInfo *v, BarInfo *b) {
	PasteOptionRet	result;
	HLock((Handle)ScrapMemHdl);
	result = copy_option_f(msgIdPasteOption, &ScrapViewInfo, &(*ScrapMemHdl)->b, v, b);
	HUnlock((Handle)ScrapMemHdl);
	if (result == PstOptNoNeed) res_msg(0, NULL, 7, 0);
	return result;
}
static void paste_option_itg(SBIntegrator *si) {
	BarInfo	*sb, *db;
	integScorePtr	dstScore;
	short	i, k;
	unsigned long	flagOut;
	Boolean	needRedraw = false;
	if (!si) return;
	flagOut = copy_option(-1, 11);
	if (!flagOut) return;
	enque_binfo_history();
	if (flagOut & (1<<cboxView)) {
		if (check_view_changed(&si->vInfo, &ScrapViewInfo))
			needRedraw = true;
		si->vInfo = ScrapViewInfo;
	}
	HLock((Handle)si->scoreHandle);
	dstScore = *si->scoreHandle;
	HLock((Handle)ScrapMemHdl);
	for (i = k = 0; i < si->nSectionsH * si->nSectionsW; i ++)
	if (ItgSelectedP(dstScore, i)) {
		sb = &(*ScrapMemHdl)[k].b;
		db = &dstScore[i].b;
		if (flagOut & (1<<cboxPlay))
			BlockMove(sb->playOn, db->playOn, sizeof(Boolean) * NTotalParts);
		if (flagOut & (1<<cboxInst)) {
			BlockMove(sb->instrument, db->instrument, sizeof(long) * NNoteCh);
			BlockMove(sb->drumsInst, db->drumsInst, sizeof(long) * NDsPercParts);
		}
		if (flagOut & (1<<cboxOctave)) BlockMove(sb->octave, db->octave, NNoteCh - 1);
		if (flagOut & (1<<cboxControl))
			BlockMove(sb->control[0], db->control[0], NNoteCh * NControls);
		if (flagOut & (1<<cboxUnitBeat)) BlockMove(sb->unitBeat, db->unitBeat, NTotalParts);
		if (flagOut & (1<<cboxIteration)) BlockMove(sb->iteration, db->iteration, NTotalParts);
		if (flagOut & (1<<cboxProtection)) BlockMove(sb->protect, db->protect, NTotalParts);
		if (flagOut & (1<<cboxGenePart))
			BlockMove(sb->genePart[0], db->genePart[0], 3 * NTotalParts);
		if (flagOut & (1<<cboxTempo)) db->tempo = sb->tempo;
		if (flagOut & (1<<cboxKeyScale))
			{ db->keyScale = sb->keyScale; db->keyNote = sb->keyNote; }
		if (flagOut & ((1<<cboxUnitBeat)|(1<<cboxIteration)|(1<<cboxGenePart)))
			{ develop_score(&dstScore[i].i, db); needRedraw = true; }
		k = (k + 1) % ScrapN;
	}
	HUnlock((Handle)ScrapMemHdl);
	HUnlock((Handle)si->scoreHandle);
	if (needRedraw) update_integrator(si);
	integ_modified(si);
}
void PasteOptionCB(void) {
	extern	BarInfo	*CurrentBI;
	SBField *sf; SBgedit *sg;
	PasteOptionRet	result;
	if (ScrapN == 0 || !CurrentBI) return;
	switch (GetWRefCon(TopWindow)) {
		case WIDField: sf = FindField(TopWindow);
		if (!sf) return;
		result = paste_option_fg(&sf->v, &sf->b);
		if (result == PstOptNeedRedraw || result == PstOptDone)
			devlop_all_score(sf);
		break;
		case WIDgedit: sg = FindGedit(TopWindow);
		if (sg) if (paste_option_fg(&sg->v, &sg->b)
			== PstOptNeedRedraw) draw_gedit(sg);
		break;
		case WIDIntegrator:
		paste_option_itg(FindIntegrator(TopWindow));
		break;
	}
	setup_partoption_cntrl();
}
