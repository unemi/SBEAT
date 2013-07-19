#include  <Navigation.h>
#include	<StandardFile.h>
#include	<stdio.h>
#include	<string.h>
#include	"decl.h"
#define	MallocUnit	16
SBField			*Fields = NULL, *FreeFields = NULL;
SBIntegrator	*Integrators = NULL, *FreeInteg = NULL;

SBField *new_field_rec(void) {
	SBField	*sf;
	if (!FreeFields) {
		short	i;
		FreeFields = (SBField *)NewPtr(sizeof(SBField) * MallocUnit);
		if (!FreeFields) {
			error_msg("\pNo memory for new Filed", MemError());
			return NULL;
		}
		for (i = 1; i < MallocUnit; i ++)
			FreeFields[i - 1].post = FreeFields + i;
		FreeFields[MallocUnit - 1].post = NULL;
	}
	sf = FreeFields;
	FreeFields = FreeFields->post;
	sf->post = Fields;
	if (Fields) Fields->pre = sf;
	else if (!Integrators) check_first_window();
	Fields = sf;
	sf->pre = NULL;
	sf->act = 0; set_activate_count(&sf->act);
	return sf;
}
SBIntegrator *new_integrator_rec(void) {
	SBIntegrator	*si;
	if (!FreeInteg) {
		short	i;
		FreeInteg = (SBIntegrator *)NewPtr(sizeof(SBIntegrator) * MallocUnit);
		if (!FreeInteg) {
			error_msg("\pNewPtr in new_integrator", MemError());
			return NULL;
		}
		for (i = 1; i < MallocUnit; i ++)
			FreeInteg[i - 1].post = FreeInteg + i;
		FreeInteg[MallocUnit - 1].post = NULL;
	}
	si = FreeInteg;
	FreeInteg = FreeInteg->post;
	si->post = Integrators;
	if (Integrators) Integrators->pre = si;
	else if (!Fields) check_first_window();
	Integrators = si;
	si->pre = NULL;
	si->act = 0; set_activate_count(&si->act);
	si->animeTime = 0;
	si->winPosition.h = si->winPosition.v = si->origWinHeight = 0;
	return si;
}
void free_field(SBField *sf) {
	if (sf->pre) sf->pre->post = sf->post;
	else if (sf == Fields) Fields = sf->post;
	if (sf->post) sf->post->pre = sf->pre;
}
void free_integrator(SBIntegrator *si) {
	if (si->scoreHandle) DisposeHandle((Handle)si->scoreHandle);
	if (si->pre) si->pre->post = si->post;
	else if (si == Integrators) Integrators = si->post;
	if (si->post) si->post->pre = si->pre;
}
void for_all_fields(void (*proc)(SBField *)) {
	SBField	*sf;
	for (sf = Fields; sf; sf = sf->post) (*proc)(sf);
}
SBField *FindField(WindowPtr win) {
	SBField	*sf;
	for (sf = Fields; sf; sf = sf->post)
		if (sf->win == win) return sf;
	return NULL;
}
SBIntegrator *FindIntegrator(WindowPtr win) {
	SBIntegrator	*si;
	for (si = Integrators; si; si = si->post)
		if (si->win == win) return si;
	return NULL;
}
/********* File related routines **************/
#define	FileBufferSize	512
static short	FileRefNum = -1;
static short	FileBufIndex = 0;
static long		FileBufInSize = 0;
static char		FileBuffer[FileBufferSize];
static NavTypeListHandle setup_type_list(short n, OSType *fileTypes) {
	NavTypeListHandle	openTypeList =
		(NavTypeListHandle)NewHandle(sizeof(NavTypeList) + sizeof(OSType) * (n-1));
	short	i;
	if (openTypeList != nil) {
		HLock((Handle)openTypeList);
		(*openTypeList)->componentSignature = CreatorCode;
		(*openTypeList)->osTypeCount = n;
		for (i = 0; i < n; i ++)
			(*openTypeList)->osType[i] = fileTypes[i];
		HUnlock((Handle)openTypeList);
	}
	return openTypeList;
}
#define	kCustomWidth	340
#define	kCustomHeight	52
#define	kControlListID	258
static	Handle 	gDitlList;
static	short	CopyrightItemID;
#define	VolumeAdjustCBoxID	(CopyrightItemID + 1)
#define	TailPauseTextID		(CopyrightItemID + 3)
#define	TailPauseUpDnID		(CopyrightItemID + 5)
static void set_pause_string(DialogPtr dlg, short pa) {
	ControlRef	control;
	unsigned char	pauseStr[16];
	GetDialogItemAsControl(dlg, TailPauseTextID, &control);
	sprintf((char *)pauseStr + 1, "%d.%d", pa/10, pa%10);
	pauseStr[0] = strlen((char *)pauseStr + 1);
	SetDialogItemText((Handle)control, pauseStr);
}
static void doClickSMFSave(NavCBRecPtr callBackParms, NavCallBackUserData cl) {
	ControlPartCode	part;
	Boolean	modified = false;
	ControlRef	control;
	Point	pt = callBackParms->eventData.eventDataParms.event->where;
	SetPortWindowPort(callBackParms->window);
	GlobalToLocal(&pt);
	control = FindControlUnderMouse(pt, callBackParms->window, &part);
	if (control && cl) switch (part) {
		case kControlUpButtonPart:
			((smfOptionalInfo *)cl)->pause ++; modified = true; break;
		case kControlDownButtonPart: if (((smfOptionalInfo *)cl)->pause > 0) {
			((smfOptionalInfo *)cl)->pause --; modified = true;
		} break;
	}
	if (modified)
		set_pause_string(GetDialogFromWindow(callBackParms->window),
			((smfOptionalInfo *)cl)->pause);
}
static void doKeyInSMFSave(NavCBRecPtr callBackParms, NavCallBackUserData cl) {
	OSErr	err;
	ControlRef	control, cntrl;
	DialogPtr	dlg = GetDialogFromWindow(callBackParms->window);
	unsigned char	pauseStr[16];
	int	pa, pb;
	GetDialogItemAsControl(dlg, TailPauseTextID, &control);
	err = GetKeyboardFocus(callBackParms->window, &cntrl);
	if (err != noErr || cntrl != control) return;
	GetDialogItemText((Handle)control, pauseStr);
	pauseStr[pauseStr[0] + 1] = '\0';
	sscanf((char *)pauseStr + 1, "%d.%d", &pa, &pb);
	((smfOptionalInfo *)cl)->pause = (pa = pa * 10 + pb);
	GetDialogItemAsControl(dlg, TailPauseUpDnID, &control);
	SetControlValue(control, pa);
}
static void smf_navi_customize(NavCBRecPtr callBackParms) {
	static short	LastTryWidth, LastTryHeight;
	short	neededWidth = callBackParms->customRect.left + kCustomWidth;
	short	neededHeight = callBackParms->customRect.top + kCustomHeight;
	if ((callBackParms->customRect.right == 0)
	 && (callBackParms->customRect.bottom == 0)) {
		callBackParms->customRect.right = neededWidth;
		callBackParms->customRect.bottom = neededHeight;
	} else {
		if (LastTryWidth != callBackParms->customRect.right)
			if (callBackParms->customRect.right < neededWidth)
				callBackParms->customRect.right = neededWidth;
		if (LastTryHeight != callBackParms->customRect.bottom)
			if (callBackParms->customRect.bottom < neededHeight)
				callBackParms->customRect.bottom = neededHeight;
	}
	LastTryWidth = callBackParms->customRect.right;
	LastTryHeight = callBackParms->customRect.bottom;	
}
static void get_real_name_of_user(unsigned char *name) {
	OSErr	err;
	ICInstance inst;
	ICAttr	attr;
	long	size = 50;
	if ((err = ICStart(&inst, CreatorCode)) != noErr)
		{ error_msg("\pICStart", err); return; }
	if ((err = ICBegin(inst, icReadOnlyPerm)) != noErr)
		{ error_msg("\pICBegin", err); goto stop; }
	err = ICGetPref(inst, kICRealName, &attr, name, &size);
	if (err || !name[0]) {
		size = 50;
		err = ICGetPref(inst, kICEmail, &attr, name, &size);
		if (err || !name[0]) BlockMove("\pMe", name, 3);
	}
	(void)ICEnd(inst);
stop:
	(void)ICStop(inst);
}
static void smf_navi_start(NavCBRecPtr callBackParms, NavCallBackUserData cl) {
	smfOptionalInfo	*smfInfo = (smfOptionalInfo *)cl;
	unsigned char	*copyRight = smfInfo->copyRight;
	short	theErr;
	CopyrightItemID = 0;
	gDitlList = GetResource('DITL', kControlListID);
	if ((gDitlList == NULL) || (ResError() != noErr)) return;
	theErr = NavCustomControl(callBackParms->context,
		kNavCtlAddControlList, gDitlList);
	if (theErr != noErr) return;
	theErr = NavCustomControl(callBackParms->context,
		kNavCtlGetFirstControlID, &CopyrightItemID);
	if (theErr == noErr) {
		ControlHandle	control;
		DialogRef	dlg = GetDialogFromWindow(callBackParms->window);
		CopyrightItemID += 2;
		GetDialogItemAsControl(dlg, CopyrightItemID, &control);
		if (!copyRight[0]) {
			Handle	dateFormat = GetResource('itl1', 128);
//			GetDialogItemText((Handle)control, copyRight);
			get_real_name_of_user(copyRight + 4);
			copyRight[0] = copyRight[4] + 4;
			BlockMove("(c) ", copyRight + 1, 4);
			if (dateFormat) {
				unsigned long	secs;
				short	n = copyRight[0] + 2;
				GetDateTime(&secs);
				DateString(secs, longDate, copyRight + n, dateFormat);
				copyRight[0] += copyRight[n] + 2;
				copyRight[n-1] = ','; copyRight[n] = ' ';
			}
		}
		SetDialogItemText((Handle)control, copyRight);
		GetDialogItemAsControl(dlg, VolumeAdjustCBoxID, &control);
		SetControlValue(control, smfInfo->adjust);
		set_pause_string(dlg, smfInfo->pause);
		GetDialogItemAsControl(dlg, TailPauseUpDnID, &control);
		SetControlValue(control, smfInfo->pause);
	} else CopyrightItemID = 0;
}
static void smf_navi_finish(DialogPtr dlg, NavCallBackUserData cl) {
	smfOptionalInfo	*smfInfo = (smfOptionalInfo *)cl;
	ControlHandle	control;
	GetDialogItemAsControl(dlg, CopyrightItemID, &control);
	GetDialogItemText((Handle)control, smfInfo->copyRight);
	GetDialogItemAsControl(dlg, VolumeAdjustCBoxID, &control);
	smfInfo->adjust = GetControlValue(control);
}
static pascal void myEventProc(const NavEventCallbackMessage callBackSelector, 
	NavCBRecPtr callBackParms, NavCallBackUserData cl) {
	EventRecord	*e = callBackParms->eventData.eventDataParms.event;
	switch (callBackSelector) {
		case kNavCBEvent:
		switch (e->what) {
			case updateEvt: DoUpdate(e); break;
			case mouseDown: doClickSMFSave(callBackParms, cl); break;
			case keyDown: doKeyInSMFSave(callBackParms, cl); break;
		} break;
		default: if (cl) switch (callBackSelector) {
			case kNavCBCustomize: smf_navi_customize(callBackParms); break;
			case kNavCBStart: smf_navi_start(callBackParms, cl); break;
			case kNavCBTerminate: if (gDitlList)
				{ ReleaseResource(gDitlList); gDitlList = NULL; }
			break;
			case kNavCBAccept:
			smf_navi_finish(GetDialogFromWindow(callBackParms->window), cl);
			break;
		}
	}
}
void open_from_fss(FSSpec *fss, OSType type) {
	switch (type) {
		case FileTypeField: open_field(fss); break;
		case FileTypeIntegrator: open_integrator(fss); break;
		case FileTypeOptions: load_options(fss); break;
		default: error_msg("\pUnknown file type", type);
	}
}
Boolean open_from_AE_data(AEDescList *docList) {
	FSSpec	myFSS;
	OSErr	err;
	long	index, itemsInList;
	Size actualSize; AEKeyword keywd; DescType returnedType;
	FInfo	finfo;
	err = AECountItems(docList, &itemsInList);
	if (err != noErr) { SysBeep(10); return false; }
	for (index = 1; index <= itemsInList; index ++) {
		err = AEGetNthPtr(docList, index, typeFSS,
			&keywd, &returnedType, &myFSS, sizeof(myFSS), &actualSize);
		if (err != noErr) continue;
		if (FSpGetFInfo(&myFSS, &finfo) != noErr) continue;
		open_from_fss(&myFSS, finfo.fdType);
	}
	err = AEDisposeDesc(docList);
	return true;
}
void OpenCB(void) {
	static SFTypeList	tlist = { FileTypeField, FileTypeIntegrator, FileTypeOptions };
	OSErr	theErr;
	NavReplyRecord	theReply;
	NavDialogOptions	dialogOptions;
	NavTypeListHandle	openTypeList;
	NavEventUPP			eventUPP;
	unsigned char	OpenTitle[32];
	GetIndString(OpenTitle, 128, 2);
	theErr = NavGetDefaultDialogOptions(&dialogOptions);
	dialogOptions.dialogOptionFlags |= kNavNoTypePopup;
	dialogOptions.dialogOptionFlags &= ~kNavAllowPreviews;
	BlockMove(OpenTitle, dialogOptions.windowTitle, OpenTitle[0]+1);
	openTypeList = setup_type_list(3, tlist);
	eventUPP = NewNavEventUPP(&myEventProc);
	theErr = NavGetFile( nil, &theReply, &dialogOptions,
		eventUPP, nil, nil, openTypeList, nil );
	DisposeNavEventUPP(eventUPP);
	if (!theErr) open_from_AE_data(&theReply.selection);
	theErr = NavDisposeReply( &theReply );
	if (openTypeList != NULL) DisposeHandle( (Handle)openTypeList );
}
extern	WindowPtr	TopWindow;
void SaveCB(void) {
	if (!TopWindow) return;
	switch (GetWRefCon(TopWindow)) {
		case WIDField: save_field(FindField(TopWindow)); break;
		case WIDIntegrator: save_integrator(FindIntegrator(TopWindow)); break;
		case WIDgedit: save_gedit_option(FindGedit(TopWindow));
	}
}
void SaveAsCB(void) {
	if (!TopWindow) return;
	switch (GetWRefCon(TopWindow)) {
		case WIDField: save_field_as(FindField(TopWindow)); break;
		case WIDIntegrator: save_integrator_as(FindIntegrator(TopWindow));
	}
}
static Boolean open_to_save(OSType creatorCode, OSType fileType,
	FSSpec *fss, short *ref, Boolean replacing) {
	OSErr	theErr;
	if (replacing) {
		theErr = FSpOpenDF(fss, fsRdWrPerm, ref);
		if (theErr) { error_msg(fss->name, theErr); return false; }
	} else {
		theErr = FSpCreate(fss, creatorCode, fileType, smSystemScript);
		if (theErr) { error_msg(fss->name, theErr); return false; }
		theErr = FSpOpenDF(fss, fsRdWrPerm, ref);
		if (theErr) { error_msg(fss->name, theErr); return false; }
	}
	return true;
}
static Boolean query_save_file(OSType creatorCode, OSType fileType,
	unsigned char *title, unsigned char *fname, FSSpec *fss, short *ref, void *cl) {
	OSErr				theErr;
	NavReplyRecord		theReply;
	NavDialogOptions	dialogOptions;
	NavEventUPP			eventUPP;
	Boolean	result;
	Size actualSize; AEKeyword keywd; DescType returnedType;
	theErr = NavGetDefaultDialogOptions(&dialogOptions);
	dialogOptions.dialogOptionFlags |= kNavNoTypePopup;
	if (fname) BlockMove(fname, dialogOptions.savedFileName, fname[0]+1);
	if (title) BlockMove(title, dialogOptions.windowTitle, title[0]+1);
	eventUPP = NewNavEventUPP(&myEventProc);
	theErr = NavPutFile(nil, &theReply, &dialogOptions,
		eventUPP, fileType, creatorCode, cl);
	DisposeNavEventUPP(eventUPP);
	if (theErr) { if (theErr != -128) error_msg(fname, theErr); return false; }
	theErr = AEGetNthPtr(&theReply.selection, 1, typeFSS,
		&keywd, &returnedType, fss, sizeof(FSSpec), &actualSize);
	if (theErr) { error_msg(fname, theErr); result = false; goto error; }
	result = open_to_save(creatorCode, fileType, fss, ref, theReply.replacing);
/*	NavCompleteSave(&theReply, kNavTranslateInPlace);*/
error:
	NavDisposeReply(&theReply);
	return result;
}
void SaveAsSMFCB(void) {
	static	unsigned char	title[32] = {0};
	static	smfOptionalInfo	smfInfo = { true, 20, {0} };
	short	ref;
	Str255	fname;
	FSSpec	fss;
	if (!TopWindow) return;
	if (!title[0]) GetIndString(title, 128, 14);
	GetWTitle(TopWindow, fname);
	BlockMove(".mid", fname+fname[0]+1, 4); fname[0] += 4;
	if (!query_save_file(CreatorCodeSMF, FileTypeSMF, title,
		fname, &fss, &ref, &smfInfo)) return;
	write_smf(TopWindow, ref, &smfInfo);
	FSClose(ref);
}
void SaveOptionsCB(void) {
	static	unsigned char	title[32] ={0};
	short	ref;
	FSSpec	fss;
	if (!title[0]) GetIndString(title, 128, 15);
	if (!query_save_file(CreatorCode, FileTypeOptions, title,
		"\pSbeat options", &fss, &ref, nil)) return;
	save_options(ref);
	FSClose(ref);
}
Boolean save_as_setup(WindowPtr win, FSSpec *fss, short *ref,
	OSType ftype, unsigned char title[]) {
	Str255	fname;
	GetWTitle(win, fname);
	if (!query_save_file(CreatorCode, ftype, title, fname, fss, ref, nil))
		return false;
	SetWTitle(win, fss->name);
	set_win_menu_item(win);
	return true;
}
