#include  <stdio.h>
#include	<string.h>
#include	"decl.h"
#include	"dialog.h"
#define	CntlThis	1
#define	CntlNew		2
#define	CntlSpeakerOff	3
#define	CntlSpeakerOn	4
#define	CntlSelect	(CntlSpeakerOn+1)
#define	CntlProtect	(CntlSpeakerOn+2)
extern	WindowPtr	TopWindow, PlayWindow;
extern	Boolean	PlayAll, PlayLoop;
extern	Boolean	on_MacOS_X;
extern	unsigned char	displayPartID[], displayQueue[];

#define	MaxWinMenuItems	32
static	short	WindowCount = 0;
static	WindowPtr	WindowsInMenu[MaxWinMenuItems];

void set_win_menu_item(WindowPtr window) {
	static unsigned char	fieldName[16] = {0}, scoreName[16] = {0};
	MenuHandle	mh = GetMenuHandle(mWindow);
	short	k;
	unsigned char	*p, c, title[63];
	for (k = 0; k < WindowCount; k ++)
		if (WindowsInMenu[k] == window) break;
	if (k >= WindowCount) {
		if (WindowCount >= MaxWinMenuItems) return;
		title[0] = 1; title[1] = 'A';
		AppendMenu(mh, title);
		WindowsInMenu[WindowCount ++] = window;
	}
	switch (GetWRefCon(window)) {
		case WIDField: if (!fieldName[0]) GetIndString(fieldName, 128, 16);
		BlockMove(fieldName, title, fieldName[0] + 1);
		p = &title[fieldName[0]]; break;
		case WIDIntegrator: if (!scoreName[0]) GetIndString(scoreName, 128, 17);
		BlockMove(scoreName, title, scoreName[0] + 1);
		p = &title[scoreName[0]]; break;
		default: title[0] = 0; p = title;
	}
	c = *p;
	GetWTitle(window, p);
	if (p != title) { title[0] += *p; *p = c; }
	SetMenuItemText(mh, k + iWindowList, title);
}
static void delete_win_menu_item(WindowPtr window) {
	short	i;
	for (i = 0; i < WindowCount; i ++)
		if (WindowsInMenu[i] == window) break;
	if (i >= WindowCount) return;
	DeleteMenuItem(GetMenuHandle(mWindow), i + iWindowList);
	for (i ++; i < WindowCount; i ++) WindowsInMenu[i - 1] = WindowsInMenu[i];
	WindowCount --;
}
void select_win_menu_item(short item) {
	if (item < 0 || item >= WindowCount) return;
	SelectWindow(WindowsInMenu[item]);
}
void check_win_menu_item(WindowPtr window) {
	short	i;
	MenuHandle	mh = GetMenuHandle(mWindow);
	for (i = 0; i < WindowCount; i ++)
		CheckMenuItem(mh, i + iWindowList, (WindowsInMenu[i] == window));
}	
static void set_button_icon(ControlHandle control, Boolean flag,
	short iconOn, short iconOff, short refOn, short refOff) {
	ControlButtonContentInfo	cinfo;
	cinfo.contentType = kControlContentCIconRes;
	cinfo.u.resID = flag? iconOn : iconOff;
	SetControlData(control, kControlEntireControl,
		kControlBevelButtonContentTag, sizeof(cinfo), &cinfo);
	if (refOn != refOff) SetControlReference(control, (flag? refOn : refOff));
	HiliteControl(control, (flag? kControlButtonPart : 0));
	if (!flag) DrawOneControl(control);
}
static void set_protect_icon(ControlHandle control, Boolean flag) {
	set_button_icon(control, flag,
		IcnProtectOn, IcnProtectOff, CntlProtect, CntlProtect);
}
static void set_speaker_icon(ControlHandle control, Boolean flag) {
	set_button_icon(control, flag,
		IcnSpeakerOn, IcnSpeakerOff, CntlSpeakerOn, CntlSpeakerOff);
}
void clear_speaker_icon(SBField *sf) {
	if (!sf) return;
	if (sf->PlayAll) {
		set_speaker_icon(sf->all_speaker, false);
		sf->PlayAll = PlayAll = false;
	} else if (sf->BtnOnID >= 0) {
		set_speaker_icon(sf->speaker[sf->BtnOnID], false);
		sf->PlayID = sf->BtnOnID = -1;
	}
}
static void set_btn_title(ControlHandle button, unsigned char *s, short idx) {
	if (!s[0]) GetIndString(s, 128, idx);
	SetControlTitle(button, s);
}
static WindowPtr make_field_window(unsigned char *title) {
	WindowPtr	window = GetNewCWindow(kBaseResID, nil, kMoveToFront);
	if (!window) {
		error_msg("\pGetNewCWindow in make_field_window",0);
		return NULL;
	}
	SetWRefCon(window, WIDField);
	set_drag_callback(window);
	SizeWindow(window, WinWidth, WinHeight, true);
	SetWTitle(window, title);
	SetWindowProxyCreatorAndType(window, CreatorCode, FileTypeField, 0 /*kOnSystemDisk*/);
	EnableMenuItem(GetMenuHandle(mFile), iSaveAsSMF);
	set_win_menu_item(window);
	return window;
}
#define	FieldTitlePad	4
#define	FieldBtnWidth	48
static ControlHandle create_title_sub_text(
	SBField *sf, short left, short width, unsigned char *str) {
	Rect	r;
	ControlHandle	control;
	SetRect(&r, left - 1, FldControlPad + 2, left + width + 1,
		FldControlPad + FldControlH);
	control = NewControl(sf->win,&r,"\p",true,0,0,0,kControlStaticTextProc,0);
	if (control) SetControlData(control, kControlEntireControl,
		kControlStaticTextTextTag, str[0], str + 1);
	return control;
}
#if	TARGET_API_MAC_CARBON
void set_control_help_tag(ControlHandle control,
	short side, short resID, short index) {
	static struct HMHelpContentRec hc;
	hc.tagSide = side;
	hc.content[0].contentType = kHMStringResContent;
	hc.content[0].u.tagStringRes.hmmResID = resID;
	hc.content[0].u.tagStringRes.hmmIndex = index;
	hc.content[1] = hc.content[0];
	hc.content[1].u.tagStringRes.hmmResID = resID + 1;
	HMSetControlHelpContent(control, &hc);
}
#else
void set_control_help_tag(ControlHandle, short, short, short) { }
#endif
static void setup_field_controls(SBField *sf) {
	static	short	FieldMessageBase = 16, NextTTLp1, NextTTLp2, NextTTLp3,
		NextTTLw1, NextTTLw2, NextTTLw3;
	static	unsigned char	NextTitle1[32], NextTitle2[2] = "\p/", NextTitle3[32],
		PlayAllTitle[32];
	static	short	NextBTNpThis = -1, NextBTNpNew;
	static	unsigned char	selBoxTitle[32], sThis[8], sNew[8];
	ControlFontStyleRec	fstyle;
	OSErr	err;
	short	i, j, k, a, x, y;
	Rect	r;
	ControlHandle	container, sub[7];
	SInt16	si16;
	if (NextBTNpThis < 0) {
		GetIndString(PlayAllTitle, 128, 7);
		GetIndString(NextTitle1, 128, 8);
		GetIndString(NextTitle3, 128, 9);
		GetIndString(sThis, 128, 10);
		GetIndString(sNew, 128, 11);
		TextSize(0);
		NextTTLw3 = StringWidth(NextTitle3);
		NextTTLp3 = WinWidth - FieldTitlePad - NextTTLw3;
		NextBTNpNew = NextTTLp3 - FieldTitlePad - FieldBtnWidth;
		NextTTLw2 = StringWidth(NextTitle2);
		NextTTLp2 = NextBTNpNew - FieldTitlePad - NextTTLw2;
		NextBTNpThis = NextTTLp2 - FieldTitlePad - FieldBtnWidth;
		NextTTLw1 = StringWidth(NextTitle1);
		NextTTLp1 = NextBTNpThis - FieldTitlePad - NextTTLw1;
	}
	if ((err = CreateRootControl(sf->win, &sf->root)) != noErr) {
		error_msg("\pCreateRootControl for field", err); ExitToShell();
	}
	for (i = k = 0, y = WinTopSpace; i < FieldNRows; i ++, y += SubWinYUnit) {
	if (on_MacOS_X) {
		SetRect(&r, 0, y, WinWidth, y + IndControlH);
		container = NewControl(sf->win,&r,"\p",true,0,0,0,kControlWindowHeaderProc,0);
	}
	for (j = 0, x = WinLeftSpace; j < FieldNColumns;
		k ++, j ++, x += SubWinXUnit) {
		if (!on_MacOS_X) {
			SetRect(&r, x, y, x + SubWinWidth + SubWinBorder*2, y + IndControlH);
			container = NewControl(sf->win,&r,"\p",true,0,0,0,
				kControlWindowHeaderProc,0);
		}
		SetRect(&r, x+FldControlPad, y+FldControlPad,
			x+FldControlH+FldControlPad, y+FldControlH+FldControlPad);
		sf->speaker[k] = sub[0] = NewControl(sf->win,&r,"\p",true,
			0,kControlBehaviorPushbutton|kControlContentCIconRes,IcnSpeakerOff,
			kControlBevelButtonNormalBevelProc, CntlSpeakerOff);
		set_control_help_tag(sf->speaker[k], kHMOutsideTopLeftAligned, 160, 1);
		r.left = r.right + FldControlPad;
		r.right = r.left + FldControlH;
		sf->ptcButton[k] = sub[1] = NewControl(sf->win,&r,"\p",true,
			0,kControlBehaviorPushbutton|kControlContentCIconRes,IcnProtectOff,
			kControlBevelButtonNormalBevelProc, CntlProtect);
		set_control_help_tag(sf->ptcButton[k], kHMOutsideTopLeftAligned, 160, 2);
		r.left = r.right + FldControlPad;
		r.right = x + SubWinWidth + (SubWinBorder - FldControlPad) * 2;
		GetIndString(selBoxTitle, 128, 4);
		sf->selButton[k] = sub[2] = NewControl(sf->win,&r,selBoxTitle,true,
			0,0,1,kControlCheckBoxProc,CntlSelect);
		for (a = 0; a < 3; a ++) if ((err = EmbedControl(sub[a], container)) != noErr)
			{ error_msg("\pEmbedControl ind", err); ExitToShell(); }
	}}
	SetRect(&r, NextBTNpThis, FldControlPad,
		NextBTNpThis + FieldBtnWidth, FldControlPad + FldControlH);
	sub[0] = NewControl(sf->win,&r,sThis,true,0,0,1,
		kControlBevelButtonNormalBevelProc,CntlThis);
	OffsetRect(&r, NextBTNpNew - NextBTNpThis, 0);
	sub[1] = NewControl(sf->win,&r,sNew,true,0,0,1,
		kControlBevelButtonNormalBevelProc,CntlNew);
	if (on_MacOS_X) {
		fstyle.flags = kControlUseSizeMask;
		fstyle.size = ButtonFontSizeX;
		SetControlFontStyle(sub[0], &fstyle);
		SetControlFontStyle(sub[1], &fstyle);
	}
	sub[2] = create_title_sub_text(sf, NextTTLp1, NextTTLw1, NextTitle1);
	sub[3] = create_title_sub_text(sf, NextTTLp2, NextTTLw2, NextTitle2);
	sub[4] = create_title_sub_text(sf, NextTTLp3, NextTTLw3, NextTitle3);
	SetRect(&r, NextTTLp1 - 2, 1, WinWidth - 1, WinTopSpace - 1);
	sf->next_pane = NewControl(sf->win,&r,"\p",true,kControlSupportsEmbedding,
		0,0,kControlUserPaneProc,0);
	SetRect(&r, -1, -1, WinWidth+1, WinTopSpace+1);
	container = NewControl(sf->win,&r,"\p",true,0,0,0,kControlWindowHeaderProc,0);
	SetRect(&r, FldControlPad, FldControlPad,
		NextTTLp1 - FldControlPad - FieldBtnWidth, FldControlPad + FldControlH);
	sf->all_speaker = NewControl(sf->win,&r,PlayAllTitle,true,
		0,kControlBehaviorPushbutton|kControlContentCIconRes,IcnSpeakerOff,
		kControlBevelButtonNormalBevelProc,CntlSpeakerOff);
	si16 = kControlBevelButtonAlignLeft;
	SetControlData(sf->all_speaker, kControlButtonPart,
		kControlBevelButtonGraphicAlignTag, sizeof(si16), &si16);
	fstyle.flags = kControlUseSizeMask;
	fstyle.size = on_MacOS_X? ButtonFontSizeX : SmallFontSize9;
	SetControlFontStyle(sf->all_speaker, &fstyle);
	if ((err = EmbedControl(sf->next_pane, container)) != noErr)
		{ error_msg("\pEmbedControl next pane", err); ExitToShell(); }
	if ((err = EmbedControl(sf->all_speaker, container)) != noErr)
		{ error_msg("\pEmbedControl all speaker", err); ExitToShell(); }
	for (a = 0; a < 5; a ++)
	if ((err = EmbedControl(sub[a], sf->next_pane)) != noErr)
		{ error_msg("\pEmbedControl \"this\" button", err); ExitToShell(); }
	ShowWindow(sf->win);
/*	{
		FSSpec	fss;
		FSMakeFSSpec(0,0,"\pCntlHierachy",&fss);
		DumpControlHierarchy(sf->win, &fss);
	}*/
}
void untitled_str(unsigned char *title, long *num) {
	GetIndString(title, 128, 1);
	if (*num > 1) {
		short	i = title[0];
		unsigned char	c = title[i];
		NumToString(*num, title + i);
		title[0] += title[i];
		title[i] = c;
	}
	(*num) ++;
}
void setup_new_field(SBField *sf, unsigned char *tl) {
	static	long	NewFieldNum = 1;
	unsigned char	title[64];
	if (!tl) {
		untitled_str(title, &NewFieldNum);
		tl = title;
	}
	sf->win = make_field_window(tl);
	sf->fmode = NewFile;
	sf->refNum = 0;
	sf->PlayID = sf->BtnOnID = -1;
	sf->PlayAll = false;
	sf->prevOp = GOpNothing;
	SetWindowModified(sf->win, true);
	setup_field_controls(sf);
}
static void setup_new_field_from_origWin(SBField *sf, WindowRef origWin) {
	unsigned char	len, c, title[64];
	GetWTitle(origWin, title);
	len = title[0]; c = title[len];
	if (len > 1 && title[len-1] == ',' &&
		(('0' <= c && c <= '9') || ('A' <= c && c <= 'Z')
	  || ('a' <= c && c <= 'z'))) switch (c) {
		case '9': title[len] = 'A'; break;
		case 'Z': title[len] = 'a'; break;
		case 'z': title[len] = '0'; break;
		default: title[len] ++;
	} else { title[len+1] = ','; title[len+2] = '1'; title[0] += 2; }
	setup_new_field(sf, title);
}
void NewFieldCB(void) {
	SBField	*sf = new_field_rec();
	if (!sf) return;
	set_default_poption(sf);
	pop_init(sf);
	setup_new_field(sf, NULL);
}
void next_in_new(SBField *orig_sf) {
	SBField	*new_sf = new_field_rec();
	short	i;
	if (!new_sf) return;
	for (i = 0; i < PopulationSize; i ++) {
		new_sf->pop[i] = orig_sf->pop[i];
		new_sf->sel[i] = 0;
	}
	new_sf->b = orig_sf->b;
	new_sf->v = orig_sf->v;
	if (orig_sf->refNum == 0) setup_new_field(new_sf, NULL);
	else setup_new_field_from_origWin(new_sf, orig_sf->win);
	next_generation(orig_sf, new_sf);
}
Boolean equiv_fss(FSSpec *fs1, FSSpec *fs2) {
	short	i;
	if (fs1->parID != fs2->parID) return false;
	for (i = 0; i <= fs1->name[0]; i ++)
		if (fs1->name[i] != fs2->name[i]) return false;
	return true;
}
void open_field(FSSpec *fss) {
	short	i;
	SBField	*sf;
	extern	SBField	*Fields;
	for (sf = Fields; sf; sf = sf->post)
		if (equiv_fss(&sf->fsSpec, fss))
			{ SelectWindow(sf->win); return; }
	sf = new_field_rec();
	if (!sf) return;
	if (!read_options(fss, sf)) return;
	for (i = 0; i < PopulationSize; i ++) sf->sel[i] = 0;
	sf->win = make_field_window(fss->name);
	sf->fmode = NotModifiedFile;
	sf->fsSpec = *fss;
	sf->PlayID = sf->BtnOnID = -1;
	sf->PlayAll = false;
	SetWindowModified(sf->win, false);
	setup_field_controls(sf);
}
static Boolean save_field_ref(SBField *sf) {
	long	n = sizeof(Individual) * PopulationSize;
	SetFPos(sf->refNum, fsFromStart, 0L);	/* rewind */
	save_options(sf->refNum);
	if (FSWrite(sf->refNum, &n, sf->pop) != noErr) return false;
	sf->fmode = NotModifiedFile;
	SetWindowModified(sf->win, false);
	return true;
}
Boolean save_field_as(SBField *sf) {
	static	unsigned char	title[32] = {0};
	if (!sf) return false;
	if (!title[0]) GetIndString(title, 128, 12);
	if (save_as_setup(sf->win, &sf->fsSpec, &sf->refNum,
		FileTypeField, title))
		return save_field_ref(sf);
	else return false;
}
Boolean save_field(SBField *sf) {
	if (!sf) return false;
	switch (sf->fmode) {
		case NewFile: return save_field_as(sf);
		case ModifiedFile: return save_field_ref(sf);
	}
	return true;
}
void close_sb_window(WindowPtr win) {
	if (PlayWindow == win) stop_tune();
	delete_win_menu_item(win);
	DisposeWindow(win);
	if (TopWindow == win) reset_topwindow();
	check_no_window();
}
void close_field(SBField *sf) {
	extern	SBgedit	*Gedits;
	SBgedit	*sg;
	WindowPtr	win;
	if (!sf) return;
	win = sf->win;
	if (sf->fmode != NotModifiedFile) switch (save_change_alert(win, 1)) {
		case itemCancel: return;
		case itemOK: if (!save_field(sf)) return;
	}
	if (sf->refNum > 0) FSClose(sf->refNum);
	remove_drag_callback(win);
	KillControls(win);
	rm_history(win);
	for (sg = Gedits; sg; sg = sg->post)
		if (sg->sf == sf) sg->sf = NULL;
	sf->win = NULL;
	free_field(sf);
	close_sb_window(win);
}
void field_modified(SBField *sf) {
	if (sf->fmode == NotModifiedFile) {
		sf->fmode = ModifiedFile;
		EnableMenuItem(GetMenuHandle(mFile), iSaveFile);
		SetWindowModified(sf->win, true);
	}
}
short find_primary(SBField *sf) {
	short	i;
	if (!sf) return -1;
	for (i = 0; i < PopulationSize; i ++)
		if (SelectedP(sf,i)) return i;
	return -1;
}
void field_ind_changed(SBField *sf, short id) {
	SetPort(myGetWPort(sf->win));
	draw_field_ind(sf, id);
	field_modified(sf);
	revise_tune_queue(sf->win, id);
	if (sf->sel[id] & HadSelectedFlag) {
		sf->sel[id] &= ~ HadSelectedFlag;
		check_disable_DoItAgain(sf);
	}
}
void activate_field_controls(SBField *sf, Boolean flag) {
	if (!sf) return;
	if (flag) {
		ActivateControl(sf->root);
		setup_current_po(&sf->b, &sf->v);
	} else DeactivateControl(sf->root);
}
static void play_ind(SBField *sf, short id) {
	if (id >= 0) sf->PlayID = id;
	else if (sf->PlayID < 0) sf->PlayID = 0;
	else if (PlayLoop) sf->PlayID = (sf->PlayID + 1) % PopulationSize;
	else sf->PlayID = 0;
	if (sf->win == TopWindow) {
		if (!PlayLoop) stop_tune();
		PlayAll = sf->PlayAll;
		play_sequence(sf->win, sf->PlayID);
	}
}
static void clickon_speaker_icon(SBField *sf, ControlHandle button, short id) {
	set_speaker_icon(button, true);
	if (sf->PlayAll && button != sf->all_speaker) {
		set_speaker_icon(sf->all_speaker, false);
		sf->PlayAll = false;
	} else if (sf->BtnOnID >= 0 && button != sf->speaker[sf->BtnOnID])
		set_speaker_icon(sf->speaker[sf->BtnOnID], false);
	if (button == sf->all_speaker) {
		sf->PlayAll = true; play_ind(sf, -1); sf->BtnOnID = -1;
	} else { play_ind(sf, id); sf->BtnOnID = id; }
}
static void clickoff_speaker_icon(SBField *sf, ControlHandle button) {
	set_speaker_icon(button, false);
	sf->PlayID = sf->BtnOnID = -1;
	sf->PlayAll = false;
	if (sf->win == TopWindow) stop_tune();
}
void flip_sel_mode(SBField *sf, short id) {
	sf->sel[id] ^= SelectedFlag;
	draw_frame(sf, id);
	SetControlValue(sf->selButton[id], (SelectedP(sf,id)? 1 : 0));
}
static void change_sel_mode(SBField *sf, short id, EventRecord *e) {
	static	unsigned long	prevTime = 0;
	if (e->when - prevTime < GetDblTime()) {
		short	i;
		for (i = 0; i < PopulationSize; i ++) if (i != id) {
			if (SelectedP(sf, i)) flip_sel_mode(sf, i);
		}
		if (!SelectedP(sf, id)) flip_sel_mode(sf, id);
		next_generation(sf, NULL);
		prevTime = 0;
	} else {
		flip_sel_mode(sf, id);
		setup_field_menu(sf);
		prevTime = e->when;
	}
}
static void change_protect_mode(SBField *sf, short id) {
	sf->sel[id] ^= ProtectedFlag;
	draw_frame(sf, id);
	set_protect_icon(sf->ptcButton[id], ProtectedP(sf, id));
	setup_field_menu(sf);
}
static void click_ind(SBField *sf, short id, EventRecord *e) {
	switch (e->modifiers & (shiftKey|controlKey|optionKey|cmdKey)) {
		case shiftKey: change_sel_mode(sf, id, e); break;
//		case optionKey: change_protect_mode(sf, id); break;
		case 0: if (sf->BtnOnID != id)
			clickon_speaker_icon(sf, sf->speaker[id], id);
		else clickoff_speaker_icon(sf, sf->speaker[id]);
	}
}
Boolean draw_temp_drag_frame(Rect *r, Point where) {
	Boolean	dragp;
	Pattern	grayPat;
	GetQDGlobalsGray(&grayPat);
	PenSize(2, 2); PenPat(&grayPat);
	PenMode(patXor);
	FrameRect(r);
	dragp = WaitMouseMoved(where);
	FrameRect(r);
	PenNormal();
	return dragp;
}
void click_field(SBField *sf, EventRecord *e) {
	if (sf) {
		short	part, id, x, y;
		Point	p = e->where;
		ControlHandle	control;
		SetPort(myGetWPort(sf->win));
		GlobalToLocal(&p);
		id = ((y = p.v - WinTopSpace) / SubWinYUnit) * FieldNColumns
			+ (x = p.h - WinLeftSpace) / SubWinXUnit;
		control = FindControlUnderMouse(p, sf->win, &part);
		if (part && control) {
			long	cref = GetControlReference(control);
			if (!cref) return;
			HandleControlClick(control, p, e->modifiers, nil);
			switch (cref) {
				case CntlThis: next_generation(sf, NULL); break;
				case CntlNew: next_in_new(sf); break;
				case CntlSpeakerOff: clickon_speaker_icon(sf, control, id); break;
				case CntlSpeakerOn: clickoff_speaker_icon(sf, control); break;
				case CntlSelect: change_sel_mode(sf, id, e); break;
				case CntlProtect: change_protect_mode(sf, id); break;
			}
		} else if (x > 0 && y > 0) {
			y -= (y / SubWinYUnit) * SubWinYUnit + IndControlH;
			if (y < 0) SelectWindow(sf->win);
			else {
				Rect	r;
				short	part = (e->modifiers & optionKey)? y / PartHeight : -1;
				set_my_cursor(CURSclosed);
				field_part_to_rect(id, part, &r);
				if (draw_temp_drag_frame(&r, e->where)) drag_field_ind(sf, e);
				else {
					if (sf->win == TopWindow) click_ind(sf, id, e);
					else SelectWindow(sf->win);
					set_my_cursor(CURSgrabber);
				}
			}
		} else SelectWindow(sf->win);
	}
}
void keyin_field(SBField *sf, EventRecord *e) {
	short	d, ccode = e->message & charCodeMask;
	GrafPtr	oldGrf;
	if (!sf) return;
	if (sf->win != TopWindow) return;
	GetPort(&oldGrf); SetPort(myGetWPort(sf->win));
	switch (ccode) {
		case '\r': switch(e->modifiers & (optionKey|shiftKey|controlKey|cmdKey)) {
			case shiftKey: if (sf->PlayAll && sf->PlayID >= 0)
				change_sel_mode(sf, sf->PlayID, e);
			else if (sf->BtnOnID >= 0) change_sel_mode(sf, sf->BtnOnID, e);
			break;
			case 0: if (sf->PlayAll)
				clickoff_speaker_icon(sf, sf->all_speaker);
			else clickon_speaker_icon(sf, sf->all_speaker, -1);
		} break;
		case 0x08: case 0x1b: /* DELETE and ESC */
		if (sf->PlayAll) clickoff_speaker_icon(sf, sf->all_speaker);
		else if (sf->BtnOnID >= 0)
			clickoff_speaker_icon(sf, sf->speaker[sf->BtnOnID]);
		break;
		default: if (ccode >= 0x1c && ccode <= 0x1f) {
			if (sf->PlayAll && sf->PlayID >= 0) d = sf->PlayID;
			else if (sf->BtnOnID < 0) d = PopulationSize / 2;
			else {
				short	x = sf->BtnOnID % FieldNColumns,
						y = sf->BtnOnID / FieldNColumns;
				switch (ccode) {
					case 0x1c: /* left */
					x = (x - 1 + FieldNColumns) % FieldNColumns; break;
					case 0x1d: /* right */
					x = (x + 1) % FieldNColumns; break;
					case 0x1e: /* up */
					y = (y - 1 + FieldNRows) % FieldNRows; break;
					case 0x1f: /* down */
					y = (y + 1) % FieldNRows; break;
				}
				d = y * FieldNColumns + x;
			}
			clickon_speaker_icon(sf, sf->speaker[d], d);
		}
	}
	SetPort(oldGrf);
}
