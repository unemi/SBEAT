/*
  theScore[i] contains the basic score (0 ... 15)
	ind->score[GuitarPart][i].note = 0 ... 15
	ind->score[BassPart][i].note = 0 ... 17
*/
#define	MorphRECURSIVE	1
#define	MorphDIFFERENCE	0
#define	MorphDIRECT		0
#include	"decl.h"
#include	"score.h"
static	short	scoreIndex = 0, scoreMax = 16, scoreKeyNote = KeyNoteVariable;
static	char	theScore[NShortBeats];

short	delta_note[] =
/*	{-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1};*/
	{-2,-2,-1,-1,-1,-1, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2};
/*	{-4,-3,-3,-2,-2,-1,-1, 0, 0, 1, 1, 2, 2, 3, 3, 4};*/
short	delta_velocity[] =
	{-7,-6,-5,-4,-3,-2,-1, 0, 0, 1, 2, 3, 4, 5, 6, 7};

#if	MorphRECURSIVE
static void new_note(unsigned char info[], short width, short delta[]) {
	short	i, s, w = (width + 1) / 2;
	if (w > 1) new_note(info, w, delta);
	else theScore[scoreIndex ++] = (scoreKeyNote == KeyNoteVariable)?
		(info[0] % (scoreMax / 2)) + (scoreMax / 4) :
		14 - scoreKeyNote;
	for (i = w; i < width; i ++) {
		s = (theScore[scoreIndex - w] % scoreMax) + delta[info[i] & 0x0f];
		if (s < 0) s = 0; else if (s >= scoreMax) s = scoreMax - 1;
		theScore[scoreIndex ++] = s;
	}
}
#endif
#if	MorphDIFFERENCE
static void new_note(unsigned char info[], short width, short delta[]) {
	short	i, s;
	theScore[0] = (scoreKeyNote == KeyNoteVariable)?
		(info[0] % (scoreMax / 2)) + (scoreMax / 4) :
		14 - scoreKeyNote;
	for (i = 1; i < width; i ++) {
		s = theScore[i - 1] + delta[info[i] & 0x0f];
		if (s < 0) s = 0; else if (s >= scoreMax) s = scoreMax - 1;
		theScore[i] = s;
	}
}
#endif
#if	MorphDIRECT
static void new_note(unsigned char info[], short width, short *) {
	short	i, s;
	for (i = 0; i < width; i ++) {
		s = info[i] % scoreMax;
		if (s < 0) s = 0; else if (s >= scoreMax) s = scoreMax - 1;
		theScore[i] = s;
	}
}
#endif

void develop_score(Individual *ind, BarInfo *bi) {
	short	i, j, p, gm, gr, len;
	Boolean	pause[NTotalParts];
	scoreIndex = 0; scoreMax = 16; scoreKeyNote = bi->keyNote;
	new_note(ind->gene[ChordPart], NShortBeats, delta_note);
	for (p = 0; p < NTotalParts; p ++) pause[p] = false;
	for (i = 0; i < NShortBeats; i ++) {
		for (p = 0; p < NTotalParts; p ++) {
			gm = bi->genePart[p][0];
			gr = bi->genePart[p][1];
			j = i % (NShortBeats >> bi->iteration[p]);
			if (i % (1 << bi->unitBeat[p])) ind->score[p][i].note = NoteRemain;
			else if (PauseP(ind->gene[gr][j])) {
				ind->score[p][i].note = pause[p]? NoteRemain : NoteRest;
				pause[p] = true;
			} else if ((j % InhibitRemainBy) != 0 && RemainP(ind->gene[gr][j])) {
				ind->score[p][i].note = NoteRemain;
			} else {
				ind->score[p][i].effect = EffectPart(ind->gene[gm][j]);
				if (p < ChordPart && gm < ChordPart) {
					ind->score[p][i].note = (theScore[j] & 0x0f)
						+ (SoloShiftValue(ind->gene[gm][j]) - 1) * 2;
					if (ind->score[p][i].note < 0) ind->score[p][i].note += 7;
				} else if (p < DsPercPart) {
					ind->score[p][i].note = theScore[j] & 0x0f;
				} else {
					ind->score[p][i].note = ind->gene[gm][j] & 0x1f;
				}
				pause[p] = false;
			}
		}
	}
	for (p = DsPercPart; p < NTotalParts; p ++)
	for (i = len = 0; i < NShortBeats; i ++)
	switch (ind->score[p][i].note) {
		case NoteRest: case NoteRemain: 
		if (len > 0) {
			len ++;
			if (len > 4) { ind->score[p][i].note = NoteRest; len = 0; }
			else ind->score[p][i].note = NoteRemain;
		} else {
			ind->score[p][i].note = (i > 0)? NoteRemain : NoteRest;
		} break;
		default: len = 1;
	}
	for (p = 0; p < NTotalParts; p ++) {
		scoreIndex = 0; scoreMax = VelocityMax;
		scoreKeyNote = KeyNoteVariable;
		len = NShortBeats >> bi->iteration[p];
		new_note(ind->gene[GnFraction + bi->genePart[p][2]],
			len, delta_velocity);
		for (i = 0; i < NShortBeats; i ++)
			ind->score[p][i].velocity = theScore[i % len];
	}
}
void devlop_all_score(SBField *sf) {
	short	i;
	if (!sf) return;
	for (i = 0; i < PopulationSize; i ++)
		develop_score(&sf->pop[i], &sf->b);
	draw_window(sf);
}
void proc_score(Score s[NTotalParts][NShortBeats], BarInfo *pi, ScoreProc proc) {
	short	i, p, k;
	Boolean	flag;
	Score	sc[NTotalParts];
	short	len[NTotalParts], min;
	for (i = 0; i < NShortBeats; i += min) {
		for (p = 0, min = 9999, flag = false; p < NTotalParts; p ++) {
			if (pi->playOn[p]) {
				sc[p] = s[p][i];
				if (sc[p].note != NoteRemain) flag = true;
				for (len[p] = 1, k = i+1; k < NShortBeats; k ++, len[p] ++)
					if (s[p][k].note != NoteRemain) break;
			} else if (i) {
				sc[p].note = NoteRemain; len[p] = NShortBeats - i;
			} else {
				sc[p].note = NoteRest; len[p] = NShortBeats; flag = true;
			}
			if (min > len[p]) min = len[p];
		}
		if (flag) (*proc)(sc, pi, len, min);
	}
}
