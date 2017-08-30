/**
  SBEAT: A classic MacOS 9 app of breeding tool for musical composition
  developed by T. Unemi in 2001
  Copyright (C) 2013  unemi

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**/


#include  <stdio.h>
#include	<string.h>
#include	"decl.h"
#include	"score.h"
#include	"dialog.h"
#include	"timbre.h"
#include	"vers.h"

#define	MallocUnit	16

SBgedit	*Gedits = NULL, *FreeGedits = NULL;
DialogPtr	geditDlg = NULL;
static unsigned char	solo_label[] = "-0+";
static char	current_velocity[NShortBeats];
static short	nTimbres;
static unsigned char	timbreCandidates[32];

extern	WindowPtr	TopWindow, PlayWindow;
extern	short	delta_note[], delta_velocity[];

static SBgedit *new_gedit_rec(void) {
	SBgedit	*sg;
	if (!FreeGedits) {
		short	i;
		FreeGedits = (SBgedit *)NewPtr(sizeof(SBgedit) * MallocUnit);
		if (!FreeGedits) {
			error_msg("\pNewPtr in new_gedit", MemError());
			return NULL;
		}
		for (i = 1; i < MallocUnit; i ++)
			FreeGedits[i - 1].post = FreeGedits + i;
		FreeGedits[MallocUnit - 1].post = NULL;
	}
	sg = FreeGedits;
	FreeGedits = FreeGedits->post;
	sg->post = Gedits;
	if (Gedits) Gedits->pre = sg;
	Gedits = sg;
	sg->pre = NULL;
	return sg;
}
static void free_gedit(SBgedit *sg) {
	if (sg->pre) sg->pre->post = sg->post;
	else if (sg == Gedits) Gedits = sg->post;
	if (sg->post) sg->post->pre = sg->pre;
}
SBgedit *FindGedit(WindowPtr win) {
	SBgedit	*sg;
	for (sg = Gedits; sg; sg = sg->post)
		if (sg->win == win) return sg;
	return NULL;
}
static unsigned char bin2hex(unsigned char a) {
	if (a < 10) return '0' + a;
	else return 'A' - 10 + a;
}
static void set_rhythm_icon(ControlRef button, short id) {
	ControlButtonContentInfo	cinfo;
	cinfo.contentType = kControlContentCIconRes;
	cinfo.u.resID = id;
	SetControlData(button, kControlEntireControl,
		kControlBevelButtonContentTag, sizeof(cinfo), &cinfo);
	SetControlReference(button, id);
	DrawOneControl(button);
}
static void set_melody_shift_txt(short k, unsigned char *txt) {
	if (k == 0) { txt[0] = 1; txt[1] = '0'; }
	else if (k < 0) { txt[0] = 2; txt[1] = '-'; txt[2] = '0' - k; }
	else { txt[0] = 2; txt[1] = '+'; txt[2] = '0' + k; }
}
static void set_gedit_box_title(short itemID, short textID, short chrID) {
	ControlRef	control;
	unsigned char	txt[48], chrTxt[16];
	GetIndString(chrTxt, 132, 4);
	chrTxt[chrTxt[0] + 1] = '\0';
	GetIndString(txt, 132, textID);
	sprintf((char *)txt + txt[0] + 1, " (%s #%d)", chrTxt + 1, chrID + 1);
	txt[0] = strlen((char *)txt + 1);
	GetDialogItemAsControl(geditDlg, itemID, &control);
	SetControlTitle(control, txt);
}
Boolean setup_gedit(SBgedit *sg) {
	ControlRef	control;
	Gene	*gene;
	short	epart, gm, gr, gv, i, textID;
	unsigned char	txt[32];
	GetDialogItemAsControl(geditDlg, GEsaveID, &control);
	if (sg->fmode == ModifiedFile) ActivateControl(control);
	else DeactivateControl(control);
	GetDialogItemAsControl(geditDlg, GEmenuID, &control);
	if (!control) return false;
	epart = sg->editPart;
	if (epart < 0 || epart >= NTotalParts) return false;
	SetControlValue(control, epart + 1);
	gm = sg->b.genePart[(epart == ChordPart + 1)? ChordPart : epart][0];
	gr = sg->b.genePart[epart][1];
	gv = sg->b.genePart[epart][2];
	gene = sg->ind.gene[gm];
	if (epart >= DsPercPart) {
		extern	unsigned char	*timbreList[];
		long	k, mask, dsInst = sg->b.drumsInst[epart - DsPercPart];
		nTimbres = timbreList[dsInst & 0xff][0];
		for (i = 0, k = 0, mask = 0x100; i < nTimbres; i ++, mask <<= 1)
			if (!(mask & dsInst)) timbreCandidates[++ k] = i;
		timbreCandidates[0] = k;
		textID = 3;
		for (i = 0; i < NShortBeats; i ++) {
			GetDialogItemAsControl(geditDlg, GEmelodyID+i, &control);
			txt[0] = 1; txt[1] = bin2hex(get_timbre_id(epart, gene[i] & 0x1f, &sg->b));
			SetControlTitle(control, txt);
			if (k <= 1) DeactivateControl(control);
			else ActivateControl(control);
		}
	} else if (epart >= ChordPart) {
		short	k;
		textID = 2;
		GetDialogItemAsControl(geditDlg, GEmelodyID, &control);
		set_key_note_txt(gene[0], txt);
		SetControlTitle(control, txt);
		if (sg->b.keyNote != KeyNoteVariable) DeactivateControl(control);
		for (i = 1; i < NShortBeats; i ++) {
			GetDialogItemAsControl(geditDlg, GEmelodyID+i, &control);
			ActivateControl(control);
			k = delta_note[gene[i] & 0x0f];
			set_melody_shift_txt(k, txt);
			SetControlTitle(control, txt);
		}
	} else {
		textID = 1;
		for (i = 0; i < NShortBeats; i ++) {
			GetDialogItemAsControl(geditDlg, GEmelodyID+i, &control);
			ActivateControl(control);
			txt[0] = 1;
			txt[1] = solo_label[SoloShiftValue(gene[i])];
			SetControlTitle(control, txt);
		}
	}
	gene = sg->ind.gene[gr];
	for (i = 0; i < NShortBeats; i ++) {
		long	v = (PauseP(gene[i]))? IcnRhythmRest :
			((RemainP(gene[i]))? IcnRhythmCont : IcnRhythmNote);
		if ((i % InhibitRemainBy) == 0 && v == IcnRhythmCont) v = IcnRhythmNote;
		GetDialogItemAsControl(geditDlg, GErhythmID+i, &control);
		SetControlReference(control, v);
		set_rhythm_icon(control, v);
	}
	gene = sg->ind.gene[GnFraction + gv];
	for (i = 0; i < NShortBeats; i ++) {
		long	v;
		if (i == 0) v = gene[i] & 0x1f;
		else v = delta_velocity[gene[i] & 0x0f];
		GetDialogItemAsControl(geditDlg, GEvelocitySlider+i, &control);
		SetControlValue(control, v);
		NumToString(v, txt);
		GetDialogItemAsControl(geditDlg, GEvelocityText+i, &control);
		SetDialogItemText((Handle)control, txt);
	}
	set_gedit_box_title(GEmelodyTitleText, textID, gm);
	set_gedit_box_title(GErhythmTitleText, 5, gr);
	set_gedit_box_title(GEvelocityTitleText, 6, gv);
	return true;
}
static void get_win_size(WindowPtr win, Point *size, Point *leftTop) {
	Rect	rct;
	RgnHandle	rgn = NewRgn();
	GetWindowRegion(win, kWindowStructureRgn, rgn);
	GetRegionBounds(rgn, &rct);
	size->h = rct.right - rct.left;
	size->v = rct.bottom - rct.top;
	SetPort(myGetWPort(win));
	leftTop->h = leftTop->v = 0;
	LocalToGlobal(leftTop);
	leftTop->h -= rct.left;
	leftTop->v -= rct.top;
	DisposeRgn(rgn);
}
static void position_gedit_dlg(WindowPtr win) {
	static	Point	dialogSize = {0, 0}, dialogLeftTop;
	static	Point	windowSize, windowLeftTop;
	Rect	rct;
	Point	p;
	short	x, y;
	extern	Boolean	on_MacOS_X;
	if (dialogSize.v == 0) {
		get_win_size(GetDialogWindow(geditDlg), &dialogSize, &dialogLeftTop);
		get_win_size(win, &windowSize, &windowLeftTop);
	} else SetPort(myGetWPort(win));
	p.h = p.v = 0;
	LocalToGlobal(&p);
	rct = (*GetGDevice())->gdRect;
	x = p.h - windowLeftTop.h + windowSize.h/2 - dialogSize.h/2;
	if (on_MacOS_X) InsetRect(&rct, 2, 2);
	if (x < rct.left) x = rct.left;
	else if (x + dialogSize.h > rct.right) x = rct.right - dialogSize.h;
	y = p.v - windowLeftTop.v + windowSize.v;
	if (on_MacOS_X) y += 2;
	if (y + dialogSize.v > rct.bottom)
		y = p.v - windowLeftTop.v - dialogSize.v;
	MoveWindow(GetDialogWindow(geditDlg),
		x + dialogLeftTop.h, y + dialogLeftTop.v, false);
}
void open_gedit(void) {
	static short	WinID = 0;
	SBgedit	*sg;
	SBField	*sf = NULL;
	SBIntegrator	*si;
	short	id, k;
	unsigned char	title[31];
	switch (GetWRefCon(TopWindow)) {
		case WIDField: sf = FindField(TopWindow); if (!sf) return;
		if ((id = find_primary(sf)) < 0) return;
		break;
		case WIDIntegrator: si = FindIntegrator(TopWindow); if (!si) return;
		if ((id = si->sel) < 0) return;
		break;
		default: return;
	}
	if (!(sg = new_gedit_rec())) return;
	sg->win = GetNewCWindow(131, nil, kMoveToFront);
	if (!sg->win) {
		error_msg("\pGetNewCWindow in open_gedit", 0);
		free_gedit(sg); return;
	}
	SetWRefCon(sg->win, WIDgedit);
	GetWTitle(sg->win, title);
	WinID ++; NumToString(WinID, &title[k = title[0]+1]);
	title[0] = k + title[k]; title[k] = ' ';
	SetWTitle(sg->win, title);
	sg->sf = sf;
	sg->si = si;
	sg->id = id;
	sg->fmode = NotModifiedFile;
	SetWindowModified(sg->win, false);
	if (sf) {
		sg->ind = sf->pop[id];
		sg->b = sf->b; sg->v = sf->v;
	} else {
		sg->ind = (*si->scoreHandle)[si->sel].i;
		sg->b = (*si->scoreHandle)[si->sel].b;
		sg->v = si->vInfo;
	}
	sg->playing = false;
	for (id = 0; id < NTotalParts; id ++)
		if (sg->v.viewOn[id] && id != sg->v.viewQue[NDispParts - 1]) break;
	sg->editPart = (id < NTotalParts)? id : 0;
	set_win_menu_item(sg->win);
}
static void set_gedit_saved(SBgedit *sg) {
	ControlRef	button;
	sg->fmode = NotModifiedFile;
	SetWindowModified(sg->win, false);
	DisableMenuItem(GetMenuHandle(mFile), iSaveFile);
	GetDialogItemAsControl(geditDlg, GEsaveID, &button);
	DeactivateControl(button);
	SetWindowModified(sg->win, false);
}
static Boolean save_gedit_in_new_field(SBgedit *sg) {
	SBField	*sf;
	WindowPtr	oldTop;
	switch (save_change_alert(sg->win, 5)) {
		case itemCancel: return false;
		case itemOK: sf = new_field_rec();
		if (sf) {
			sf->b = sg->b; sf->v = sg->v;
			oldTop = TopWindow; TopWindow = sg->win;
			pop_init(sf);
			TopWindow = oldTop;
			setup_new_field(sf, NULL);
			sg->sf = sf;
			set_gedit_saved(sg);
		}
	}
	return true;
}
void close_gedit(SBgedit *sg) {
	WindowPtr	win = sg->win;
	if (!sg) return;
	if (sg->fmode == ModifiedFile) {
		if (sg->sf) switch (save_change_alert(win, 3)) {
			case itemCancel: return;
			case itemOK: if (!save_gedit_option(sg)) return;
		} else if (!save_gedit_in_new_field(sg)) return;
	}
	activate_gedit(win, false);
	sg->win = NULL;
	free_gedit(sg);
	close_sb_window(win);
}
void update_gedit(WindowPtr win) {
	SBgedit	*sg = FindGedit(win);
	if (!sg) return;
	SetPort(myGetWPort(win));
	draw_gedit(sg);
}
void click_gedit(SBgedit *sg, EventRecord *e) {
	Rect	rct;
	short	part;
	Point	p = e->where;
	if (!sg) return;
	SetPort(myGetWPort(sg->win)); GlobalToLocal(&p);
	if (p.h < WinLeftSpace) { SelectWindow(sg->win); return; }
	set_my_cursor(CURSclosed);
	if (e->modifiers & optionKey) part = p.v / PartHeight;
	else part = -1;
	gedit_part_to_rect(part, &rct);
	if (draw_temp_drag_frame(&rct, e->where))
		drag_gedit_ind(sg, e);
	else SelectWindow(sg->win);
	set_my_cursor(CURSgrabber);
}
void switch_GEplay_button(Boolean activatePlay) {
	ControlRef	play, stop;
	GetDialogItemAsControl(geditDlg, GEplayID, &play);
	GetDialogItemAsControl(geditDlg, GEstopID, &stop);
	if (activatePlay) {
		ActivateControl(play); DeactivateControl(stop);
	} else {
		ActivateControl(stop); DeactivateControl(play);
	}
}
MenuHandle get_gedit_part_menu(void) {
	ControlRef	ch;
	MenuHandle	mh;
	long	asize;
	GetDialogItemAsControl(geditDlg, GEmenuID, &ch);
//	return GetControlPopupMenuHandle(ch);	// doesn't work in Carbon1.4
	GetControlData(ch, kControlMenuPart, kControlGroupBoxMenuHandleTag,
		sizeof(mh), &mh, &asize);
	return mh;
}
#ifdef	JAPANESE_VERSION
#define	SaveBtnFld	"\ptB[hÉÛ¶"
#define	SaveBtnScr	"\pXRAÉÛ¶"
#else
#define	SaveBtnFld	"\pSave to the field"
#define	SaveBtnScr	"\pSave to the score"
#endif
static void open_gedit_dlg(SBgedit *sg) {
	short	i;
	ControlRef	ch;
	MenuHandle	mh;
	extern	Boolean	on_MacOS_X;
	if (!geditDlg) {
		geditDlg = GetNewDialog(131, NULL, kMoveToFront);
		if (!geditDlg) { error_msg("\pGetNewDialog", 129); return; }
		SetWRefCon(GetDialogWindow(geditDlg), WIDgeditDlg);
		GetDialogItemAsControl(geditDlg, GEvelocitySlider, &ch);
		SetControlMaximum(ch, VelocityMax / 2 - 1);
		SetControlMinimum(ch, 0);
		for (i = 1; i < NShortBeats; i ++) {
			GetDialogItemAsControl(geditDlg, GEvelocitySlider + i, &ch);
			SetControlMaximum(ch, 7);
			SetControlMinimum(ch, -7);
		}
		if (on_MacOS_X) {
			set_small_font(geditDlg, GEmenuID, 1, teJustLeft);
			set_small_font(geditDlg, GErhythmTitleText, nGEtitleText, teJustLeft);
			set_small_font(geditDlg, GEvelocityText, NShortBeats, teCenter);
		}
	} else activateRootCntl(geditDlg);
	mh = get_gedit_part_menu();
	if (!mh) { error_msg("\pNo menu in setup_all_part_menu_items", 0); return; }
	for (i = 0; i < NTotalParts; i ++) setup_part_menu_item(i, mh);
	GetDialogItemAsControl(geditDlg, GEsaveID, &ch);
	SetControlTitle(ch, (sg->sf? SaveBtnFld : SaveBtnScr));
	setup_gedit(sg);
	position_gedit_dlg(sg->win);
	switch_GEplay_button(!sg->playing);
	BringToFront(GetDialogWindow(geditDlg));
	ShowWindow(GetDialogWindow(geditDlg));
}
static void setup_gedit_menu(SBgedit *sg) {
	MenuRef	mh;
	mh = GetMenuHandle(mFile);
	if (sg->fmode == ModifiedFile) EnableMenuItem(mh, iSaveFile);
	else DisableMenuItem(mh, iSaveFile);
	DisableMenuItem(mh, iSaveAs);
	EnableMenuItem(mh, iSaveOption);
	DisableMenuItem(mh, iReset);
	setup_edit_menu(true, true, WIDgedit);
	DisableMenuItem(GetMenuHandle(mBreed), 0);
	DrawMenuBar();
}
void activate_gedit(WindowPtr win, Boolean activate) {
	SBgedit	*sg = FindGedit(win);
	if (!sg) return;
	if (activate) {
		if (is_checked(iPartOption)) open_part_option();
		if (is_checked(iPlayer)) open_player();
		set_activate_count(&sg->act);
		activate_poption_items(true);
		setup_current_po(&sg->b, &sg->v);
		open_gedit_dlg(sg);
		setup_gedit_menu(sg);
		activate_protection(false);
	} else if (geditDlg) HideWindow(GetDialogWindow(geditDlg));
}
void paste_gedit(WindowPtr win, Individual *ind, BarInfo *bi, ViewInfo *vi) {
	SBgedit	*sg = FindGedit(win);
	if (!sg) return;
	enque_gedit_history(sg);
	sg->ind = *ind;
	sg->b = *bi;
	sg->v = *vi;
	setup_partoption_cntrl();
	setup_tempo_and_key();
	update_gedit(win);
	setup_gedit(sg);
	play_restart();
	gedit_modified(sg);
}
Boolean save_gedit_option(SBgedit *sg) {
	SBField	*sf = sg->sf;
	SBIntegrator	*si = sg->si;
	PasteOptionRet	done;
	if (sg->fmode == NotModifiedFile) return false;
	if (sf) {
		enque_field_ind_history(sf, sg->id);
		sf->pop[sg->id] = sg->ind;
		if (copy_option_f(7, &sg->v, &sg->b, &sf->v, &sf->b)
			== PstOptDone) devlop_all_score(sf);
		else {
			develop_score(&sf->pop[sg->id], &sf->b);
			draw_window(sf);
		}
		field_modified(sf);
	} else if (si) {
		integScorePtr	scorep;
		GrafPtr	oldPort;
		enque_integ_history(HistIOneBar, si, sg->id, 0);
		HLock((Handle)(si->scoreHandle));
		scorep = *si->scoreHandle;
		scorep[sg->id].i = sg->ind;
		done = copy_option_f(18, &sg->v, &sg->b, &si->vInfo, &scorep[sg->id].b);
		develop_score(&scorep[sg->id].i, &scorep[sg->id].b);
		GetPort(&oldPort); SetPort(GetWindowPort(si->win));
		draw_sel_frame(si, false);
		if (done == PstOptDone) draw_integ_ind(si, sg->id, true);
		else draw_integ_scores(si);
		draw_sel_frame(si, true);
		SetPort(oldPort);
		HUnlock((Handle)(si->scoreHandle));
		integ_modified(si);
	} else return save_gedit_in_new_field(sg);
	set_gedit_saved(sg);
	return true;
}
static void revise_gedit_score(SBgedit *sg, Boolean redrawFlag) {
	develop_score(&sg->ind, &sg->b);
	if (redrawFlag) update_gedit(sg->win);
	play_restart();
	gedit_modified(sg);
}
static void click_rhythm_button(SBgedit *sg, dialogSelectRec *r) {
	ControlButtonContentInfo	cinfo;
	Gene	*gene, rhythmGene;
	long	oldValue, newValue;
	Boolean	redrawFlag;
	short	gr, i, id = r->itemID - GErhythmID;
	enque_gedit_history(sg);
	oldValue = GetControlReference(r->control);
	if (id % InhibitRemainBy)
		newValue = ((oldValue - IcnRhythmCont + 1) % 3) + IcnRhythmCont;
	else newValue = (oldValue == IcnRhythmRest)? IcnRhythmNote : IcnRhythmRest;
	cinfo.contentType = kControlContentCIconRes;
	cinfo.u.resID = newValue;
	SetControlData(r->control, kControlEntireControl,
		kControlBevelButtonContentTag, sizeof(cinfo), &cinfo);
	DrawOneControl(r->control);
	SetControlReference(r->control, newValue);
	gr = sg->b.genePart[sg->editPart][1];
	gene = sg->ind.gene[gr];
	switch (newValue) {
		case IcnRhythmCont: rhythmGene = (Random() & 0x60) | 0x80; break;
		case IcnRhythmRest: rhythmGene = 0x60; break;
		case IcnRhythmNote: rhythmGene = ((unsigned short)Random() % 3) << 5;
	}
	gene[id] = (gene[id] & 0x1f) | rhythmGene;
	if (oldValue == IcnRhythmNote || newValue == IcnRhythmNote) redrawFlag = true;
	else for (redrawFlag = true, i = id - 1; i >= 0; i --) {
		if (PauseP(gene[i])) { redrawFlag = false; break; }
		else if ((i % InhibitRemainBy) == 0 || !RemainP(gene[i]))
			{ redrawFlag = true; break; }
	}
	revise_gedit_score(sg, redrawFlag);
}
static void set_velocity_value(SBgedit *sg, dialogSelectRec *r, long value, Boolean flag) {
	ControlRef	text;
	short	id = r->itemID - GEvelocitySlider;
	unsigned char	txt[16];
	Gene	*gene = &sg->ind.gene[GnFraction + sg->editPart][id];
	enque_gedit_history(sg);
	if (flag) SetControlValue(r->control, value);
	GetDialogItemAsControl(geditDlg, GEvelocityText + id, &text);
	NumToString(value, txt);
	SetDialogItemText((Handle)text, txt);
	if (id == 0) *gene = (*gene & 0xe0) | (value & 0x1f);
	else {
		if (value < 0) value += 7;
		else if (value > 0) value += 8;
		else value = (Random() & 1) + 7;
		*gene = (*gene & 0xf0) | (value & 0x0f);
	}
}
static void click_velocity_slider(SBgedit *sg, dialogSelectRec *r) {
	long	value;
	value = GetControlValue(r->control);
	switch (r->partCode) {
		case kControlPageDownPart:
		if (value > 0) set_velocity_value(sg, r, value - 1, true);
		break;
		case kControlPageUpPart:
		if (value < 63) set_velocity_value(sg, r, value + 1, true);
		break;
		default: set_velocity_value(sg, r, value, false);
	}
	revise_gedit_score(sg, false);
}
static void click_note_shift_button(SBgedit *sg, dialogSelectRec *r) {
	Gene	*gene, msGene;
	short	newValue, gm, id = r->itemID - GEmelodyID;
	unsigned char	txt[4];
	enque_gedit_history(sg);
	gm = sg->b.genePart[sg->editPart][0];
	gene = &sg->ind.gene[gm][id];
	newValue = (SoloShiftValue(*gene) + 1) % 3;
	txt[0] = 1;
	txt[1] = solo_label[newValue];
	SetControlTitle(r->control, txt);
	msGene = ((unsigned short)Random() % ((newValue < 2)? 3 : 2)) * 3 + newValue;
	*gene = (*gene & 0xe0) | msGene;
	revise_gedit_score(sg, true);
}
void check_gedit_dlg(EventRecord *e) {
	dialogSelectRec	r;
	short	newValue;
	SBgedit	*sg = FindGedit(TopWindow);
	extern	Boolean	PlayAll;
	if (!dialog_select(e, geditDlg, &r)) return;
	if (!sg) return;
	switch (r.itemID) {
		case GEmenuID: newValue = GetControlValue(r.control) - 1;
		if (newValue != sg->editPart) {
			sg->editPart = newValue;
			setup_gedit(sg);
			if (!sg->v.viewOn[sg->editPart]) click_display_sw(sg->editPart);
			else {
				if (sg->v.viewQue[NDispParts - 1] == newValue)
					click_display_sw(sg->v.viewQue[NDispParts - 2]);
				SetPort(myGetWPort(sg->win)); draw_gedit(sg);
			}
		} break;
		case GEplayID: sg->playing = true;
		switch_GEplay_button(false);
		PlayAll = false;
		play_sequence(TopWindow, 0); break;
		case GEstopID: sg->playing = false;
		switch_GEplay_button(true);
		stop_tune(); break;
		case GEsaveID: save_gedit_option(sg); break;
		default:
		if (r.itemID >= GErhythmID && r.itemID < GErhythmID+NShortBeats)
			click_rhythm_button(sg, &r);
		else if (r.itemID >= GEvelocitySlider && r.itemID < GEvelocitySlider+NShortBeats)
			click_velocity_slider(sg, &r);
		else if (sg->editPart < ChordPart)
			click_note_shift_button(sg, &r);
	}
}
static short popup_menu_select(
	MenuRef *mhp, short menuID, short oldValue, long mask, ControlRef button) {
	MenuRef	mh;
	Rect	rect;
	Point	pt;
	short	i, n, newValue;
	long	mm;
	static	short	oldKeyScale = -999;
	extern	Boolean	on_MacOS_X;
	if (!*mhp) {
		*mhp = mh = GetMenu(menuID);
		if (!mh) { error_msg("\pGetMenu in popup_menu_select", menuID); return -1; }
		InsertMenu(mh, -1);
		SetMenuFont(mh, 0, (on_MacOS_X? SmallFontSizeX : SmallFontSize9));
	} else mh = *mhp;
	switch (menuID) {
		case mKeyNote:
		setup_key_note_menu(mh, 1, &oldKeyScale); break;
	}
	n = CountMenuItems(mh);
	for (i = 1, mm = 1; i <= n; i ++, mm <<= 1) {
		CheckMenuItem(mh, i, (i == oldValue+1));
		if (mask >= 0) {
			if (mask & mm) DisableMenuItem(mh, i);
			else EnableMenuItem(mh, i);
		}
	}
	HiliteControl(button, kControlButtonPart);
	GetControlBounds(button, &rect);
	pt.h = rect.right; pt.v = rect.top;
	SetPort(myGetDPort(geditDlg)); LocalToGlobal(&pt);
	newValue = LoWord(PopUpMenuSelect(mh, pt.v, pt.h, oldValue + 1)) - 1;
	if (oldValue == newValue) newValue = -1;
	HiliteControl(button, 0);
	return newValue;
}
static void click_drums_button(SBgedit *sg, Gene *gene, ControlRef button) {
	short	oldValue, newValue;
	unsigned char	txt[4];
	oldValue = get_timbre_id(sg->editPart, gene[0] & 0x1f, &sg->b);
	if (timbreCandidates[0] < 4) {
		short	i;
		for (i = 1; i <= timbreCandidates[0]; i ++)
			if (oldValue <= timbreCandidates[i]) break;
		newValue = timbreCandidates[(i % timbreCandidates[0]) + 1];
	} else {
		long	k, dsInst = sg->b.drumsInst[sg->editPart - DsPercPart];
		static	MenuRef	menu[3] = { NULL, NULL, NULL };
		static	short	menuID[3] = { mDrumStickTimbres, mTomTomTimbres, mCrashTimbres };
		static	short	setID[3] = { diDsStick, diTomTom, diCrashCymbal };
		for (k = 0; k < 3; k ++) if (setID[k] == (dsInst & 0xff)) break;
		if (k >= 3) { error_msg("\pTimbre set missed.", 999); return; }
		newValue = popup_menu_select(&menu[k], menuID[k], oldValue, dsInst >> 8, button);
		if (newValue < 0) return;
	}
	enque_gedit_history(sg);
	gene[0] = (gene[0] & 0xe0) | newValue;
	txt[0] = 1; txt[1] = bin2hex(newValue);
	SetControlTitle(button, txt);
	revise_gedit_score(sg, true);
}
static void click_key_button(SBgedit *sg, Gene *gene, ControlRef button) {
	static	MenuRef	menu = NULL;
	short	newValue = popup_menu_select(&menu, mKeyNote, 7 - (*gene & 7), -1, button);
	if (newValue >= 0) {
		unsigned char	txt[8];
		enque_gedit_history(sg);
		newValue = 7 - newValue;
		gene[0] = (gene[0] & 0xe0) | newValue | ((unsigned short)Random() & 0x08);
		revise_gedit_score(sg, true);
		set_key_note_txt(newValue, txt);
		SetControlTitle(button, txt);
	}
}
static void click_melody_button(SBgedit *sg, Gene *gene, ControlRef button) {
	static	MenuRef	menu = NULL;
	static	start_delta[] = {0, 2, 6, 10, 14},
			n_delta[] = {2, 4, 4, 4, 2};
	short	newValue = popup_menu_select(&menu,
		mMelodyShft, 2 - delta_note[*gene & 0x0f], -1, button);
	if (newValue >= 0) {
		unsigned char	txt[4];
		enque_gedit_history(sg);
		newValue = 4 - newValue;
		gene[0] = (gene[0] & 0xe0) |
			(((unsigned short)Random() % n_delta[newValue]) + start_delta[newValue]);
		revise_gedit_score(sg, true);
		set_melody_shift_txt(newValue - 2, txt);
		SetControlTitle(button, txt);
	}
}
enum { gvApplyButton = 1, gvCancelButton, gvTitleText, gvEditText, gvSlider };
static pascal Boolean my_slider_filter(DialogPtr dlg, EventRecord *e, short *item) {
	ControlRef	button;
	OSErr	err;
	switch (e->what) {
		case keyDown: switch (e->message & charCodeMask) {
			case '\r': err = GetDialogItemAsControl(dlg, gvApplyButton, &button);
			if (err == noErr && IsControlActive(button)) *item = gvApplyButton;
			else return false;
			break;
			case 0x1b: *item = gvCancelButton; break;
			case '.': if (e->modifiers & cmdKey) *item = gvCancelButton;
			default: return false;
		}
		hilite_button(dlg, *item);
		return true;
		case updateEvt:
		if ((WindowPtr)e->message != GetDialogWindow(dlg))
			DoUpdate(e);
		break;
		case activateEvt:
		if ((WindowPtr)e->message != GetDialogWindow(dlg))
			{ *item = 999; return true; }
	}
	return false;
}
void click_gedit_dlg(EventRecord *e) {
	short	itemID;
	Point	p = e->where;
	SBgedit	*sg = FindGedit(TopWindow);
	if (!sg) return;
	SetPort(myGetDPort(geditDlg)); GlobalToLocal(&p);
	itemID = FindDialogItem(geditDlg, p) + 1;
	if (sg->editPart >= ChordPart) {
		if (GEmelodyID <= itemID && itemID < GErhythmID
		 && !(itemID == GEmelodyID && sg->editPart < DsPercPart
		 		&& sg->b.keyNote != KeyNoteVariable)) {
			ControlRef	button;
			short	p = (sg->editPart == ChordPart + 1)? ChordPart : sg->editPart;
			Gene	*gene
				= &sg->ind.gene[sg->b.genePart[p][0]][itemID - GEmelodyID];
			GetDialogItemAsControl(geditDlg, itemID, &button);
			if (sg->editPart >= DsPercPart) click_drums_button(sg, gene, button);
			else if (itemID == GEmelodyID) click_key_button(sg, gene, button);
			else click_melody_button(sg, gene, button);
			return;
		}
	}
	check_gedit_dlg(e);
}
void gedit_modified(SBgedit *sg) {
	if (sg->fmode == NotModifiedFile) {
		ControlRef	button;
		sg->fmode = ModifiedFile;
		SetWindowModified(sg->win, true);
		GetDialogItemAsControl(geditDlg, GEsaveID, &button);
		ActivateControl(button);
		EnableMenuItem(GetMenuHandle(mFile), iSaveFile);
		SetWindowModified(sg->win, true);
	}
}
void gedit_bi_changed(WindowPtr win, short part) {
	SBgedit	*sg = FindGedit(win);
	if (!sg) return;
	develop_score(&sg->ind, &sg->b);
	update_gedit(win);
	play_restart();
	gedit_modified(sg);
	if (part == sg->editPart || part < 0) setup_gedit(sg);
}
