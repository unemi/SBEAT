#include  <stdio.h>
#include	<string.h>
#include	<InternetConfig.h>
#include	<Balloons.h>
#include	"decl.h"
#define	WaitTics	30
WindowPtr	TopWindow;
long	AppearanceVersion;
long	OS_Version;
Boolean	on_MacOS_X;
static	WindowPtr	InfoWindow = NULL;
static	unsigned short	HelpMenuItemID;
static	void (* NextProc)(void *) = NULL;
static	void	*NextProcParam = NULL;
extern	DialogPtr	playerDlg, geditDlg;

void hilite_button(DialogPtr dlg, short item) {
	ControlHandle	button;
	unsigned long	finalTicks;
	if (on_MacOS_X) return;
	GetDialogItemAsControl(dlg, item, &button);
	HiliteControl(button, kControlButtonPart);
	Delay(8, &finalTicks);
	HiliteControl(button, kControlNoPart);
}
static pascal Boolean err_alert_filter(DialogPtr dlg, EventRecord *e, short *item) {
	switch (e->what) {
		case keyDown: if ((e->message & charCodeMask) == '\r') {
			*item = itemOK; hilite_button(dlg, itemOK);
			return true;
		} break;
		case updateEvt:
		if ((WindowPtr)e->message != GetDialogWindow(dlg)) DoUpdate(e);
		break;
		case nullEvent: check_playing(); check_integ_sel();
	}
	return false;
}
void res_msg(short index1, unsigned char *msg, short index2, OSErr err) {
	short	result;
	unsigned char	*mp, buf[80], message[256];
	static AlertStdAlertParamRec	ap = {
		false, false, NULL,
		(unsigned char *)(-1), NULL, NULL,
		kAlertStdAlertOKButton, 0,
		kWindowAlertPositionMainScreen
	};
	if (!ap.filterProc) {
		ap.filterProc = NewModalFilterUPP(&err_alert_filter);
	}
	mp = message + 1; message[0] = 0;
	if (index1) {
		GetIndString(buf, kBaseResID+3, index1);
		BlockMove(buf + 1, mp, buf[0]);
		message[0] += buf[0]; mp += buf[0];
	}
	if (msg) {
		BlockMove(msg + 1, mp, msg[0]);
		message[0] += msg[0]; mp += msg[0];
	}
	if (index2) {
		GetIndString(buf, kBaseResID+3, index2);
		BlockMove(buf + 1, mp, buf[0]);
		message[0] += buf[0]; mp += buf[0];
	}
	if (err) {
		unsigned char	n;
		GetIndString(buf, kBaseResID+2, 15);
		NumToString(err, buf + (n = buf[0] + 1));
		buf[0] += buf[n] + 1;
		buf[n] = ' ';
		mp = buf;
	} else mp = NULL;
	set_my_cursor(CURSsystem);
	StandardAlert(kAlertCautionAlert, message, mp, &ap, &result);
}
void error_msg(unsigned char *msg, OSErr err) {
	res_msg(0, msg, 0, err);
}
#ifdef	DEBUG
void event_monitor(char *message, EventRecord *e) {
	static	FILE	*fd = NULL;
	char	*name, *msg, tempName[24];
	unsigned char	title[256];
	if (e->what == nullEvent) return;
	if (!fd) fd = fopen("event.log", "w");
	if (!fd) return;
	switch (e->what) {
		case mouseDown: name = "mouseDown"; break;
		case mouseUp: name = "mouseUp"; break;
		case keyDown: name = "keyDown"; break;
		case keyUp: name = "keyUp"; break;
		case autoKey: name = "autoKey"; break;
		case updateEvt: name = "updateEvt"; break;
		case diskEvt: name = "diskEvt"; break;
		case activateEvt: name = (e->modifiers & activeFlag)? "activate" : "deactivate";
		break;
		case osEvt: name = "osEvt"; break;
		case kHighLevelEvent: name = "highLevel"; break;
		default: name = tempName; sprintf(tempName, "unknowEvt(0x%x)", e->what);
	}
	switch (e->what) {
		case updateEvt: case activateEvt:
		GetWTitle((WindowPtr)e->message, title);
		msg = (char *)title + 1;
		title[title[0] + 1] = '\0'; break;
		default: msg = (char *)title; sprintf(msg, "0x%x", e->message);
	}
	fprintf(fd, "%s %d (%d,%d) %s %s\n", message,
		e->when, e->where.h, e->where.v, name, msg);
	fflush(fd);
}
#endif
static pascal Boolean my_sc_alert_filter(DialogPtr dlg, EventRecord *e, short *item) {
	switch (e->what) {
		case keyDown: switch (e->message & charCodeMask) {
			case '\r': *item = itemOK; break;
			case 0x1b: *item = itemCancel; break;
			case 'D': case 'd': case 'Q': case 'q':
			if ((e->modifiers & cmdKey) == 0) return false;
			*item = itemNoOp; break;
			case '.': case 'W': case 'w':
			if ((e->modifiers & cmdKey) == 0) return false;
			*item = itemCancel; break;
			default: return false;
		}
		hilite_button(dlg, *item);
		return true;
		case updateEvt:
		if ((WindowPtr)e->message != GetDialogWindow(dlg)) DoUpdate(e);
		break;
		case nullEvent: check_playing(); check_integ_sel();
	}
	return false;
}
enum {
	idxLabelSave = 16,
	idxLabelDont, idxSaveChanged1, idxSaveChanged2
};
short save_change_alert(WindowPtr win, short idx) {
	short	result;
	Boolean	collapsed;
	unsigned char	*p, buf[80], message[256];
	static unsigned char	msg[32];
	static AlertStdAlertParamRec	ap = {
		false, false, NULL,
		msg, (unsigned char *)-1, NULL,
		kAlertStdAlertOKButton, kAlertStdAlertCancelButton,
		kWindowAlertPositionParentWindow
	};
	if (!ap.filterProc) {
		ap.filterProc = NewModalFilterUPP(&my_sc_alert_filter);
		GetIndString(msg, kBaseResID+2, idxLabelSave);
		ap.otherText = p = msg + msg[0] + 1;; 
		GetIndString(p, kBaseResID+2, idxLabelDont);
	}
	if (IsWindowCollapsed(win)) {
		CollapseWindow(win, false); collapsed = true;
	} else collapsed = false;
	SelectWindow(win);
	set_my_cursor(CURSsystem);
	GetIndString(message, kBaseResID+2, idx);
	GetWTitle(win, buf);
	BlockMove(buf + 1, (p = message + message[0] + 1), buf[0]);
	message[0] += buf[0]; p += buf[0];
	GetIndString(buf, kBaseResID+2, idx + 1);
	BlockMove(buf + 1, p, buf[0]);
	message[0] += buf[0];
	StandardAlert(kAlertCautionAlert, message, NULL, &ap, &result);
	if (collapsed && result == kAlertStdAlertCancelButton)
		CollapseWindow(win, true);
	return result;
}
/*short save_change_alert(WindowPtr, short) {
	static NavDialogOptions	option = {
		kNavDialogOptionsVersion,
		kNavDefaultNavDlogOptions, {-1, -1},
		"\pSBEAT", NULL, NULL, NULL,
		NULL, NULL, NULL, NULL
		};
	NavAskDiscardChangesResult	reply;
	NavAskDiscardChanges(&option, &reply, NULL, NULL);
	return reply;
}*/
static void close_window(WindowPtr win) {
	switch (GetWRefCon(win)) {
		case WIDField: close_field(FindField(win)); break;
		case WIDIntegrator: close_integrator(FindIntegrator(win)); break;
		case WIDgedit: close_gedit(FindGedit(win)); break;
		case WIDPlayer: close_player(); break;
		case WIDPartOption: close_part_option(); break;
	}
}
static void closeInfoWindow(void *cl) {
	if (TickCount() > (long)cl) {
		if (InfoWindow) { DisposeWindow(InfoWindow); InfoWindow = NULL; }
		NextProc = NULL;
	}
}
static void show_info_window(short second) {
	NextProcParam = (void *)(TickCount() + second * 60);
	NextProc = closeInfoWindow;
	if (InfoWindow) { SelectWindow(InfoWindow); return; }
	InfoWindow = GetNewCWindow(130, nil, kMoveToFront);
	SetWRefCon(InfoWindow, WIDOpenning);
	ShowWindow(InfoWindow);
}
static void OnSbeatCB(void) { show_info_window(60); }
static void PreferenceCB(void) { setup_preferences(); }
static void NopCB(void) {}
static void CloseCB(void) {
	WindowPtr	win = TopWindow;
	if (!win) win = FrontWindow();
	if (win) close_window(win);
}
static void ResetCB(void) {
	switch (GetWRefCon(TopWindow)) {
		case WIDField: reset_pop(FindField(TopWindow)); break;
		case WIDIntegrator: reset_integrator(FindIntegrator(TopWindow));
	}
}
static void QuitCB(void) {
	SBField	*sf, *nsf;
	SBIntegrator	*si, *nsi;
	SBgedit	*sg, *nsg;
	extern	SBField	*Fields;
	extern	SBIntegrator	*Integrators;
	extern	SBgedit	*Gedits;
	if (InfoWindow)
		{ DisposeWindow(InfoWindow); InfoWindow = NULL; }
	for (sg = Gedits; sg; sg = nsg) {
		nsg = sg->post;
		close_gedit(sg);
		if (sg->win) return;
	}
	for (si = Integrators; si; si = nsi) {
		nsi = si->post;
		close_integrator(si);
		if (si->win) return;
	}
	for (sf = Fields; sf; sf = nsf) {
		nsf = sf->post;
		close_field(sf);
		if (sf->win) return;
	}
	ExitToShell();
}
static void NextCB(void) {
	SBField *sf = FindField(TopWindow);
	if (sf) next_generation(sf, NULL);
}
static void NextInNewCB(void) {
	SBField *sf = FindField(TopWindow);
	if (sf) next_in_new(sf);
}
Boolean is_checked(short item) {
	short	mark;
	GetItemMark(GetMenuHandle(mWindow), item, &mark);
	return (mark != 0);
}
static void PartOptionCB(void) {
	if (is_checked(iPartOption)) close_part_option();
	else open_part_option();
}
static void PlayerCB(void) {
	if (is_checked(iPlayer)) close_player();
	else open_player();
}
static void charHex(char *p, char c) {
	static char	hex[] = "0123456789ABCDEF";
	p[0] = '%';
	p[1] = hex[(c >> 4) & 0x0f];
	p[2] = hex[c & 0x0f];
}
static void launch_url(char *url, long len) {
	OSErr	err;
	ICInstance inst;
	long	startSel = 0;
	if ((err = ICStart(&inst, CreatorCode)) != noErr)
		{ error_msg("\pICStart", err); return; }
	if ((err = ICLaunchURL(inst, "\p", url, len, &startSel, &len)) != noErr)
		error_msg("\pICLaunchURL", err);
	(void)ICStop(inst);
}
static void switch_man_page(void) {
	OSStatus	status;
	FSSpec	fs;
	FSRef	ref;
	short	i, k, len;
	char	*pre, *p, *q, path[512];
	static	char	escaped[] = "!\"#$%&'\\;*?&+~\177";
/*
	if (OS_Version < 0x0900) {
		static char	manualUrl[] =
		"http://www.edu.t.soka.ac.jp/~unemi/SSS2002/sbeatManual/";
		launch_url(manualUrl, strlen(manualUrl));
		return;
	}
*/
	status = FSMakeFSSpec(0,0,"\pManual",&fs);
	if (status) { error_msg("\pFSMakeFSSpec",status); return; }
	status = FSpMakeFSRef(&fs, &ref);
	if (status) { error_msg("\pFSpMakeFSRef",status); return; }
	status = FSRefMakePath(&ref, (unsigned char *)path, 255);
	if (status) { error_msg("\pFSRefMakePath",status); return; }
	if ((len = strlen(path)) > 200)
		{ error_msg("\pToo long path name.", len); return; }
	pre = on_MacOS_X? "locahost" : "/";
	for (i = k = 0; i < len; i ++) {
		if (!on_MacOS_X && path[i] == '/') k ++;
		else if (path[i] <= ' ') k ++;
		else for (p = escaped; *p; p ++)
			if (path[i] == p[0]) { k ++; break; }
	}
	q = path + 7 + strlen(pre) + len + k * 2;
	if (on_MacOS_X) *q = '/';
	BlockMove("index.html", q + (on_MacOS_X? 1 : 0), 11);
	for (i = len - 1, q --; i >= 0; i --, q --) {
		if (!on_MacOS_X) {
			if (path[i] == ':') { *q = '/'; continue; }
			else if (path[i] == '/') { charHex(q-=2,'/'); continue; }
		}
		if (path[i] <= ' ') charHex(q-=2, path[i]);
		else {
			for (p = escaped; *p; p ++) if (path[i] == p[0]) break;
			if (*p) charHex(q-=2,*p);
			else *q = path[i];
		}
	}
	BlockMove("file://", path, 7);
	BlockMove(pre, path + 7, strlen(pre));
	status = AHGotoPage(NULL,
		CFStringCreateWithCString(NULL, path, kCFStringEncodingMacRoman), NULL);
	if (status != noErr) error_msg("\pAHGotoPage",status);
}
static void goto_website(void) {
	static	unsigned char	urlStr[64] = {0};
	if (!urlStr[0]) GetIndString(urlStr,256,4);
	launch_url((char *)&urlStr[1], urlStr[0]);
}
typedef	void	(*CBProc)(void);
#define kNBarButtons	6
static int	NMenuButtons[] = { 2, 12, 13, 6, 2, 2 };
static const CBProc
	appleCBs[] = { OnSbeatCB, PreferenceCB },
	fileCBs[] = { NewFieldCB, NewIntegratorCB, OpenCB, NopCB,
		SaveCB, SaveAsCB, SaveAsSMFCB, SaveOptionsCB, NopCB,
		ResetCB, CloseCB, QuitCB },
	editCBs[] = { UndoCB, NopCB, CutCB, CopyCB, PasteCB, PasteOptionCB,
		ClearCB, SelectAllCB, NopCB, open_gedit, NopCB, NopCB, PreferenceCB },
	breedCBs[] = { DoItAgainCB, NopCB, NextCB, NextInNewCB, NopCB, MutateCB },
	windowCBs[] = { PartOptionCB, PlayerCB },
	helpCBs[] = { switch_man_page, goto_website },
	*MenuCBs[] = { appleCBs, fileCBs, editCBs, breedCBs, windowCBs, helpCBs };

static void MenuInit(void) {
	MenuHandle	mh;
	SetMenuBar(GetNewMBar(on_MacOS_X? kBaseResID+1 : kBaseResID));
	InsertMenu(GetMenu(mScoreEdit), -1);
	DrawMenuBar();
	if (on_MacOS_X) {
		EnableMenuItem(GetMenuHandle(mApple), NMenuButtons[0]);
		DeleteMenuItem(GetMenuHandle(mFile), NMenuButtons[1]);
		mh = GetMenuHandle(mEdit);
		DeleteMenuItem(mh, NMenuButtons[2]);
		DeleteMenuItem(mh, NMenuButtons[2] - 1);
	} else {
		unsigned char	itemStr[64];
		NMenuButtons[0] --;
		myHMGetHelpMenu(mh, HelpMenuItemID);
		GetIndString(itemStr,256,1);
		AppendMenu(mh, itemStr);
	}
}
static pascal OSErr DoOpenApp(const AppleEvent *, AppleEvent *, long) {
	extern	SBField		*Fields;
	static void activate_field(SBField *);
	NewFieldCB();
	if (!Fields) ExitToShell();
	activate_field(Fields);
	return noErr;
}
static pascal OSErr DoOpenDoc(const AppleEvent *e, AppleEvent *, long) {
	static	AEDescList	docList;
	OSErr	err = AEGetParamDesc(e, keyDirectObject, typeAEList, &docList);
	if (err != noErr) { SysBeep(10); return err; }
	err = open_from_AE_data(&docList);
	if (InfoWindow) reset_topwindow();
	return err;
}
static pascal OSErr DoPrintDoc(const AppleEvent *, AppleEvent *, long)
{ return noErr; }
static pascal OSErr DoQuitApp(const AppleEvent *, AppleEvent *, long)
{ QuitCB(); return noErr; }
static pascal OSErr DoShowPref(const AppleEvent *, AppleEvent *, long)
{ setup_preferences(); return noErr; }

static void EventInit(void) {
	static struct {
		pascal OSErr (*proc)(const AppleEvent *, AppleEvent *, long);
		AEEventID eid;
	} *p, evh[] = {
		{DoOpenApp, kAEOpenApplication},
		{DoOpenDoc, kAEOpenDocuments},
		{DoPrintDoc, kAEPrintDocuments},
		{DoQuitApp, kAEQuitApplication},
		{DoShowPref, kAEShowPreferences},
		{NULL, 0}
	};
	static AEEventHandlerUPP	ehUPP;
	OSErr err;
	for (p = evh; p->proc; p ++) {
		ehUPP = NewAEEventHandlerUPP(p->proc);
		err = AEInstallEventHandler(kCoreEventClass, p->eid, ehUPP, 0L, false);
	}
}
static void HandleMenuChoice(long choice) {
	short	menu, item;
	menu = HiWord(choice);
	item = LoWord(choice);
	if (!on_MacOS_X && menu == kHMHelpMenuID)
		{ menu = mHelp; item -= HelpMenuItemID - 1; }
	switch (menu) {
		case mScoreEdit: score_edit(item); break;
		default: if (mApple <= menu && menu < mApple+kNBarButtons) {
			if (item > 0 && item <= NMenuButtons[menu-mApple])
				(*MenuCBs[menu-mApple][item-1])();
			else if (menu == mWindow)
				select_win_menu_item(item - iWindowList);
		}
	}
	HiliteMenu(0);
}
static void HandleMouseDown(EventRecord *e) {
	WindowPtr		whichWindow;
	if (InfoWindow) { DisposeWindow(InfoWindow); InfoWindow = NULL; }
	else switch (FindWindow(e->where, &whichWindow)) {
		case inMenuBar: HandleMenuChoice(MenuSelect(e->where)); break;
		case inDrag: DragWindow(whichWindow, e->where, NULL);
		break;
		case inGoAway:
		if (TrackGoAway(whichWindow, e->where)) close_window(whichWindow);
		break;
		case inGrow: grow_integrator(FindIntegrator(whichWindow), e); break;
		case inZoomOut:
		if (TrackBox(whichWindow, e->where, inZoomOut))
			zoom_integ(FindIntegrator(whichWindow));
		break;
		case inContent: switch (GetWRefCon(whichWindow)) {
			case WIDPlayer: click_player(e); break;
			case WIDPartOption: click_part_option(e); break;
			case WIDgeditDlg: click_gedit_dlg(e); break;
			case WIDField: click_field(FindField(whichWindow), e); break;
			case WIDIntegrator: click_integrator(FindIntegrator(whichWindow), e); break;
			case WIDgedit: click_gedit(FindGedit(whichWindow), e);
		}
	}
}
static void HandleKeyDown(EventRecord *e) {
	long	choice = MenuEvent(e);
	if (InfoWindow) { DisposeWindow(InfoWindow); InfoWindow = NULL; }
	else if (HiWord(choice)) HandleMenuChoice(choice);
	else if ((e->modifiers & shiftKey) && (e->message & charCodeMask) == '/')
		switch_man_page();
	else {
		if (keyin_player(e)) return;
		if (!TopWindow) return;
		switch (GetWRefCon(TopWindow)) {
			case WIDField: keyin_field(FindField(TopWindow), e); break;
			case WIDIntegrator: keyin_integrator(FindIntegrator(TopWindow), e);
		}
	}
}
static void draw_infowindow(void) {
	GrafPtr	old;
	PicHandle	p;
	Rect	bound;
	if (InfoWindow) {
		GetPort(&old);
		SetPort(myGetWPort(InfoWindow));
		p = GetPicture(128);
		DrawPicture(p, GetWindowPortBounds(InfoWindow, &bound));
		ReleaseResource((Handle)p);
		SetPort(old);
	}
}	
void DoUpdate(EventRecord *e) {
	WindowPtr	win = (WindowPtr)e->message;
	RgnHandle	visRgn = NewRgn();
	BeginUpdate(win);
	switch (GetWRefCon(win)) {
		case WIDField: update_field(FindField(win)); break;
		case WIDIntegrator: update_integrator(FindIntegrator(win)); break;
		case WIDgedit: update_gedit(win); break;
		case WIDOpenning: draw_infowindow(); break;
		case WIDPlayer:
		UpdateControls(win,
			GetPortVisibleRegion(myGetWPort(win), visRgn)); break;
		case WIDPartOption:
		UpdateControls(win,
			GetPortVisibleRegion(myGetWPort(win), visRgn)); break;
		case WIDgeditDlg:
		UpdateDialog(geditDlg,
			GetPortVisibleRegion(myGetWPort(win), visRgn)); break;
	}
	EndUpdate(win);
	DisposeRgn(visRgn);
}
Boolean front_is_dialog(void) {
	long	refcon = GetWRefCon(FrontWindow());
	return (refcon != WIDField && refcon != WIDIntegrator && refcon != WIDgedit);
}
static void set_menu_item_ability(MenuHandle mh, short item, Boolean enable) {
	if (enable) EnableMenuItem(mh, item);
	else DisableMenuItem(mh, item);
}
void setup_edit_menu(Boolean copy, Boolean paste, short winType) {
	MenuHandle	mh = GetMenuHandle(mEdit), mh2 = GetMenuHandle(mScoreEdit);
	Boolean	integp = (winType == WIDIntegSel2), cc = (copy && integp);
	Boolean	enable;
	short	i;
	set_menu_item_ability(mh, iCopy, copy);
	set_menu_item_ability(mh, iPaste, paste);
	set_menu_item_ability(mh, iPasteOpt, (!integp || paste));
	set_menu_item_ability(mh, iCut, cc);
	set_menu_item_ability(mh, iClear, cc);
	set_menu_item_ability(mh, iSelectAll,
		(winType == WIDField || winType == WIDIntegrator || integp));
	if (!integp) set_menu_item_ability(mh, iGeneEdit, copy &&
		(winType == WIDField || winType == WIDIntegrator));
	if (integp) {
		set_menu_item_ability(mh2, iNoteUp, copy);
		set_menu_item_ability(mh2, iNoteDown, copy);
	} else if (winType == WIDIntegrator) {
		SBIntegrator	*si;
		set_menu_item_ability(mh2, iShiftLeft, copy);
		set_menu_item_ability(mh2, iShiftRight, copy);
		set_menu_item_ability(mh2, iExpand, true);
		si = FindIntegrator(TopWindow);
		if (si) set_menu_item_ability(mh2, iShrink, (si->nSectionsH > 1));
	}
	enable = false;
	if (winType == WIDIntegrator || integp)
		for (i = 1; i <= nMItemsScoreEdit; i ++)
			if (IsMenuItemEnabled(mh2, i)) { enable = true; break; }
	set_menu_item_ability(mh, iScoreEdit, enable);
	if (!enable) for (i = 1; i <= nMItemsEdit; i ++)
		if (IsMenuItemEnabled(mh, i)) { enable = true; break; }
	set_menu_item_ability(mh, 0, enable);
}
void setup_field_menu(SBField *sf) {
	short	i, nm, np, ns, no;
	MenuHandle	mh;
	if (!sf) return;
	mh = GetMenuHandle(mFile);
	if (sf->fmode == ModifiedFile) EnableMenuItem(mh, iSaveFile);
	else DisableMenuItem(mh, iSaveFile);
	EnableMenuItem(mh, iSaveAs);
	EnableMenuItem(mh, iSaveOption);
	EnableMenuItem(mh, iReset);
	nm = np = ns = no = 0;
	for (i = 0; i < PopulationSize; i ++)
	switch (sf->sel[i] & (SelectedFlag | ProtectedFlag)) {
		case SelectedFlag: ns ++; nm ++; break;
		case ProtectedFlag: break;
		case SelectedFlag | ProtectedFlag: ns ++; break;
		default: np ++;
	}
	if (sf->prevOp != GOpNothing)
		for (i = 0; i < PopulationSize; i ++)
			if (sf->sel[i] & HadSelectedFlag) no ++;
	setup_edit_menu((ns > 0), (nm > 0), WIDField);
	mh = GetMenuHandle(mBreed);
	if (nm > 0 || (ns > 0 && np > 0) || no > 0) EnableMenuItem(mh, 0);
	else DisableMenuItem(mh, 0);
	if (no > 0) EnableMenuItem(mh, iDoItAgain);
	else DisableMenuItem(mh, iDoItAgain);
	if (nm > 0) EnableMenuItem(mh, iMutate);
	else DisableMenuItem(mh, iMutate);
	if (ns > 0 && np > 0) {
		EnableMenuItem(mh, iNextThis); EnableMenuItem(mh, iNextNew);
		ActivateControl(sf->next_pane);
	} else {
		DisableMenuItem(mh, iNextThis); DisableMenuItem(mh, iNextNew);
		DeactivateControl(sf->next_pane);
	}
	DrawMenuBar();
}
static void setup_integrator_menu(SBIntegrator *si) {
	MenuHandle	mh;
	Boolean	psel, sel2p, selon;
	psel = (si->sel >= 0);
	mh = GetMenuHandle(mFile);
	if (si->fmode == ModifiedFile) EnableMenuItem(mh, iSaveFile);
	else DisableMenuItem(mh, iSaveFile);
	EnableMenuItem(mh, iSaveAs);
	if (psel) EnableMenuItem(mh, iSaveOption);
	else DisableMenuItem(mh, iSaveOption);
	EnableMenuItem(mh, iReset);
	setup_edit_menu(psel && ItgOnP(*si->scoreHandle,si->sel),
		psel, WIDIntegrator);
	selon = check_integ_selected_on(si, &sel2p);
	setup_edit_menu(selon, sel2p, WIDIntegSel2);
	DisableMenuItem(GetMenuHandle(mBreed), 0);
	DrawMenuBar();
}
static void activate_field(SBField *sf) {
	if (sf) {
		if (is_checked(iPartOption)) open_part_option();
		if (is_checked(iPlayer)) open_player();
		setup_field_menu(sf);
		set_activate_count(&sf->act);
		activate_field_controls(sf, true);
		activate_poption_items(true);
		activate_protection(true);
	}
}
static void activate_integrator(SBIntegrator *si) {
	if (si) {
		if (is_checked(iPartOption)) open_part_option();
		if (is_checked(iPlayer)) open_player();
		setup_integrator_menu(si);
		set_activate_count(&si->act);
		activate_integ_controls(si, true);
		activate_poption_items(check_integ_selected_on(si, NULL));
		activate_protection(false);
	}
}
void reset_topwindow(void) {
	unsigned long	act = 0;
	SBField	*sf, *tsf = NULL; SBIntegrator *si, *tsi = NULL;
	SBgedit *sg, *tsg = NULL;
	extern SBField *Fields; extern SBIntegrator *Integrators;
	extern SBgedit *Gedits;
	TopWindow = NULL;
	for (sf = Fields; sf; sf = sf->post)
		if (act < sf->act) { act = sf->act; tsf = sf; TopWindow = sf->win; }
	for (si = Integrators; si; si = si->post)
		if (act < si->act) { act = si->act; tsi = si; TopWindow = si->win; }
	for (sg = Gedits; sg; sg = sg->post)
		if (act < sg->act) { act = sg->act; tsg = sg; TopWindow = sg->win; }
	if (TopWindow) {
		if (tsg) activate_gedit(tsg->win, true);
		else if (tsi) activate_integrator(tsi);
		else activate_field(tsf);
		HiliteWindow(TopWindow, true);
		check_win_menu_item(TopWindow);
		play_selected_score(TopWindow);
	}
}
void set_activate_count(unsigned long *cnt) {
	static	unsigned long	activateCount = 1;
	if (*cnt < activateCount) *cnt = (++ activateCount);
}
static	short	currentIconID = -1;
void reset_my_cursor(void) { currentIconID = -1; }
void set_my_cursor(short k) {
	static	CursHandle	Cursor[NCursors] =
		{ NULL, NULL, NULL, NULL, NULL, NULL, NULL };
	if (currentIconID != k) {
		if (k < 0) {
			InitCursor(); currentIconID = -1;
		} else {
			if (!Cursor[k]) Cursor[k] = GetCursor(k + 128);
			if (Cursor[k]) { SetCursor(*Cursor[k]); currentIconID = k; }
}}}
static void set_ind_cursor(void) {
	short	modfier = 0, newIconID;
	KeyMap	keys;
	GetKeys(keys);
/*	if (keys[0] || keys[1] || keys[2] || keys[3])
		SysBeep(10);*/
	if (keys[1] & 1) modfier |= shiftKey;
	if (keys[1] & 4) modfier |= optionKey;
	if (keys[1] & 8) modfier |= controlKey;
	if (keys[1] & 0x8000) modfier |= cmdKey;
	switch (modfier) {
		case optionKey: newIconID = CURSpick; break;
		case shiftKey: if (PlayTypeFieldP(TopWindow))
			{ newIconID = CURSselect; break; }
		default: newIconID = CURSgrabber;
	}
	set_my_cursor(newIconID);
}
static void check_mouse_move(EventRecord *e) {
	WindowPtr	window;
	ControlHandle	control;
	Point	p;
	GrafPtr	old;
	short	pc;
	Boolean	indp;
	if (FindWindow(e->where, &window) != inContent)
		{ set_my_cursor(CURSsystem); return; }
	GetPort(&old);
	SetPort(myGetWPort(window));
	p = e->where;
	GlobalToLocal(&p);
	control = FindControlUnderMouse(p, window, &pc);
	if (control) {
		set_my_cursor((kControlNoPart < pc && pc < kControlDisabledPart
		 && pc != kControlIconPart)?
			((pc == kControlEditTextPart)? CURSeditText : CURSfinger) : CURSsystem);
	} else {
		switch (GetWRefCon(window)) {
			case WIDField: indp = (p.h > WinLeftSpace && p.v > WinTopSpace); break;
			case WIDgedit: indp = (p.h > WinLeftSpace); break;
			case WIDIntegrator:
			if (p.h > WinLeftSpace && p.h < IntWindowWidth && p.v > intWinTopSpace)
			switch (get_cursor_mode(FindIntegrator(window), e->modifiers)) {
				case CntlPalm: set_ind_cursor(); break;
				case CntlSelArrow: set_my_cursor(CURSselArrw); break;
				default: set_my_cursor(CURSsystem);
			} else set_my_cursor(CURSsystem);
			return;
		}
		if (indp) set_ind_cursor();
		else set_my_cursor(CURSsystem);
	}
	SetPort(old);
}
void activate_root_cntl(WindowPtr win) {
	OSErr	err;
	ControlHandle	root;
	err = GetRootControl(win, &root);
	if (err == noErr) ActivateControl(root);
}
static void DoActivate(EventRecord *e) {
	WindowPtr	win = (WindowPtr)e->message;
	long	winType = GetWRefCon(win);
	if (e->modifiers & activeFlag) {
		if (winType >= WIDField && winType <= WIDgedit) {
			TopWindow = win;
			switch (winType) {
				case WIDField: activate_field(FindField(win));
				break;
				case WIDIntegrator: activate_integrator(FindIntegrator(win));
				break;
				case WIDgedit: activate_gedit(win, true);
			}
			check_win_menu_item(win);
			play_selected_score(win);
		} /*else switch (winType) {
			case WIDPlayer: //case WIDPartOption:
			activate_root_cntl(win);
		}*/
	} else switch (winType) {
		case WIDField: activate_field_controls(FindField(win), false);
		break;
		case WIDIntegrator: activate_integ_controls(FindIntegrator(win), false);
		break;
		case WIDgedit: activate_gedit(win, false);
	}
}
static void EventLoop(void) {
	EventRecord	event;
	show_info_window(7);
	draw_infowindow();
	for(;;) {
		WaitNextEvent(everyEvent,&event,4,NULL);
#ifdef	DEBUG
		event_monitor("EventLoop",&event);
#endif
		switch (event.what) {
			case kHighLevelEvent: AEProcessAppleEvent(&event); break;
			case mouseDown: HandleMouseDown(&event); break;
			case keyDown: case autoKey: HandleKeyDown(&event); break;
			case updateEvt: DoUpdate(&event); break;
			case activateEvt: DoActivate(&event); break;
			case nullEvent:
			check_playing();
			check_mouse_move(&event);
			check_integ_sel();
			if (NextProc) (*NextProc)(NextProcParam);
		}
	}
}
static void CheckOS(void) {
	unsigned char	*msg;
	long	r;
	if (Gestalt(gestaltAppearanceAttr, &r) != noErr || (r & 3) != 1) {
		msg = "\pYour machine doesn't have Appearance 1.01 or upper.";
		goto error;
	} else if (Gestalt(gestaltAppearanceVersion, &AppearanceVersion) != noErr)
		 AppearanceVersion = 0;
	on_MacOS_X = (Gestalt(gestaltSystemVersion, &OS_Version) == noErr
		&& OS_Version >= 0x01000);
	return;
error:
	error_msg(msg, 0);
	ExitToShell();
}

void main(void)
{
	EnvInit();
	{	unsigned long	seed;
		GetDateTime(&seed);
		SetQDGlobalsRandomSeed((long)seed);
	}
	CheckOS();
	MenuInit();
	EventInit();
	DropFileInit();
	EventLoop();
}
