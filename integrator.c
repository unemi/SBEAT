#include  <stdio.h>
#include	<string.h>
#include	"decl.h"
#include	"dialog.h"
#include	"vers.h"

extern	short	playID;
extern	Boolean	on_MacOS_X;
extern	WindowPtr	TopWindow, PlayWindow;
extern	unsigned char	displayPartID[], displayQueue[];

#define	MaxDisplaScale	4
static	short	ButtonIDs1[] = { CntlShiftL, CntlShiftR, 0},
				ButtonIDs2[] = { CntlNoteUp, CntlNoteDn, 0};
static void activate_integ_buttons(SBIntegrator *si, Boolean active, short *ids) {
	short	i;
	ControlHandle	button;
	for (i = 0; ids[i]; i ++) {
		button = si->button[ids[i] - 1];
		if (active) ActivateControl(button);
		else DeactivateControl(button);
	}
}
static void set_integ_bottom_text(SBIntegrator *si, Boolean flag) {
	short	psel = si->sel + 1;
#ifdef	JAPANESE_VERSION
	char	buf[128], fmtText[64], *p;
	if (psel > 0) {
		GetIndString((unsigned char *)fmtText, 128, 18);
		fmtText[fmtText[0] + 1] = '\0';
		sprintf(buf, fmtText + 1, psel);
	} else {
		GetIndString((unsigned char *)fmtText, 128, 19);
		BlockMove(fmtText + 1, buf, fmtText[0]);
		buf[fmtText[0]] = '\0';
	}
	p = buf + strlen(buf);
	if (si->nSelBars > 0) {
		GetIndString((unsigned char *)fmtText, 128, 20);
		fmtText[fmtText[0] + 1] = '\0';
		sprintf(p, fmtText + 1, si->nSelBars);
	} else {
		GetIndString((unsigned char *)fmtText, 128, 21);
		BlockMove(fmtText + 1, p, fmtText[0]);
		p[fmtText[0]] = '\0';
	}
#else
	char	buf[128], fmtText[64], tmpText[32], *p;
	if (psel > 0) {
		GetIndString((unsigned char *)fmtText, 128, 18);
		fmtText[fmtText[0] + 1] = '\0';
		if (psel <= 12) {
			GetIndString((unsigned char *)tmpText, 258, psel);
			tmpText[tmpText[0] + 1] = '\0';
			sprintf(buf, fmtText + 1, tmpText + 1);
		} else {
			char	*post = "th";
			if (psel > 20) switch (psel % 10) {
				case 1: post = "st"; break;
				case 2: post = "nd"; break;
				case 3: post = "rd"; break;
			}
			sprintf(tmpText, "%d%s", psel, post);
			sprintf(buf, fmtText + 1, tmpText);
		}
	} else {
		GetIndString((unsigned char *)fmtText, 128, 19);
		fmtText[fmtText[0] + 1] = '\0';
		BlockMove(fmtText + 1, buf, fmtText[0]);
		buf[fmtText[0]] = '\0';
	}
	p = buf + strlen(buf) + 1;
	p[-1] = ' ';
	if (si->nSelBars > 0) {
		char	*be, *s;
		GetIndString((unsigned char *)fmtText, 128, 20);
		fmtText[fmtText[0] + 1] = '\0';
		if (si->nSelBars == 1) { be = "is"; s = ""; }
		else { be = "are"; s = "s"; }
		if (si->nSelBars <= 12)
			GetIndString((unsigned char *)tmpText, 257, si->nSelBars);
		else NumToString(si->nSelBars, (unsigned char *)tmpText);
		tmpText[tmpText[0] + 1] = '\0';
		sprintf(p, fmtText + 1, be, tmpText + 1, s);
	} else {
		GetIndString((unsigned char *)fmtText, 128, 21);
		BlockMove(fmtText + 1, p, fmtText[0]);
		p[fmtText[0]] = '\0';
	}
#endif
	SetControlData(si->bottomText,kControlEditTextPart,
		kControlStaticTextTextTag,strlen(buf),buf);
	if (flag) Draw1Control(si->bottomText);
}
static void make_integ_window(SBIntegrator *si, unsigned char *title) {
	ControlHandle	container;
	Rect	r;
	short	i;
	OSErr	err;
	ControlFontStyleRec	fstyle;
	si->win = GetNewCWindow(kBaseResID+1, nil, kMoveToFront);
	if ((err = CreateRootControl(si->win, &si->root)) != noErr) {
		error_msg("\pCreateRootControl for integrator", err); ExitToShell();
	}
	si->scrollBar = GetNewControl(129, si->win);
	if ((err = EmbedControl(si->scrollBar, si->root)) != noErr)
		{ error_msg("\pEmbedControl scrollbar", err); ExitToShell(); }
	si->winHeight = IntWindowHeight;
	SizeWindow(si->win, IntWindowWidth+GrowIconSize,
		si->winHeight+intWinTopSpace+GrowIconSize, true);
	SetWRefCon(si->win, WIDIntegrator);
	set_drag_callback(si->win);
	MoveControl(si->scrollBar, IntWindowWidth, intWinTopSpace-1);
	SizeControl(si->scrollBar, ScrollBarWidth, IntWindowHeight+2);
	SetControlMaximum(si->scrollBar, si->barHeight*si->nSectionsH-IntWindowHeight);
	SetControlValue(si->scrollBar, 0); si->scroll = 0;
	SetControlViewSize(si->scrollBar, IntWindowHeight); /* MacOS 8.5 or later */
	SetRect(&r, -1, -1, IntWindowWidth+ScrollBarWidth, intWinTopSpace);
	container = NewControl(si->win,&r,"\p",true,0,0,0,kControlWindowHeaderProc,0);
	if ((err = EmbedControl(container, si->root)) != noErr)
		{ error_msg("\pEmbedControl header", err); ExitToShell(); }
	r.top = IntControlPadH; r.bottom = r.top + IntControlH;
	r.left = (IntWindowWidth+ScrollBarWidth-1
		-(IntControlW+IntControlPadW)*NintButtons+IntControlPadW) / 2;
	for (i = 0; i < NintButtons; i ++) {
		r.right = r.left + IntControlW;
		si->button[i] = NewControl(si->win, &r, "\p", true, 0,
			kControlBehaviorPushbutton|kControlContentCIconRes, 160 + i,
			kControlBevelButtonSmallBevelProc, i + 1);
		set_control_help_tag(si->button[i], kHMOutsideTopLeftAligned, 162, i + 1);
		if ((err = EmbedControl(si->button[i], container)) != noErr)
			{ error_msg("\pEmbedControl integrator button", err); ExitToShell(); }
		r.left = r.right + IntControlPadW;
	}
	SetRect(&r, -1, IntWindowHeight+intWinTopSpace,
		IntWindowWidth+1, IntWindowHeight+intWinTopSpace+GrowIconSize+1);
	si->bottomBar = NewControl(si->win,&r,"\p",true,0,0,0,kControlPlacardProc,0);
	if ((err = EmbedControl(si->bottomBar, si->root)) != noErr)
		{ error_msg("\pEmbedControl bottom bar", err); ExitToShell(); }
	InsetRect(&r,2,2); r.left += 10;
	si->bottomText = NewControl(si->win,&r,"\p",true,0,0,0,kControlStaticTextProc,0);
	if ((err = EmbedControl(si->bottomText, si->bottomBar)) != noErr)
		{ error_msg("\pEmbedControl bottom bar", err); ExitToShell(); }
	si->PlayID = si->sel = -1;
	fstyle.flags = kControlUseSizeMask;
	fstyle.size = 9;
	SetControlFontStyle(si->bottomText, &fstyle);
	set_integ_bottom_text(si,false);
	si->cursor_mode = CntlPalm;
	HiliteControl(si->button[CntlPalm-1], kControlButtonPart);
	HiliteControl(si->button[CntlSelArrow-1], kControlNoPart);
	DeactivateControl(si->button[CntlStop-1]);
	DeactivateControl(si->button[CntlPause-1]);
	if (si->displayScale == 1) DeactivateControl(si->button[CntlLarger-1]);
	activate_integ_buttons(si, false, ButtonIDs1);
	activate_integ_buttons(si, (si->nSelBars > 0), ButtonIDs2);
	SetWTitle(si->win, title);
	SetWindowProxyCreatorAndType(si->win, CreatorCode, FileTypeIntegrator, kOnSystemDisk);
	ShowWindow(si->win);
	EnableMenuItem(GetMenuHandle(mFile), iSaveAsSMF);
	set_win_menu_item(si->win);
}
static void adjust_bar_size(SBIntegrator *si) {
	short	d = si->displayScale;
	si->barHeight = IntegYUnit / d;
	si->barWidth = SubWinWidth / d;
	si->nSectionsW = NSectionsW * d;
}
static Boolean setup_score_handle(SBIntegrator *si) {
	integScoreHandle	scoreH;
	si->nBars = si->nSectionsH * si->nSectionsW;
	scoreH = (integScoreHandle)NewHandle(sizeof(integScoreRec) * si->nBars);
	if (!scoreH) {
		error_msg("\pNewHandle for setup_score_handle", MemError());
		return false;
	}
	si->scoreHandle = scoreH;
	return true;
}
void NewIntegratorCB(void) {
	static	long	NewIntegratorNum = 1;
	short	i;
	SBIntegrator	*si = new_integrator_rec();
	unsigned char	title[64];
	integScorePtr	scorep;
	if (!si) return;
	untitled_str(title, &NewIntegratorNum);
	si->displayScale = DefaultDispScale;
	adjust_bar_size(si);
	si->nSectionsH = (PopulationSize + si->nSectionsW - 1) / si->nSectionsW;
	if (!setup_score_handle(si)) return;
	HLock((Handle)si->scoreHandle);
	scorep = *(si->scoreHandle);
	for (i = 0; i < si->nBars; i ++)
		scorep[i].flag = 0;
	for (i = 0; i < NDispParts; i ++) {
		si->vInfo.viewOn[i] = true;
		si->vInfo.viewPartID[i] = si->vInfo.viewQue[i] = i;
	}
	for (; i < NTotalParts; i ++) si->vInfo.viewOn[i] = false;
	if (TopWindow) {
		SBField *sf; SBgedit *sg;
		switch (GetWRefCon(TopWindow)) {
			case WIDField: sf = FindField(TopWindow); if (!sf) break;
			for (i = 0; i < PopulationSize; i ++) {
				scorep[i].flag = ItgOnFlag;
				scorep[i].i = sf->pop[i];
				scorep[i].b = sf->b;
			}
			si->vInfo = sf->v;
			break;
			case WIDgedit: sg = FindGedit(TopWindow); if (!sg) break;
			scorep[0].flag = ItgOnFlag;
			scorep[0].i = sg->ind;
			scorep[0].b = sg->b;
			si->vInfo = sg->v;
		}
	}
	HUnlock((Handle)si->scoreHandle);
	si->fmode = NewFile;
	si->refNum = 0;
	si->nSelBars = 0;
	make_integ_window(si, (unsigned char *)title);
	SetWindowModified(si->win, true);
}

typedef struct {
	char	version[16];
	short	sel;
	short	scroll;
	short	nSectionsH;
	short	winHeight;
	short	animeTime;
	short	origWinHeight;
	Point	winPosition;
	ViewInfo	vInfo;
}	IntegImageHeader;

void open_integrator(FSSpec *fss) {
	short	refNum;
	long	i, n;
	OSErr	err;
	SBIntegrator	*si;
	integScorePtr	scorep;
	IntegImageHeader	img;
	extern	SBIntegrator	*Integrators;
	for (si = Integrators; si; si = si->post)
		if (equiv_fss(&si->fsSpec, fss))
			{ SelectWindow(si->win); return; }
	err = FSpOpenDF(fss,fsRdWrPerm,&refNum);
	if (err) { error_msg(fss->name, err); return; }
	n = sizeof(img);
	err = FSRead(refNum, &n, &img);
	if (err != noErr) { error_msg(fss->name, err); FSClose(refNum); return; }
	if (strncmp(img.version, "SBEAT", 5)
	 || strncmp(img.version+5, ProductVersionShort, 2)) {
		error_msg("\pIt isn't a compatible version.", 0);
		FSClose(refNum); return;
	}
	si = new_integrator_rec();
	if (!si) { FSClose(refNum); return; }
	si->nSectionsH = img.nSectionsH & 0x0fff;
	si->displayScale = img.nSectionsH >> 12;
	if (si->displayScale <= 0) si->displayScale = 1;
	adjust_bar_size(si);
	if (!setup_score_handle(si)) return;
/*	si->sel = img.sel;
	si->scroll = img.scroll;
	si->winHeight = img.winHeight;
	si->origWinHeight = img.origWinHeight;
	si->winPosition = img.winPosition;
*/	si->vInfo = img.vInfo;
	si->refNum = refNum;
	HLock((Handle)si->scoreHandle);
	scorep = *(si->scoreHandle);
	n = sizeof(integScoreRec) * si->nBars;
	err = FSRead(refNum, &n, scorep);
	for (i = si->nSelBars = 0; i < si->nBars; i ++)
		if (ItgSelectedP(scorep, i)) si->nSelBars ++;
	HUnlock((Handle)si->scoreHandle);
	if (err != noErr) { error_msg(fss->name, err); FSClose(refNum); return; }
	si->fmode = NotModifiedFile;
	si->fsSpec = *fss;
	make_integ_window(si, fss->name);
	SetWindowModified(si->win, false);
}
static Boolean save_integrator_ref(SBIntegrator *si) {
	long	n;
	short	err;
	integScorePtr	scorep;
	IntegImageHeader	img;
	SetFPos(si->refNum, fsFromStart, 0);
	sprintf(img.version, "SBEAT%s\r", ProductVersionShort);
	img.sel = si->sel;
	img.scroll = si->scroll;
	img.nSectionsH = si->nSectionsH | (si->displayScale << 12);
	img.winHeight = si->winHeight;
	img.origWinHeight = si->origWinHeight;
	img.winPosition = si->winPosition;
	img.vInfo = si->vInfo;
	n = sizeof(img);
	if (FSWrite(si->refNum, &n, &img) != noErr) return false;
	HLock((Handle)si->scoreHandle);
	scorep = *(si->scoreHandle);
	n = sizeof(integScoreRec) * si->nBars;
	err = FSWrite(si->refNum, &n, scorep);
	HUnlock((Handle)si->scoreHandle);
	SetEOF(si->refNum, n + sizeof(img));
	si->fmode = NotModifiedFile;
	SetWindowModified(si->win, false);
	return true;
}
Boolean save_integrator_as(SBIntegrator *si) {
	static	unsigned char	title[32] = {0};
	if (!si) return false;
	if (!title[0]) GetIndString(title, 128, 13);
	if (save_as_setup(si->win, &si->fsSpec, &si->refNum,
		FileTypeIntegrator, title))
		return save_integrator_ref(si);
	else return false;
}
Boolean save_integrator(SBIntegrator *si) {
	if (!si) return false;
	switch (si->fmode) {
		case NewFile: return save_integrator_as(si);
		case ModifiedFile: return save_integrator_ref(si);
	}
	return true;
}
void close_integrator(SBIntegrator *si) {
	short	i;
	WindowPtr	win;
	if (!si) return;
	win = si->win;
	for (i = 0; i < si->nBars; i ++) if (ItgOnP(*si->scoreHandle,i)) break;
	if (i < si->nBars && si->fmode != NotModifiedFile)
	switch (save_change_alert(win, 1)) {
		case itemCancel: return;
		case itemOK: if (!save_integrator(si)) return;
	}
	if (si->refNum > 0) FSClose(si->refNum);
	remove_drag_callback(win);
	rm_history(win);
	si->win = NULL;
	free_integrator(si);
	close_sb_window(win);
}
void reset_integrator(SBIntegrator *si) {
	short	i;
	if (!si) return;
	if (si->win == PlayWindow) stop_tune();
	HLock((Handle)si->scoreHandle);
	for (i = 0; i < si->nBars; i ++) (*si->scoreHandle)[i].flag = 0;
	HUnlock((Handle)si->scoreHandle);
	update_integrator(si);
}
static void draw_sel_frame_pat(SBIntegrator *si,
	unsigned short pats1, unsigned short pats2) {
	short	top, bottom, left, right, ytop, i;
	Pattern	p;
	unsigned char	patc;
	if (si->sel < 0) return;
	ytop = top = (si->sel / si->nSectionsW) * si->barHeight - si->scroll;
	if (top >= si->winHeight || top <= -si->barHeight) return;
	bottom = top + SubWinPHeight / si->displayScale - 1;
	if (top < 0) top = 0;
	if (bottom >= si->winHeight) bottom = si->winHeight - 1;
	left = WinLeftSpace + (si->sel % si->nSectionsW) * si->barWidth;
	right = left + si->barWidth - 2;
	SetPort(myGetWPort(si->win));
	PenSize(SelFrameWidth, SelFrameWidth);
	PenMode(patXor);
	ForeColor(blackColor);
	top += intWinTopSpace; bottom += intWinTopSpace;
	if (ytop >= 0) {
		patc = (pats1 >> si->animeTime) & 0xff;
		for (i = 0; i < 8; i ++) p.pat[i] = patc;
		PenPat(&p);
		MoveTo(left, top); Line(si->barWidth-2, 0);
	}
	if (ytop + SubWinPHeight / si->displayScale <= si->winHeight) {
		patc = (pats2 << si->animeTime) >> 8;
		for (i = 0; i < 8; i ++) p.pat[i] = patc;
		PenPat(&p);
		MoveTo(left, bottom); Line(si->barWidth-2, 0);
	}
	patc = (pats1 >> ((si->animeTime + si->scroll) % 8)) & 0xff;
	for (i = 0; i < 8; i ++, patc >>= 1)
		p.pat[i] = (patc & 1)? 0xff : 0x00;
	PenPat(&p);
	MoveTo(left, top); Line(0, bottom - top);
	patc = (pats1 >> ((si->animeTime - si->scroll + 8888) % 8)) & 0xff;
	for (i = 0; i < 8; i ++, patc <<= 1)
		p.pat[i] = (patc & 0x80)? 0xff : 0x00;
	PenPat(&p);
	MoveTo(right, top); Line(0, bottom - top);
	PenNormal();
}
void check_integ_sel(void) {
	SBIntegrator	*si;
	GrafPtr	oldG;
	if (!TopWindow) return;
	if (GetWRefCon(TopWindow) != WIDIntegrator) return;
	si = FindIntegrator(TopWindow);
	if (!si) return;
	if (si->sel < 0) return;
	GetPort(&oldG); SetPort(myGetWPort(si->win));
	draw_sel_frame_pat(si, 0x9090, 0x0909);
	SetPort(oldG);
	si->animeTime = (si->animeTime + 1) % 8;
}
void draw_sel_frame(SBIntegrator *si, Boolean) {
	draw_sel_frame_pat(si, 0x1f1f, 0xf8f8);
}
void activate_integ_controls(SBIntegrator *si, Boolean flag) {
	if (!si) return;
	if (flag) {
		ActivateControl(si->root);
		set_current_integ_po(si);
	} else DeactivateControl(si->root);
}
void update_integrator(SBIntegrator *si) {
	Rect	rs;
	GrafPtr	oldg;
	RgnHandle	oldClip, growClip, newClip;
	extern	Point	noteP;
	if (!si) return;
	GetPort(&oldg);
	SetPort(myGetWPort(si->win));
	begin_integ_update();
	draw_integ_scores(si);
	if (playID >= 0 && PlayWindow == si->win) inverse_playing_mark();
	draw_sel_frame(si, true);
	oldClip = NewRgn(); growClip = NewRgn(); newClip = NewRgn();
	GetClip(oldClip);
	GetWindowPortBounds(si->win, &rs);
	rs.top = intWinTopSpace; rs.bottom -= GrowIconSize;
	rs.left = rs.right - GrowIconSize;
	RectRgn(growClip, &rs);
	SectRgn(oldClip, growClip, newClip);
	DisposeRgn(oldClip); DisposeRgn(growClip);
	SetClip(newClip);
	DrawGrowIcon(si->win);
	end_integ_update();
	UpdateControls(si->win, GetPortVisibleRegion(
		myGetWPort(si->win), newClip));
	SetPort(oldg);
	DisposeRgn(newClip);
}
static void change_integ_sel_mode(SBIntegrator *si, short) {
	switch (si->cursor_mode) {
		case CntlPalm: si->cursor_mode = CntlSelArrow;
		HiliteControl(si->button[CntlPalm-1], kControlNoPart);
		HiliteControl(si->button[CntlSelArrow-1], kControlButtonPart); break;
		case CntlSelArrow: si->cursor_mode = CntlPalm;
		HiliteControl(si->button[CntlSelArrow-1], kControlNoPart);
		HiliteControl(si->button[CntlPalm-1], kControlButtonPart); break;
	}
}
static void click_play_button(SBIntegrator *si) {
	short	i;
	for (i = 0; i < si->nBars; i ++) if (ItgOnP(*si->scoreHandle,i)) break;
	if (i >= si->nBars) return;
	if (si->win == TopWindow) {
		short	id = 0;
		if (si->PlayID >= 0) id = si->PlayID;
		else if (si->sel >= 0) id = si->sel;
		play_sequence(si->win, id);
	} else if (si->PlayID < 0)
		si->PlayID = (si->sel >= 0)? si->sel : 0;
	DeactivateControl(si->button[CntlPlay-1]);
	ActivateControl(si->button[CntlStop-1]);
	ActivateControl(si->button[CntlPause-1]);
}
static void click_stop_button(SBIntegrator *si) {
	si->PlayID = -1;
	if (si->win == PlayWindow) stop_tune();
	DeactivateControl(si->button[CntlStop-1]);
	DeactivateControl(si->button[CntlPause-1]);
	ActivateControl(si->button[CntlPlay-1]);
}
static void click_pause_button(SBIntegrator *si) {
	if (si->PlayID >= 0) {
		si->PlayID -= 20000;
		if (si->win == PlayWindow) stop_tune();
		DeactivateControl(si->button[CntlStop-1]);
		HiliteControl(si->button[CntlPause-1], kControlButtonPart);
	} else {
		if (si->PlayID < -1) si->PlayID += 20000;
		HiliteControl(si->button[CntlPause-1], kControlNoPart);
		click_play_button(si);
	}
}
static void score_note_shift(SBIntegrator *si, short delta) {
	short	id, p, pp;
	integScorePtr	scorep;
	Boolean	modified = false, tuneModified = false;
	extern	DialogRef	playerDlg;
	HLock((Handle)si->scoreHandle);
	scorep = *si->scoreHandle;
	enque_note_shift_history(si, scorep);
	for (id = pp = 0; id < si->nBars; id ++)
	if (ItgSelectedOnP(scorep, id)) {
		if (scorep[id].b.keyNote != KeyNoteVariable) {
			p = scorep[id].b.keyNote - delta;
			if (KeyNoteMin <= p && p <= KeyNoteMax) {
				scorep[id].b.keyNote = p;
				if (pp == 0) pp = p;	// first time
				else if (pp > 0 && pp != p) pp = -1;	// check uniq or not
			}
		} else {
			Gene	*gp;
			p = scorep[id].b.genePart[ChordPart][0];
			gp = &scorep[id].i.gene[p][0];
			*gp = (*gp & 0xe0) | ((*gp + delta) & 0x1f);
		}
		develop_score(&scorep[id].i, &scorep[id].b);
		draw_integ_ind(si, id, true);
		modified = true;
		if (id == si->PlayID) tuneModified = true;
	}
	HUnlock((Handle)si->scoreHandle);
	if (pp > 0 && playerDlg) {
		ControlRef	control;
		GetDialogItemAsControl(playerDlg, SKPopMenu, &control);
		SetControlValue(control, pp);
	}
	if (modified) integ_modified(si);
	if (si->win == PlayWindow && tuneModified)
		revise_tune_queue(si->win, si->PlayID);
}
static void integ_shift_right(SBIntegrator *si) {
	short	i;
	integScorePtr	scorep;
	if (si->sel < 0 || si->sel >= si->nBars - 1) return;
	if (!ItgOnP(*si->scoreHandle,si->sel)) return;
	for (i = si->sel + 1; i < si->nBars; i ++)
		if (!ItgOnP(*si->scoreHandle,i)) break;
	if (i >= si->nBars) i = si->nBars - 1;	// @@@@@ enlarge ???
	enque_integ_history(HistILeft, si, i, si->sel);
	integ_modified(si);
	SetPort(myGetWPort(si->win));
	HLock((Handle)si->scoreHandle);
	scorep = *(si->scoreHandle);
	for ( ; i > si->sel; i --) {
		scorep[i] = scorep[i - 1];
		draw_integ_ind(si, i, true);
	}
	scorep[si->sel].flag &= ~(ItgOnFlag | ItgSelectedFlag);
	draw_integ_ind(si, si->sel, true);
	draw_sel_frame(si, false);
	si->sel ++;
	draw_sel_frame(si, true);
	HUnlock((Handle)si->scoreHandle);
}
static void integ_shift_left(SBIntegrator *si) {
	short	i, k;
	integScorePtr	scorep;
	if (si->sel < 1) return;
	if (!ItgOnP(*si->scoreHandle,si->sel)) return;
	for (k = si->sel + 1; k < si->nBars; k ++) if (!ItgOnP(*si->scoreHandle,k)) break;
	k --;
	enque_integ_history(HistIRight, si, si->sel - 1, k);
	integ_modified(si);
	SetPort(myGetWPort(TopWindow));
	HLock((Handle)si->scoreHandle);
	scorep = *(si->scoreHandle);
	si->sel --;
	for (i = si->sel; i < k; i ++) {
		scorep[i] = scorep[i + 1];
		draw_integ_ind(si, i, true);
	}
	scorep[k].flag &= ~(ItgOnFlag | ItgSelectedFlag);
	draw_integ_ind(si, k, true);
	HUnlock((Handle)si->scoreHandle);
}
static Boolean expand_score_handle(SBIntegrator *si, short newSize) {
	Handle	newHdl;
	short	i;
	newHdl = NewHandle(sizeof(integScoreRec) * newSize);
	if (!newHdl) {
		error_msg("\pNewHandle for expanding score.", MemError());
		return false;
	}
	HLock(newHdl); HLock((Handle)si->scoreHandle);
	BlockMove((Ptr)(*si->scoreHandle), *newHdl, sizeof(integScoreRec) * si->nBars);
	HUnlock((Handle)si->scoreHandle);
	DisposeHandle((Handle)si->scoreHandle);
	si->scoreHandle = (integScoreHandle)newHdl;
	for (i = si->nBars; i < newSize; i ++)
		(*si->scoreHandle)[i].flag = 0;
	HUnlock(newHdl);
	si->nBars = newSize;
	return true;
}
Boolean expand_integ(SBIntegrator *si, short n) {
	if (!expand_score_handle(si, (si->nSectionsH + n) * si->nSectionsW))
		return false;
	if (si->nSectionsH == 1) {
		ActivateControl(si->button[CntlShrink-1]);
		if (si->win == TopWindow)
			EnableMenuItem(GetMenuHandle(mScoreEdit), iShrink);
	}
	si->nSectionsH += n;
	if (GetControlMaximum(si->scrollBar) <= 0) zoom_integ(si);
	else {
		Rect	portRect;
		GetWindowPortBounds(si->win, &portRect);
		SetControlMaximum(si->scrollBar, si->barHeight*si->nSectionsH
			-portRect.bottom+intWinTopSpace+GrowIconSize);
	}
	return true;
}
static void adjust_integ_scroll(SBIntegrator *si) {
	short	smax = si->nSectionsH * si->barHeight - si->winHeight;
	SetControlMaximum(si->scrollBar, smax);
	if (si->scroll > smax) {
		si->scroll = smax;
		SetControlValue(si->scrollBar, si->scroll);
		SetPort(myGetWPort(si->win));
		begin_integ_update();
		draw_integ_scores(si);
		end_integ_update();
	}
	SetControlViewSize(si->scrollBar, si->winHeight); /* MacOS 8.5 or later */
	set_integ_bottom_text(si,true);
}
static void resize_integrator(SBIntegrator *si, short height) {
	draw_sel_frame(si, false);
	SizeWindow(si->win, IntWindowWidth+ScrollBarWidth-1, height, true);
	height -= GrowIconSize;
	MoveControl(si->bottomBar, -1, height);
	height -= intWinTopSpace;
	SizeControl(si->scrollBar, ScrollBarWidth, height + 2);
	si->winHeight = height;
	adjust_integ_scroll(si);
	draw_sel_frame(si, true);
}
static void shrink_integ(SBIntegrator *si) {
	short	smax, i, k, m, onMask, nselb;
	OSErr	err;
	integScorePtr	scorep;
	if (si->nSectionsH <= 1) return;
	HLock((Handle)si->scoreHandle);
	k = (si->nSectionsH - 1) * si->nSectionsW;
	scorep = *si->scoreHandle;
	for (i = onMask = nselb = 0, m = 1; i < si->nSectionsW; i ++, k ++, m <<= 1) {
		if (ItgOnP(scorep, k)) onMask |= m;
		if (ItgSelectedP(scorep, k))
			{ nselb ++; scorep[k].flag &= ~ItgSelectedFlag; }
	} 
	HUnlock((Handle)si->scoreHandle);
	enque_integ_history(HistIShrink, si, onMask, 0);
	SetHandleSize((Handle)si->scoreHandle,
		sizeof(integScoreRec) * (si->nSectionsH - 1) * si->nSectionsW);
	if ((err = MemError()) != noErr)
		{ error_msg("\pSetHandleSize to shrink score.", err); return; }
	if (onMask != 0) integ_modified(si);
	si->nSectionsH --;
	si->nBars -= si->nSectionsW;
	if (si->PlayID >= si->nBars) si->PlayID = 0;
	if (si->sel >= si->nBars) si->sel = -1;
	if (nselb > 0) si->nSelBars -= nselb;
	smax = si->barHeight * si->nSectionsH - si->winHeight;
	if (smax < 0) {
		resize_integrator(si,
			si->barHeight * si->nSectionsH + intWinTopSpace + GrowIconSize);
		if (nselb > 0) set_integ_bottom_text(si, true);
	} else adjust_integ_scroll(si);
	if (si->nSectionsH == 1) {
		DeactivateControl(si->button[CntlShrink-1]);
		if (si->win == TopWindow) DisableMenuItem(GetMenuHandle(mScoreEdit), iShrink);
	}
}
static Boolean adjust_display_scale(SBIntegrator *si) {
	short topBarID, nsh, nnsecs, nWinH;
	GrafPtr	oldPort;
	topBarID = (si->scroll / si->nSectionsH) * si->nSectionsW;
	adjust_bar_size(si);
	nsh = (si->nBars + si->nSectionsW - 1) / si->nSectionsW;
	nnsecs = nsh * si->nSectionsW;
	if (nnsecs > si->nBars)
		if (!expand_score_handle(si, nnsecs)) return false;
	si->nSectionsH = nsh;
	nsh *= si->barHeight;	// nsh <- height of new space
	nWinH = (si->winHeight > nsh)? nsh :
		(si->winHeight < si->barHeight)? si->barHeight : 0;
	if (nWinH > 0) {
		si->winHeight = nWinH;
		SizeWindow(si->win, IntWindowWidth+GrowIconSize,
			nWinH+intWinTopSpace+GrowIconSize, false);
		SizeControl(si->scrollBar, ScrollBarWidth, si->winHeight + 2);
		MoveControl(si->bottomBar, -1, nWinH+intWinTopSpace);
	}
	nsh -= si->winHeight;	// nsh <- maximum value of scroll bar
	SetControlMaximum(si->scrollBar, nsh);
	SetControlViewSize(si->scrollBar, si->winHeight);
	si->scroll = topBarID / si->nSectionsW * si->nSectionsH;
	if (si->scroll > nsh) si->scroll = nsh;
	SetControlValue(si->scrollBar, si->scroll);
	GetPort(&oldPort);
	SetPort(GetWindowPort(si->win));
	begin_integ_update();
	draw_integ_scores(si);
	if (playID >= 0 && PlayWindow == si->win) inverse_playing_mark();
	draw_sel_frame(si, true);
	end_integ_update();
	SetPort(oldPort);
	return true;
}
static void disp_larger_integ(SBIntegrator *si) {
	if (si->displayScale == MaxDisplaScale)
		ActivateControl(si->button[CntlSmaller-1]);
	si->displayScale /= 2;
	if (si->displayScale <= 1) DeactivateControl(si->button[CntlLarger-1]);
	if (!adjust_display_scale(si))
		{ si->displayScale *= 2; adjust_bar_size(si); }
}
static void disp_smaller_integ(SBIntegrator *si) {
	if (si->displayScale == 1) ActivateControl(si->button[CntlLarger-1]);
	si->displayScale *= 2;
	if (si->displayScale >= MaxDisplaScale)
		DeactivateControl(si->button[CntlSmaller-1]);
	if (!adjust_display_scale(si))
		{ si->displayScale /= 2; adjust_bar_size(si); }
}
static void click_integ_button(SBIntegrator *si, Point where, EventRecord *e) {
	ControlHandle	control;
	long	cref;
	short	part;
	control = FindControlUnderMouse(where, si->win, &part);
	if (!control || !part) return;
	cref = GetControlReference(control);
	if (!cref) return;
	HandleControlClick(control, where, e->modifiers, nil);
	switch (cref) {
		case CntlPalm: case CntlSelArrow:
		change_integ_sel_mode(si, cref); break;
		case CntlPlay: click_play_button(si); break;
		case CntlStop: click_stop_button(si); break;
		case CntlPause: click_pause_button(si); break;
		case CntlNoteUp: score_note_shift(si, 1); break;
		case CntlNoteDn: score_note_shift(si, -1); break;
		case CntlShiftL: integ_shift_left(si); break;
		case CntlShiftR: integ_shift_right(si); break;
		case CntlExpand: expand_integ(si, 1); break;
		case CntlShrink: shrink_integ(si); break;
		case CntlLarger: disp_larger_integ(si); break;
		case CntlSmaller: disp_smaller_integ(si); break;
	}
}
void activate_integ_sel_ctrl(SBIntegrator *si, Boolean on, Boolean) {
	MenuHandle	mh = GetMenuHandle(mEdit);
	activate_integ_buttons(si, on, ButtonIDs1);
	if (si->win == TopWindow) setup_edit_menu(on, false, WIDIntegrator);
}
Boolean check_integ_selected_on(SBIntegrator *si, Boolean *selp) {
	Boolean	on;
	short	i;
	integScorePtr	scorep;
	HLock((Handle)si->scoreHandle);
	scorep = *si->scoreHandle;
	if (selp) *selp = false;
	for (i = 0, on = false; i < si->nBars; i ++) if (ItgSelectedP(scorep,i)) {
		if (selp) *selp = true;
		if (ItgOnP(scorep,i)) { on = true; break; }
	}
	HUnlock((Handle)si->scoreHandle);
	return on;
}
void activate_integ_sel2_ctrl(SBIntegrator *si) {
	Boolean	selp, on = check_integ_selected_on(si, &selp);
	activate_integ_buttons(si, on, ButtonIDs2);
	if (si->win == TopWindow) {
		activate_poption_items(on);
		setup_edit_menu(on, selp, WIDIntegSel2);
	}
}
short get_cursor_mode(SBIntegrator *si, short modifiers) {
	if (!si) return CntlSelArrow;
	if (modifiers & shiftKey) switch (si->cursor_mode) {
		case CntlPalm: return CntlSelArrow;
		case CntlSelArrow: return CntlPalm;
	}
	return si->cursor_mode;
}
static void select_section(SBIntegrator *si, short ns, short modifiers) {
	if (!si) return;
	if (ns < 0 || ns >= si->nBars) return;
	if (get_cursor_mode(si, modifiers) == CntlSelArrow) {
		(*si->scoreHandle)[ns].flag ^= ItgSelectedFlag;
		draw_integ_ind(si, ns, false);
		activate_integ_sel2_ctrl(si);
		set_current_integ_pi(si);
		if (ItgSelectedP(*si->scoreHandle, ns)) si->nSelBars ++;
		else si->nSelBars --;
	} else if (si->sel == ns) {
		draw_sel_frame(si, false);
		si->sel = -1;
		activate_integ_sel_ctrl(si, false, false);
	} else {
		if (si->sel >= 0) draw_sel_frame(si, false);
		activate_integ_sel_ctrl(si, ItgOnP(*si->scoreHandle,ns), true);
		si->sel = ns;
		draw_sel_frame(si, true);
	}
	set_integ_bottom_text(si,true);
}
static void scroll_integ_view(SBIntegrator *si, short d) {
	GrafPtr	old;
	GetPort(&old); SetPort(myGetWPort(si->win));
	if (d < -si->winHeight || d > si->winHeight)
		update_integrator(si);
	else {
		Rect	rs;
		RgnHandle	oldClip = NewRgn(), updateRgn = NewRgn();
		GetClip(oldClip);
		SetRect(&rs, 0, intWinTopSpace, IntWindowWidth, si->winHeight + intWinTopSpace);
		ScrollRect(&rs, 0, -d, updateRgn);
		SetClip(updateRgn);
		update_integrator(si);
		SetClip(oldClip);
		DisposeRgn(oldClip);
		DisposeRgn(updateRgn);
	}
	set_integ_bottom_text(si,true);
	SetPort(old);
}
void check_integ_playing(SBIntegrator *si) {
	short	row, top, bottom, d;
	row = playID / si->nSectionsW;
	if (row >= si->nSectionsH) return;
	top = row * si->barHeight - si->scroll;
	bottom = top + SubWinPHeight / si->displayScale;
	if (top < 0) d = top;
	else if (bottom > si->winHeight) {
		d = bottom - si->winHeight;
		if (d > top) { d = top; if (d == 0) return; }
	} else return;
	si->scroll += d;
	SetControlValue(si->scrollBar, si->scroll);
	scroll_integ_view(si, d);
	return;
}
static SBIntegrator	*scrollInteg = NULL;
static void scroll_lines(short d) {
	short	k = d - scrollInteg->scroll;
	scrollInteg->scroll = d;
	scroll_integ_view(scrollInteg, k);
}
static pascal void drag_thumb(void) {
	scroll_lines(GetControlValue(scrollInteg->scrollBar));
}
static void force_scroll_lines(ControlHandle control, short d) {
	short	c, m;
	d += (c = GetControlValue(control));
	m = GetControlMaximum(control);
	if (d < 0) d = 0;
	else if (d > m) d = m;
	if (d != c) {
		SetControlValue(control, d);
		scroll_lines(d);
	}
}
static pascal void vertical_action(ControlHandle control, short part) {
	short	d;
	switch (part) {
		case kControlUpButtonPart: d = -ScoreLinePad*2; break;
		case kControlPageUpPart: d = -IntegYUnit; break;
		case kControlDownButtonPart: d = ScoreLinePad*2; break;
		case kControlPageDownPart: d = IntegYUnit; break;
		default: return;
	}
	force_scroll_lines(control, d);
}
static short find_section(SBIntegrator *si, Point where, short *part) {
	short	y, row;
	row = (y = where.v + si->scroll - intWinTopSpace) / si->barHeight;
	if ((y -= row * si->barHeight) > SubWinPHeight / si->displayScale) return -1;
	if (part) *part = y * si->displayScale / PartHeight;
	return row * si->nSectionsW + ((where.h - WinLeftSpace) / si->barWidth);	
}
static void click_scroll_bar(SBIntegrator *si, Point where, short modifiers) {
	static	DragGrayRgnUPP		thumbUPP = NULL;
	static	ControlActionUPP	vactionUPP = NULL;
	ControlActionUPP	upp;
	ControlHandle	control;
	short	part;
	scrollInteg = si;
	control = FindControlUnderMouse(where, si->win, &part);
	if (!control) return;
	switch (part) {
		case kControlIndicatorPart:
		if (!thumbUPP) thumbUPP = NewDragGrayRgnUPP(&drag_thumb);
		upp = (ControlActionUPP)thumbUPP; break;
		case kControlUpButtonPart: case kControlDownButtonPart:
		case kControlPageUpPart: case kControlPageDownPart:
		if (!vactionUPP) vactionUPP = NewControlActionUPP(vertical_action);
		upp = vactionUPP; break;
		default: return;
	}
	HandleControlClick(control, where, modifiers, upp);
}
void drag_integ_scroll(SBIntegrator *si, short direction, short amount) {
	static	unsigned long	oldTime	= 0;
	static	SBIntegrator	*oldSi = NULL;
	static	short	oldDirection = 0;
	if (!si) {
		oldSi = NULL; oldDirection = 0;
	} else if (oldSi != si || oldDirection != direction) {
		oldSi = si; oldDirection = direction;
		oldTime = TickCount();
	} else {
		unsigned long	interval = TickCount() - oldTime;
		if (interval > 6) {
			scrollInteg = si;
			force_scroll_lines(si->scrollBar, amount*2);
			oldTime += interval;
		}
	}
}
static void redraw_gray_frame(Rect *rct) {
	Pattern	grayPat;
	begin_integ_update();
	PenMode(patXor); PenSize(2,2);
	PenPat(GetQDGlobalsGray(&grayPat));
	FrameRect(rct); PenNormal();
	end_integ_update();
}
static short	oldScroll = -1;
static void begin_rect_scroll(SBIntegrator *si, Rect *rct) {
	oldScroll = si->scroll;
	redraw_gray_frame(rct);
}
static void end_rect_scroll(SBIntegrator *si, Rect *rct, Point *pt) {
	short	oldv = pt->v;
	pt->v += oldScroll - si->scroll;
	if (rct->top == oldv) rct->top = pt->v;
	else rct->bottom = pt->v;
	redraw_gray_frame(rct);
}
static void select2_sections(SBIntegrator *si, Point startG, Point startpt) {
	Point	currentPt;
	Rect	mouseRct, rct;
	EventRecord	event;
	RgnHandle	mouseRgn;
	short	i, j, ns;
	integScorePtr	scorep;
	SetRect(&rct, startpt.h, startpt.v, startpt.h, startpt.v);
	SetRect(&mouseRct, startG.h, startG.v, startG.h+1, startG.v+1);
	mouseRgn = NewRgn();
	HLock((Handle)si->scoreHandle);
	scorep = *si->scoreHandle;
	for (;;) {
		RectRgn(mouseRgn, &mouseRct);
		WaitNextEvent(mUpMask|osMask,&event,1,mouseRgn);
		if (event.what == mouseUp) break;
		switch (event.what) {
			case nullEvent:
			currentPt = event.where;
			GlobalToLocal(&currentPt);
			if ((currentPt.v -= intWinTopSpace) < 0) {
				if (si->scroll > 0) {
					begin_rect_scroll(si, &rct);
					drag_integ_scroll(si, -1, currentPt.v);
					end_rect_scroll(si, &rct, &startpt);
				}
			} else if ((currentPt.v -= si->winHeight) < 0)
				drag_integ_scroll(NULL, 0, 0);
			else if (si->scroll < si->nSectionsH * si->barHeight) {
				begin_rect_scroll(si, &rct);
				drag_integ_scroll(si, 1, currentPt.v);
				end_rect_scroll(si, &rct, &startpt);
			}
			check_playing(); check_integ_sel(); break;
			case osEvt: if ((event.message >> 24) != mouseMovedMessage) break;
			currentPt = event.where;
			GlobalToLocal(&currentPt);
			redraw_gray_frame(&rct);
			for (j = ns = 0; j < si->nSectionsH; j ++) {
				short	y = j * si->barHeight + intWinTopSpace - si->scroll;
				for (i = 0; i < si->nSectionsW; i ++, ns ++) {
					Boolean	in;
					short	x = i * si->barWidth + WinLeftSpace;
					if (rct.top > y + SubWinPHeight / si->displayScale || rct.bottom < y
					 || rct.left > x + si->barWidth || rct.right < x) in = false;
					else in = true;
					if ((ItgTmpSelectedP(scorep,ns) != 0) != in) {
						if (in) scorep[ns].flag |= ItgTmpSelectedFlag;
						else scorep[ns].flag &= ~ItgTmpSelectedFlag;
						draw_integ_ind(si, ns, true);
					}
				}
			}
			if (startpt.h < currentPt.h) { rct.left = startpt.h; rct.right = currentPt.h; }
			else { rct.right = startpt.h; rct.left = currentPt.h; }
			if (startpt.v < currentPt.v) { rct.top = startpt.v; rct.bottom = currentPt.v; }
			else { rct.bottom = startpt.v; rct.top = currentPt.v; }
			redraw_gray_frame(&rct);
			SetRect(&mouseRct, event.where.h, event.where.v,
				event.where.h+1, event.where.v+1);
		}
	}
	redraw_gray_frame(&rct);
	for (ns = i = 0; ns < si->nBars; ns ++) {
		if (ItgTmpSelectedP(scorep,ns)) {
			scorep[ns].flag ^= (ItgSelectedFlag | ItgTmpSelectedFlag);
			draw_integ_ind(si, ns, true);
		}
		if (ItgSelectedP(scorep,ns)) i ++;
	}
	HUnlock((Handle)si->scoreHandle);
	DisposeRgn(mouseRgn);
	activate_integ_sel2_ctrl(si);
	if (i > 0) set_current_integ_pi(si);
	si->nSelBars = i;
	set_integ_bottom_text(si,true);
}
void click_integrator(SBIntegrator *si, EventRecord *e) {
	Point	p = e->where;
	if (!si) return;
	SetPort(myGetWPort(si->win));
	GlobalToLocal(&p);
	if (si->win != TopWindow && (p.v < intWinTopSpace ||
		p.h < WinLeftSpace || p.h >= IntWindowWidth))
		SelectWindow(si->win);
	else if (p.v < intWinTopSpace) click_integ_button(si, p, e);
	else if (p.h < WinLeftSpace) return;
	else if (p.h < IntWindowWidth) {
		Rect	r;
		short part, ns = find_section(si, p, &part);
		Boolean	dragp;
		short	mode = get_cursor_mode(si, e->modifiers);
		if (ns < 0) return;
		if (!(e->modifiers & optionKey)) part = -1;
		integ_part_to_rect(ns, part, si, &r);
		if (mode == CntlPalm) set_my_cursor(CURSclosed);
		begin_integ_update();
		dragp = draw_temp_drag_frame(&r, e->where);
		end_integ_update();
		if (!dragp) {
			if (si->win == TopWindow) select_section(si, ns, e->modifiers);
			else SelectWindow(si->win);
			if (mode == CntlPalm) set_my_cursor(CURSgrabber);
		} else if (mode == CntlSelArrow) {
			if (si->win == TopWindow) select2_sections(si, e->where, p);
			else SelectWindow(si->win);
		} else if (ItgOnP(*si->scoreHandle,ns)) drag_integ_ind(si, e);
	} else if (p.v < si->winHeight + intWinTopSpace)
		click_scroll_bar(si, p, e->modifiers);
	else grow_integrator(si, e);
}
void keyin_integrator(SBIntegrator *si, EventRecord *e) {
	short	d;
	if (!si) return;
	switch (e->message & charCodeMask) {
		case 0x08: ClearCB(); return; /* delete */
		case '\r': /* return */
		if (e->modifiers & shiftKey) {
			if (si->sel >= 0) select_section(si, si->sel, e->modifiers);
		} else if (si->PlayID >= 0) click_stop_button(si);
		else if (si->PlayID < -1) click_pause_button(si);
		else click_play_button(si);
		return;
		case ' ': if (si->PlayID != -1) click_pause_button(si);
		return;
		case 0x1b: select_section(si, si->sel, e->modifiers); return; /* ESC */
		case 0x1c: d = -1; break; /* left */
		case 0x1d: d = 1; break; /* right */
		case 0x1e: d = -si->nSectionsW; break; /* up */
		case 0x1f: d = si->nSectionsW; break; /* down */
		default: return;
	}
	if ((e->modifiers & controlKey) && (d < -1 || d > 1)) {
		scrollInteg = si;
		vertical_action(si->scrollBar,
			((d > 0)? kControlPageDownPart : kControlPageUpPart));
	} else {
		if (si->sel >= 0) d += si->sel;
		else if (si->PlayID >= 0) d = si->PlayID;
		else d = (si->scroll / si->barHeight) * si->nSectionsW;
		select_section(si, d, 0);
	}
}
void integ_modified(SBIntegrator *si) {
	if (si->fmode == NotModifiedFile) {
		si->fmode = ModifiedFile;
		EnableMenuItem(GetMenuHandle(mFile), iSaveFile);
		SetWindowModified(si->win, true);
	}
}
Boolean set_integ_pm_rect(SBIntegrator *si, Rect *r) {
	if (!si) return false;
	if (playID < 0 || playID >= si->nBars)
		return false;
	r->top = (playID / si->nSectionsW) * si->barHeight - si->scroll;
	r->bottom = r->top + SubWinPHeight / si->displayScale;
	r->top += SelFrameWidth; r->bottom -= SelFrameWidth;
	if (r->bottom < 0 || r->top > si->winHeight) return false;
	if (r->top < 0) r->top = 0;
	if (r->bottom > si->winHeight) r->bottom = si->winHeight;
	r->left = (playID % si->nSectionsW) * si->barWidth + SelFrameWidth;
	OffsetRect(r, 0, intWinTopSpace);
	return true;
}
void grow_integrator(SBIntegrator *si, EventRecord *e) {
	Rect	r;
	long	result;
	if (!si) return;
	r.left = IntWindowWidth+ScrollBarWidth-1;
	r.right = r.left + 1;
	r.top = PartHeight + intWinTopSpace;
	r.bottom = si->nSectionsH * si->barHeight + intWinTopSpace + GrowIconSize + 1;
	result = GrowWindow(si->win, e->where, &r);
	if (result == 0) return;
	resize_integrator(si, HiWord(result));
}
static GDHandle window_to_screen(WindowPtr win, Rect *pr) {
	Rect	wr;
	Point	pt;
	GDHandle	gdh;
	GetWindowPortBounds(win, &wr);
	pt.h = (wr.left + wr.right) / 2;
	pt.v = (wr.top + wr.bottom) / 2;
	SetPort(myGetWPort(win));
	LocalToGlobal(&pt);
	for (gdh = GetDeviceList(); gdh; gdh = GetNextDevice(gdh)) {
		wr = (*gdh)->gdRect;
		if (wr.top <= pt.v && pt.v < wr.bottom
		 && wr.left <= pt.h && pt.h < wr.right)
			{ *pr = wr; return gdh; }
	}
	return nil;
}
void zoom_integ(SBIntegrator *si) {
	static short	TopOffset = -1, BottomOffset;
	GDHandle	gdh;
	Rect	rg;
	Point	pt;
	short	cwinHeight, scoreHeight, maxHeight;
	if (!si) return;
	SetPort(myGetWPort(si->win));
	GetWindowPortBounds(si->win, &rg);
	cwinHeight = rg.bottom;
	if (TopOffset < 0) {
		RgnHandle	rgn = NewRgn();
		GetWindowRegion(si->win, kWindowStructureRgn, rgn);
		GetRegionBounds(rgn, &rg);
		pt.h = pt.v = 0; LocalToGlobal(&pt);
		TopOffset = pt.v - rg.top;
		pt.v = cwinHeight; LocalToGlobal(&pt);
		BottomOffset = rg.bottom - pt.v;
		DisposeRgn(rgn);
	}
	gdh = window_to_screen(si->win, &rg);
	if (!gdh) return;
	scoreHeight = si->nSectionsH * si->barHeight + intWinTopSpace + GrowIconSize;
	maxHeight = rg.bottom - rg.top - TopOffset - BottomOffset;
	if (TestDeviceAttribute(gdh, mainScreen)) maxHeight -= GetMBarHeight();
	if (scoreHeight > maxHeight) scoreHeight = maxHeight;
	if (si->origWinHeight == 0 || cwinHeight != scoreHeight) {
		si->origWinHeight = cwinHeight;
		pt.h = pt.v = 0; LocalToGlobal(&pt);
		si->winPosition = pt;
		if (pt.v + scoreHeight + BottomOffset > rg.bottom)
			MoveWindow(si->win, pt.h, rg.bottom - scoreHeight - BottomOffset, false);
		resize_integrator(si, scoreHeight);
	} else {
		if (scoreHeight > si->origWinHeight) {
			MoveWindow(si->win, si->winPosition.h, si->winPosition.v, false);
			resize_integrator(si, si->origWinHeight);
		}
		si->origWinHeight = 0;
	}
}
void score_edit(short item) {
	SBIntegrator	*si = FindIntegrator(TopWindow);
	if (!si) return;
	switch (item) {
		case iNoteUp: score_note_shift(si, 1); break;
		case iNoteDown: score_note_shift(si, -1); break;
		case iShiftRight: integ_shift_right(si); break;
		case iShiftLeft: integ_shift_left(si); break;
		case iExpand: expand_integ(si, 1); break;
		case iShrink: shrink_integ(si); break;
	}
}
void integ_select_all(SBIntegrator *si) {
	integScorePtr	scorep;
	short	i, n;
	if (!si) return;
	HLock((Handle)si->scoreHandle);
	scorep = *(si->scoreHandle);
	for (i = n = 0; i < si->nBars; i ++) if (!ItgSelectedP(scorep,i)) {
		n ++; scorep[i].flag |= ItgSelectedFlag;
	}
	if (n == 0) {
		for (i = 0; i < si->nBars; i ++) scorep[i].flag &= ~ItgSelectedFlag;
		si->nSelBars = 0;
	} else si->nSelBars = si->nBars;
	HUnlock((Handle)si->scoreHandle);
	update_integrator(si);
	activate_integ_sel2_ctrl(si);
	if (n) set_current_integ_pi(si);
	set_integ_bottom_text(si,true);
}
