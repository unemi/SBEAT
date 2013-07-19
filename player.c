#include  "decl.h"
#include	"dialog.h"
#include	"vers.h"

DialogRef	playerDlg = NULL;
WindowRef	playerWin = NULL;
long	CurrentTabID = 0;
static	Boolean	TempoChangeable = true;
extern	Boolean	PlayLoop, on_MacOS_X;
extern	WindowRef	TopWindow;
extern	BarInfo	*CurrentBI;
extern	BarInfoFlags	BarInfoUniq;

#define	Tempo		(CurrentBI->tempo)
#define	KeyScale	(CurrentBI->keyScale)
#define	KeyNote		(CurrentBI->keyNote)

#define	itgBIVary(x)	(TopWindow && GetWRefCon(TopWindow) == WIDIntegrator && bInfoVary(x))

static Boolean integ_mixed_tempo(void) { return itgBIVary(tempo); }
static Boolean integ_mixed_keyNote(void) { return itgBIVary(keyNote); }
static Boolean integ_mixed_keyScale(void) { return itgBIVary(keyScale); }
static Boolean integ_mixed_ETScale(void) { return itgBIVary(ETScale); }
static void set_slider_value(long v) {
	ControlHandle	control;
	GetDialogItemAsControl(playerDlg, STSlider, &control);
	SetControlValue(control, v);
}
static long get_slider_value(void) {
	ControlHandle	control;
	GetDialogItemAsControl(playerDlg, STSlider, &control);
	return GetControlValue(control);
}
static void set_text_value(long v, short id) {
	ControlHandle	control;
	unsigned char	text[256];
	NumToString(v, text);
	GetDialogItemAsControl(playerDlg, id, &control);
	SetDialogItemText((Handle)control, text);
}
static long get_text_value(short id) {
	ControlHandle	control;
	long	value;
	unsigned char	text[256];
	GetDialogItemAsControl(playerDlg, id, &control);
	GetDialogItemText((Handle)control, text);
	StringToNum(text, &value);
	return value;
}
static void value_changable(Boolean flag) {
	ControlHandle	control;
	short	v = flag? 0 : 255;
//	if (TempoChangeable == flag) return;
	GetDialogItemAsControl(playerDlg, STApply, &control);
	HiliteControl(control, v);
	if (flag && integ_mixed_tempo()) v = 255;
	GetDialogItemAsControl(playerDlg, STRevert, &control);
	HiliteControl(control, v);
	TempoChangeable = flag;
}
static void set_key_pict(void) {
	static PicHandle	pict[KeyMax+2] =
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	ControlHandle	control;
	short	pictID = integ_mixed_keyScale() ? KeyMax + 1 : (KeyScale & M8ScaleMask);
	GetDialogItemAsControl(playerDlg, SKPict, &control);
	if (!pict[pictID]) {
		pict[pictID] = GetPicture(KeyPictID + pictID);
		if (!pict[pictID]) return;
	}
	SetControlData(control, kControlPicturePart, kControlPictureHandleTag,
		sizeof(Handle), &pict[pictID]);
	DrawOneControl(control);
}
void move_win_right(WindowRef win) {
	extern	WindowRef	partoptionWin;
	static	WindowRef	*winList[]
		= { &partoptionWin, &playerWin, NULL };
	Rect box, rct; Point tl;
	short	y, w, h, i, yy;
	GDHandle	gdh = GetGDevice();
	RgnHandle	rgn = NewRgn();
	GetWindowRegion(win, kWindowStructureRgn, rgn);
	GetRegionBounds(rgn, &box);
	w = box.right - box.left; h = box.bottom - box.top;
	GetWindowRegion(win, kWindowContentRgn, rgn);
	GetRegionBounds(rgn, &rct);
	tl.h = rct.left - box.left; tl.v = rct.top - box.top;
	y = GetMBarHeight();
	if (on_MacOS_X) y += 2;
	for (i = 0; winList[i]; i ++)
	if (*winList[i] && win != *winList[i]) {
		GetWindowRegion(*winList[i], kWindowStructureRgn, rgn);
		yy = GetRegionBounds(rgn, &rct) ->bottom;
		if (on_MacOS_X) yy += 2;
		if (y < yy) y = yy;
	}
	box = (*gdh)->gdRect;
	if (y + h > box.bottom) y = box.bottom - h;
	if (on_MacOS_X) w += 2;
	MoveWindow(win, box.right-w+tl.h, y+tl.v, false);
	ShowWindow(win);
	DisposeRgn(rgn);
}
void set_key_note_txt(unsigned char g, unsigned char *txt) {
	static	unsigned char	note_str[20] = {0};
	short	idx = (g & 0x07) + 1;
	if (note_str[0] == 0) GetIndString(note_str, 128, 6);
	if (note_str[0] < 9) {
		txt[0] = 1; txt[1] = note_str[idx];
	} else {
		txt[0] = 2; BlockMove(&note_str[idx*2-1], &txt[1], 2);
	}
}
void setup_key_note_menu(MenuHandle mh, short keyItemID, short *oldKeyScale) {
	static	unsigned char	*scaleName[7] = {0}, nameStr[256];
	static	char	scaleIdx[] = {0,4,1,5,2,6,3,0,4,1,5,2,6,3,0};
	short	keyScale, i, j, k;
	unsigned char	*p, text[48];
	if (!CurrentBI) return;
	if ((keyScale = (KeyScale & M8ScaleMask)) == *oldKeyScale) return;
	if (!scaleName[0]) for (i = 0, p = nameStr; i < 7; i ++, p += p[0] + 1)
		{ GetIndString(p, 129, i + 1); scaleName[i] = p; }
	j = scaleIdx[keyScale];
	for (i = 0; i < 8; i ++, j = (j + 1) % 7) {
		set_key_note_txt(7 - i, text);
		k = text[0] + 1;
		text[k] = ':'; text[k+1] = ' ';
		BlockMove(scaleName[j] + 1, &text[k + 2], scaleName[j][0]);
		text[0] = scaleName[j][0] + k + 1;
		SetMenuItemText(mh, i + keyItemID, text);
	}
	*oldKeyScale = keyScale;
}
static void setup_player_key_note_menu(void) {
	static	MenuHandle	mh = NULL;
	static	short	oldKeyScale = -999;
	if (!mh) {
		ControlHandle	popup;
		Size	asize;
		OSErr	err;
		GetDialogItemAsControl(playerDlg, SKPopMenu, &popup);
		err = GetControlData(popup, kControlMenuPart,
			kControlPopupButtonMenuHandleTag, sizeof(mh), (Ptr)&mh, &asize);
		if (err) { error_msg("\pGet popup menu in setup_key_note_menu", err); return; }
	}
	setup_key_note_menu(mh, KeyNoteMin, &oldKeyScale);
}
static void setup_player_ETScale(void) {
	ControlHandle	control;
	short	value = integ_mixed_ETScale()? kControlCheckBoxMixedValue
	: ((CurrentBI->keyScale & ETScaleMask)?
		kControlCheckBoxCheckedValue : kControlCheckBoxUncheckedValue);
	GetDialogItemAsControl(playerDlg, SKECheckBox, &control);
	SetControlValue(control, value);
	GetDialogItemAsControl(playerDlg, SKEPane, &control);
	if (value == kControlCheckBoxUncheckedValue) ActivateControl(control);
	else DeactivateControl(control);
}
void setup_tempo_and_key(void) {
	ControlHandle	stp, skp, control;
	if (!playerDlg) return;
	GetDialogItemAsControl(playerDlg, STPane, &stp);
	GetDialogItemAsControl(playerDlg, SKPane, &skp);
	if (CurrentBI) {
		ActivateControl(stp);
		set_slider_value(Tempo);
		set_text_value(Tempo, STEdText);
		if (integ_mixed_tempo()) {
			GetDialogItemAsControl(playerDlg, STStText, &control);
			SetDialogItemText((Handle)control, "\p???");
			value_changable(true);
		} else {
			set_text_value(Tempo, STStText);
			value_changable(false);
		}
		ActivateControl(skp);
		set_key_pict();
		GetDialogItemAsControl(playerDlg, SKSlider, &control);
		SetControlValue(control, (KeyScale & M8ScaleMask));
		GetDialogItemAsControl(playerDlg, SKPopMenu, &control);
		setup_player_key_note_menu();
		SetControlValue(control, (integ_mixed_keyNote()? KeyNoteNotUniq : KeyNote));
		setup_player_ETScale();
	} else {
		DeactivateControl(stp);
		DeactivateControl(skp);
	}
}
void setup_player_cntrl(void) {
	ControlHandle	control;
	GetDialogItemAsControl(playerDlg, SFGroup, &control);
	SetControlValue(control, PlayLoop? 1 : 2);
	setup_tempo_and_key();
}
void set_small_font(DialogRef dlg, short startID, short n, short just) {
	short	i;
	ControlHandle	control;
	ControlFontStyleRec	style;
	style.flags = kControlUseSizeMask|kControlUseJustMask;
	style.size = SmallFontSizeX;
	style.just = just;
	for (i = 0; i < n; i ++) {
		if (GetDialogItemAsControl(dlg, i + startID, &control) == noErr)
			SetControlFontStyle(control, &style);
	}
}
void open_player(void) {
	if (playerWin) {
		BringToFront(playerWin);
		show_hide_window(playerWin, true);
		activate_root_cntl(playerWin);
	} else {
		Boolean	tagValue = true;
		ControlHandle	control;
		playerDlg = GetNewDialog(128, nil, kMoveToFront);
		if (!playerDlg) { error_msg("\pGetNewDialog", 128); return; }
		playerWin = GetDialogWindow(playerDlg);
		SetWRefCon(playerWin, WIDPlayer);
		if (on_MacOS_X) {
			static	short	textID[] = {
				kTabID, SFRepeat, SFPlayOnce,
				STEdText, STApply, STRevert,
#ifndef	JAPANESE_VERSION
				SKPopMenu,
#endif
				SKMenuTitle, SKECheckBox, 0};
			short	i;
			for (i = 0; textID[i]; i ++)
				set_small_font(playerDlg, textID[i], 1, teFlushDefault);
			set_small_font(playerDlg, STTicLabel, NSTTicLabels, teCenter);
		}
		GetDialogItemAsControl(playerDlg, STApply, &control);
		SetControlData(control, kControlButtonPart,
			kControlPushButtonDefaultTag, sizeof(Boolean), (Ptr)&tagValue);
		setup_player_cntrl();
		SelectDialogItemText(playerDlg, STEdText, 0, 5);
		move_win_right(playerWin);
	}
	CheckMenuItem(GetMenuHandle(mWindow), iPlayer, true);
}
static void apply_new_tempo(void) {
	long	tempo = get_slider_value();
	set_text_value(tempo, STEdText);
	set_text_value(tempo, STStText);
	if (Tempo != tempo || integ_mixed_tempo()) {
		SBIntegrator	*si;
		enque_binfo_history();
		Tempo = tempo;
		switch (GetWRefCon(TopWindow)) {
			case WIDIntegrator:
			si = FindIntegrator(TopWindow);
			if (si) {
				integScorePtr	scorep;
				short	i;
				Boolean	modified = false;
				HLock((Handle)si->scoreHandle);
				scorep = *si->scoreHandle;
				for (i = 0; i < si->nBars; i ++) if (ItgSelectedOnP(scorep,i)) {
					scorep[i].b.tempo = Tempo;
					modified = true;
				}
				HUnlock((Handle)si->scoreHandle);
				if (modified) integ_modified(si);
				BarInfoUniq.tempo = tempo;
			} break;
			case WIDField: field_modified(FindField(TopWindow)); break;
			case WIDgedit: gedit_modified(FindGedit(TopWindow));
		}
		play_restart();
	}
	value_changable(false);
}
void player_text_changed(void) {
	long	tempo;
/*	should check which item is focused if there are more than one EditTexts. */
	switch (CurrentTabID) {
		case 1:		/* Tempo */
		tempo = get_text_value(STEdText);
		if (tempo < TempoMin) tempo = TempoMin;
		else if (tempo > TempoMax) tempo = TempoMax;
		set_slider_value(tempo);
		if (!integ_mixed_tempo()) value_changable(tempo != Tempo);
	}
}
Boolean keyin_player(EventRecord *e) {
	short	c;
	if (!playerDlg || CurrentTabID != 1) return false;
	c = e->message & charCodeMask;
	if (c == '\r') { if (!TempoChangeable) return false; }
	else if (c != 0x08
	 && !(0x1c <= c && c <= 0x1f) && !('0' <= c && c <= '9')) return false;
	if (e->modifiers & optionKey) return false;
	return check_player(e);
}
Boolean click_player(EventRecord *e) {
//	if (playerDlg != FrontWindow()) SelectWindow(playerDlg);
	return check_player(e);
}
static void change_player_tab(void) {
	ControlHandle	control;
	long	value;
	short	i;
	static	struct { short paneID, teID; }
		pane[] = {{SFGroup, 0}, {STPane, STEdText}, {SKPane, 0}, {0, 0}};
	GetDialogItemAsControl(playerDlg, kTabID, &control);
	value = GetControlValue(control) - 1;
	if (CurrentTabID == value) return;
	CurrentTabID = value;
	for (i = 0; pane[i].paneID; i ++) {
		GetDialogItemAsControl(playerDlg, pane[i].paneID, &control);
		if (i == value) ShowControl(control);
		else HideControl(control);
		if (pane[i].teID) {
			if (i == value) ShowDialogItem(playerDlg, pane[i].teID);
			else HideDialogItem(playerDlg, pane[i].teID);
		}
	}
}
static void	click_play_mode(void) {
	ControlHandle	control;
	Boolean	newval;
	GetDialogItemAsControl(playerDlg, SFGroup, &control);
	newval = (GetControlValue(control) == 1);
	if (newval != PlayLoop) {
		PlayLoop = newval;
		change_play_loop();
	}
}
static void click_tempo_slider(dialogSelectRec *r) {
	long	tempo, oldv, textv;
	oldv = tempo = GetControlValue(r->control);
	textv = get_text_value(STEdText);
	if (textv == tempo) switch (r->partCode) {
		case kControlPageUpPart: tempo -= 10; break;
		case kControlPageDownPart: tempo += 10; break;
		default: return;
	}
	if (tempo < TempoMin) tempo = TempoMin;
	else if (tempo > TempoMax) tempo = TempoMax;
	if (tempo != oldv) SetControlValue(r->control, tempo);
	set_text_value(tempo, STEdText);
	if (!integ_mixed_tempo()) value_changable(tempo != Tempo);
}
static void keyScale_changed(Boolean ETSp) {
	SBIntegrator	*si;
	switch (GetWRefCon(TopWindow)) {
		case WIDIntegrator:
		si = FindIntegrator(TopWindow);
		if (si) {
			integScorePtr	scorep;
			short	i;
			Boolean	modified = false;
			HLock((Handle)si->scoreHandle);
			scorep = *si->scoreHandle;
			for (i = 0; i < si->nBars; i ++) if (ItgSelectedOnP(scorep,i)) {
				if (!ETSp) scorep[i].b.keyScale = KeyScale;
				else if (KeyScale & ETScaleMask) scorep[i].b.keyScale |= ETScaleMask;
				else scorep[i].b.keyScale &= ~ETScaleMask;
				modified = true;
			}
			HUnlock((Handle)si->scoreHandle);
			if (modified) integ_modified(si);
			if (!ETSp) BarInfoUniq.keyScale = KeyScale;
			else BarInfoUniq.ETScale = 1;
		} break;
		case WIDField: field_modified(FindField(TopWindow)); break;
		case WIDgedit: gedit_modified(FindGedit(TopWindow));
	}
	play_restart();
}
static void click_key_slider(dialogSelectRec *r) {
	long	key, v;
	key = v = GetControlValue(r->control);
	if (key == KeyScale) switch (r->partCode) {
		case kControlPageUpPart: key --; break;
		case kControlPageDownPart: key ++; break;
		default: if (!integ_mixed_keyScale()) return;
	}
	if (key < KeyMin) key = KeyMin;
	else if (key > KeyMax) key = KeyMax;
	if (key != v) SetControlValue(r->control, key);
	enque_binfo_history();
	KeyScale = key;
	set_key_pict();
	setup_player_key_note_menu();
	if (KeyNote != KeyNoteVariable) {
		ControlHandle	menu;
		GetDialogItemAsControl(playerDlg, SKPopMenu, &menu);
		DrawOneControl(menu);
	}
	keyScale_changed(false);
}
static void click_key_menu(void) {
	short	oldValue, newValue;
	ControlHandle	control;
	SBField	*sf;
	SBgedit	*sg;
	SBIntegrator	*si;
	integScorePtr	scorep;
	short	i;
	GrafPtr	oldPort;
	Boolean	modified;
	if (!CurrentBI || !TopWindow) return;
	GetDialogItemAsControl(playerDlg, SKPopMenu, &control);
	oldValue = KeyNote;
	if ((newValue = GetControlValue(control)) == oldValue) return;
	enque_binfo_history();
	KeyNote = newValue;
	switch (GetWRefCon(TopWindow)) {
		case WIDField: sf = FindField(TopWindow); if (!sf) return;
		devlop_all_score(sf);
		field_modified(sf); break;
		case WIDgedit: sg = FindGedit(TopWindow); if (!sg) return;
		if (ChordPart == sg->editPart) {
			extern	DialogPtr	geditDlg;
			GetDialogItemAsControl(geditDlg, GEmelodyID, &control);
			if (newValue == KeyNoteVariable) ActivateControl(control);
			else if (oldValue == KeyNoteVariable) DeactivateControl(control);
		}
		develop_score(&sg->ind, CurrentBI);
		update_gedit(sg->win);
		gedit_modified(sg); break;
		case WIDIntegrator: si = FindIntegrator(TopWindow); if (!si) return;
		HLock((Handle)si->scoreHandle);
		scorep = *si->scoreHandle;
		modified = false;
		GetPort(&oldPort); SetPort(GetWindowPort(TopWindow));
		for (i = 0; i < si->nBars; i ++) if (ItgSelectedOnP(scorep,i)) {
			scorep[i].b.keyNote = KeyNote;
			develop_score(&scorep[i].i, &scorep[i].b);
			draw_integ_ind(si, i, true);
			modified = true;
		}
		SetPort(oldPort);
		HUnlock((Handle)si->scoreHandle);
		if (modified) integ_modified(si);
		BarInfoUniq.keyNote = KeyNote;
		break;
		default: return;
	}
	play_restart();
}
static void click_ETS_check_box(void) {
	ControlHandle	cbox, upane;
	short	newValue;
	GetDialogItemAsControl(playerDlg, SKECheckBox, &cbox);
	newValue = (GetControlValue(cbox) == 0);
	SetControlValue(cbox, newValue);
	GetDialogItemAsControl(playerDlg, SKEPane, &upane);
	enque_binfo_history();
	if (newValue) { DeactivateControl(upane); KeyScale |= ETScaleMask; }
	else { ActivateControl(upane); KeyScale &= ~ETScaleMask; }
	keyScale_changed(true);
}
static Boolean dialog_keyin(EventRecord *e, DialogPtr dlg, dialogSelectRec *r) {
	Point	p;
	Rect	b;
	if (GetKeyboardFocus(GetDialogWindow(dlg), &r->control)) return false;
	GetControlBounds(r->control, &b);
	p.h = (b.left + b.right) / 2; p.v = (b.top + b.bottom) / 2;
	r->itemID = FindDialogItem(dlg, p) + 1;
	r->partCode = HandleControlKey(r->control, e->message,
		e->message & charCodeMask, e->modifiers);
	return true;
}
static Boolean dialog_click(EventRecord *e, DialogPtr dlg, dialogSelectRec *r) {
	GrafPtr	oldPort;
	Point	p = e->where;
	GetPort(&oldPort); SetPort(myGetDPort(dlg)); GlobalToLocal(&p);
	if (!(r->itemID = FindDialogItem(dlg, p) + 1)) return false;
	if (GetDialogItemAsControl(dlg, r->itemID, &r->control)) return false;
	if (!IsControlActive(r->control)) return false;
	r->partCode = HandleControlClick(r->control, p, e->modifiers, (ControlActionUPP)(-1));
	return (r->partCode != 0);
}
Boolean dialog_select(EventRecord *e, DialogPtr dlg, dialogSelectRec *r) {
	switch (e->what) {
		case keyDown: case autoKey: return dialog_keyin(e, dlg, r);
		case mouseDown: return dialog_click(e, dlg, r);
		default: return false;
	}
}
Boolean check_player(EventRecord *e) {
	dialogSelectRec	r;
	switch (e->what) {
		case keyDown: case autoKey:
		if (CurrentTabID != 1) return false;	/* key-in is useful only tempo tab. */
		if ((e->message & charCodeMask) == '\r' && TempoChangeable) {
			hilite_button(playerDlg, STApply);
			apply_new_tempo();
			return true;
		}
	}
	if (!dialog_select(e, playerDlg, &r)) return true;
	switch (e->what) {
		case keyDown: case autoKey:
		player_text_changed(); break;
		case mouseDown: switch (r.itemID) {
			case kTabID: change_player_tab(); break;
			case SFGroup: click_play_mode(); break;
			case STSlider: click_tempo_slider(&r); break;
			case STApply: apply_new_tempo(); break;
			case STRevert:
				set_slider_value(Tempo);
				set_text_value(Tempo, STEdText);
				value_changable(false);
			break;
			case SKSlider: click_key_slider(&r); break;
			case SKPopMenu: click_key_menu(); break;
			case SKECheckBox: click_ETS_check_box();
		}
	}
	return true;
}
void close_player(void) {
	show_hide_window(playerWin, false);
	CheckMenuItem(GetMenuHandle(mWindow), iPlayer, false);
}
