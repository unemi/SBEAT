#include  <stdio.h>
#include	<string.h>
#include	"decl.h"
#include	"vers.h"
#include	"dialog.h"

WindowPtr	partoptionWin = NULL;
ViewInfo	*CurrentVI = NULL;
BarInfo		*CurrentBI = NULL;
BarInfo		IntegBarInfo;
BarInfoFlags	BarInfoUniq;

extern	WindowRef	TopWindow, PlayWindow;
extern	DialogRef	playerDlg, geditDlg;
extern	Boolean		on_MacOS_X;

#ifdef	JAPANESE_VERSION
#define	CFSTR_JP(s)	CFStringCreateWithCString(NULL, s, kCFStringEncodingMacJapanese)
#define	PartOptionTitle	"\pパートオプション"
#define	PartOptionLReset	CFSTR_JP("リセット")
#define	poptionCntlResetWidth	54
#define	PartOptionLDisclosure	CFSTR_JP("表示/隠蔽")
#define	PartOptionDsAndPerc	CFSTR_JP("ドラム & パーカッション")
#define	PartOptionInstTitle	CFSTR_JP("音色")
#define	PartOptionRangTitle	CFSTR_JP("音域")
#define	PartOptionUnitTitle	CFSTR_JP("拍の単位")
#define	PartOptionIterTitle	CFSTR_JP("繰返し数")
#else
#define	PartOptionTitle	"\pPart options"
#define	PartOptionLReset	CFSTR("Reset")
#define	poptionCntlResetWidth	40
#define	PartOptionLDisclosure	CFSTR("Disclosure")
#define	PartOptionDsAndPerc	CFSTR("Drums & Percussion")
#define	PartOptionInstTitle	CFSTR("Instrument")
#define	PartOptionRangTitle	CFSTR("Pitch range")
#define	PartOptionUnitTitle	CFSTR("Unit beat")
#define	PartOptionIterTitle	CFSTR("Iteration")
#endif
#define	SliderMargin	18
#define	SliderHeight_X	20
#define	SliderHeight_9	16
#define	poptionButtonSize	20
#define	poptionVPad	2
#define	poptionHPad	1
#define	poptionDsclsrHeight	9
#define	poptionPartHeight	(poptionButtonSize+poptionVPad*2)
#define	poptionClosedPartH	poptionDsclsrHeight
#define	poptionTitleHeight	(poptionClosedPartH+poptionButtonSize)
#define	poptionHeight	(poptionTitleHeight+poptionPartHeight*(NTotalParts+1))
#define	poptionPlayH	poptionHPad
#define	poptionDsclsrWidth	9
#define	poptionIDTextWidth	22
#define	poptionInstrumentWidth	80
#define	poptionOctaveWidth	20
#define	poptionUnitBeatWidth	20
#define	poptionIterationWidth	20
#define poptionPrtctMPad	1
#define poptionPrtctMWidth	2
#define	poptionGPCVTWidth	16
#define	poptionGPCSldWidth	(NTotalParts+SliderMargin)
#define	poptionCntlVTWidth	24
#define	poptionCntlSldWidth	(127+SliderMargin)
#define	poptionX_Dsclsr	0
#define	poptionX_IDText	(poptionX_Dsclsr+poptionDsclsrWidth+1+poptionHPad)
#define	poptionX_View	(poptionX_IDText+poptionIDTextWidth+poptionHPad)
#define	poptionX_Play	(poptionX_View+poptionButtonSize+poptionHPad)
#define	poptionX_Inst	(poptionX_Play+poptionButtonSize+poptionHPad)
#define	poptionX_Octave	(poptionX_Inst+poptionInstrumentWidth+poptionHPad)
#define	poptionX_UnitBt	(poptionX_Octave+poptionOctaveWidth+poptionHPad)
#define	poptionX_Iterat	(poptionX_UnitBt+poptionUnitBeatWidth+poptionHPad)
#define	poptionX_Prtct	(poptionX_Iterat+poptionIterationWidth+poptionHPad)
#define	poptionX_PrtctM	(poptionX_Prtct+poptionButtonSize+poptionPrtctMPad)
#define	poptionX_GPCVT	(poptionX_PrtctM+poptionPrtctMWidth+poptionHPad)
#define	poptionX_GPCSld	(poptionX_GPCVT+poptionGPCVTWidth+poptionHPad)
#define	poptionX_CntlVT	(poptionX_GPCSld+poptionGPCSldWidth+poptionHPad)
#define	poptionX_CntlSld	(poptionX_CntlVT+poptionCntlVTWidth+poptionHPad)
#define	poptionWidth	(poptionX_CntlSld+poptionCntlSldWidth+3)
#define	NColumnDisclosures	4
#define	NtitleButtons	4
static	ControlRef
	poptionColumnDisclosuresRow,
	poptionTitleHeader,
	poptionTitleButtons[NtitleButtons],
	poptionBGPane[NTotalParts+1],
	poptionDsclsrButtons[NTotalParts+1],
	poptionInstruments[NTotalParts+1],
	poptionPlayButtons[NTotalParts],
	poptionViewButtons[NTotalParts],
	poptionUnitBeatMenus[NTotalParts],
	poptionIterationMenus[NTotalParts],
	poptionProtectButtons[NTotalParts],
	poptionGPCValueTexts[NTotalParts],
	poptionGPCSliders[NTotalParts],
	poptionOctaveMenus[NSoloParts+NChordParts],
	poptionCntlValueTexts[NNoteCh],
	poptionCntlSliders[NNoteCh];
#define	poptionGPCMenuButton	poptionTitleButtons[0]
#define	poptionCntlMenuButton	poptionTitleButtons[1]
#define	poptionResetButton		poptionTitleButtons[2]
#define	poptionDsclsrMenuButton	poptionTitleButtons[3]
enum {
	poptionGPCMnBt	= 1,
	poptionCntlMnBt,
	poptionCntlRstBt,
	poptionDsclsrMnBt
};
enum {	// menu item IDs in disclosure menu
	iShowAllParts = 1,
	iShowPlayParts,
	iShowOnlyPlayParts,
	iHideSilentParts,
	iHideAllParts
};
enum {	// index ID of Help tags
	htChromosomeMenu = 1, htControlMenu, htResetControl,
	htView, htPlayOn, htInstrument, htDrumKit, htPercussion,
	htOctave, htUnitBeat, htIteration, htProtectPart, htGPCSld, htCntlSld
};
#define	poptionTitle	0x000
#define	poptionBG		0x100
#define	poptionPlay		0x200
#define	poptionView		0x300
#define	poptionProtect	0x400
#define	poptionUnitBeat	0x500
#define	poptionIterate	0x600
#define	poptionGPCross	0x700
#define	poptionInst		0x800
#define	poptionOctave	0x900
#define	poptionCntlSld	0xa00
#define	poptionDsclsr	0xb00
#define	poptionRDsclsr	0xc00
#define	poptionKindMask	0xf00
#define	poptionPartMask	0x0ff
#define	getColumnID(v)	((((v)>>12)&0x0f)-1)	
#define	columnID(id)	(((id)+1)<<12) 
extern	RGBColor	partColor[];
static	char	*partIDNames[] = {
	"S1", "S2", "S3", "S4", "S5", "S6", "S7", "S8", "S9", "S10", "S11", "S12", "S13",
	"C1", "C2", "P1", "P2", "P3", "P4", "P5", "P6", "P7", "P8"
};
static	CIconHandle	smallIndicator = NULL;
static	Boolean	partClosed[NTotalParts+1];
static	Boolean	columnClosed[] = { 0, 0, 0, 0 };
static	short	columnX[] =
	{ poptionX_Inst, poptionX_UnitBt, poptionX_Prtct, poptionX_CntlVT, poptionWidth };
static	char	octvDefault[]
	= { 5, 5, 5, 4, 4, 4, 3, 3, 3, 2, 2, 1, 1, 3, 3 };
Boolean	playDefault[]
	= { 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0 };
void show_hide_window(WindowRef win, Boolean show) {
	if (show != IsWindowVisible(win)) TransitionWindow(win,
		kWindowZoomTransitionEffect,
		(show? kWindowShowTransitionAction : kWindowHideTransitionAction),
		NULL);
}
static void show_dialog_win(WindowPtr win, Boolean show, short item) {
	if (!win) return;
	show_hide_window(win, show);
	CheckMenuItem(GetMenuHandle(mWindow), item, show);
}
static void show_dialog(DialogPtr dlg, Boolean show, short item) {
	if (dlg) show_dialog_win(GetDialogWindow(dlg), show, item);
}
static void set_text(ControlRef txt, unsigned char *pstr) {
/*	CFStringRef	cfstr =
		CFStringCreateWithPascalString(NULL, pstr, kCFStringEncodingMacRoman);
/*	SetControlData(txt, kControlEntireControl,
		kControlStaticTextCFStringTag, sizeof(cfstr), &cfstr);*/
	SetControlData(txt, kControlEntireControl,
		kControlStaticTextTextTag, pstr[0], pstr + 1);
	DrawOneControl(txt);
}	
static void set_digits(ControlRef txt, long value) {
	unsigned char	pstr[16];
	NumToString(value, pstr);
	set_text(txt, pstr);
}
void check_no_window(void) {
	extern	SBField	*Fields;
	extern	SBIntegrator	*Integrators;
	extern	SBgedit	*Gedits;
	MenuRef	mh;
	if (Fields || Integrators || Gedits) return;
	mh = GetMenuHandle(mFile);
	DisableMenuItem(mh, iSaveFile);
	DisableMenuItem(mh, iSaveAs);
	DisableMenuItem(mh, iSaveAsSMF);
	DisableMenuItem(mh, iSaveOption);
	show_dialog_win(partoptionWin, false, iPartOption);
	show_dialog(playerDlg, false, iPlayer);
}
void check_first_window(void) {
	MenuHandle	mh = GetMenuHandle(mFile);
	EnableMenuItem(mh, iSaveAs);
	EnableMenuItem(mh, iSaveAsSMF);
	show_dialog_win(partoptionWin, true, iPartOption);
	show_dialog(playerDlg, true, iPlayer);
}
void set_default_poption(SBField *sf) {
	extern	short	defaultInstrument[], defaultDrumsInst[];
	short	i, k;
	if (CurrentBI) {
		sf->b = *CurrentBI;
		sf->v = *CurrentVI;
		return;
	}
	for (i = k = 0; i < NTotalParts; i ++) {
		sf->b.protect[i] = sf->b.unitBeat[i] = sf->b.iteration[i] = 0;
		sf->b.genePart[i][0] = sf->b.genePart[i][2] = i;	// Melody & Velocity
		if (i < ChordPart - 2) sf->b.genePart[i][1] = 0;	// Rhythm
		else sf->b.genePart[i][1] = i;
		sf->b.playOn[i] = playDefault[i];
		if (k < NDispParts && playDefault[i]) {
			sf->v.viewOn[i] = true;
			sf->v.viewPartID[k] = sf->v.viewQue[k] = i;
			k ++;
		} else sf->v.viewOn[i] = false;
	}
	for (i = 0; i < NTotalParts; i ++) {
		if (k >= NDispParts) break;
		if (!playDefault[i]) {
			sf->v.viewOn[i] = true;
			sf->v.viewPartID[k] = sf->v.viewQue[k] = i;
			k ++;
		}
	}
	for (i = 0; i < NNoteCh; i ++)
		sf->b.instrument[i] = defaultInstrument[i];
	for (i = 0; i < NDsPercParts; i ++)
		sf->b.drumsInst[i] = defaultDrumsInst[i];
	for (i = 0; i < NNoteCh - 1; i ++)
		sf->b.octave[i] = octvDefault[i];
	init_part_control(&sf->b);
	sf->b.tempo = TempoInitVal;
	sf->b.keyScale = KeyInitVal;
	sf->b.keyNote = KeyNoteVariable;
}
static void set_pv_button_icon(ControlRef button, short iconID) {
	ControlButtonContentInfo	cinfo;
	cinfo.contentType = kControlContentCIconRes;
	cinfo.u.resID = iconID;
	SetControlData(button, kControlEntireControl,
		kControlBevelButtonContentTag, sizeof(cinfo), &cinfo);
	DrawOneControl(button);
}
static void setup_view_items(ViewInfo *vi) {
	short	i;
	for (i = 0; i < NTotalParts; i ++)
		set_pv_button_icon(poptionViewButtons[i],
			(i == vi->viewQue[NDispParts - 1])? IcnEyeHalf : IcnEyeClosed - vi->viewOn[i]);
}
static short set_menu_button_title(ControlRef button) {
	MenuRef	menu;
	short	item;
	OSErr	err;
	unsigned char	itemText[32];
	if ((err = GetBevelButtonMenuValue(button, &item))) return -1;
	if ((err = GetBevelButtonMenuHandle(button, &menu))) return -1;
	GetMenuItemText(menu, item, itemText);
	SetControlTitle(button, itemText);
	return item;
}
static void redraw_partialBgPane(short id, short x, short w) {
	Rect	bound;
	short	i, j;
	RgnHandle	oldClip = NewRgn(), newClip = NewRgn();
	GetClip(oldClip);
	GetControlBounds(poptionBGPane[id], &bound);
	if (x < 0) { bound.left = 0; i = 0; }
	else for (i = 0, bound.left = columnX[0]; i < x; i ++)
		bound.left += (columnClosed[i])?
			poptionClosedPartH : columnX[i + 1] - columnX[i];
	for (j = 0, bound.right = bound.left; j < w; j ++, i ++)
		bound.right += (columnClosed[i])?
			poptionClosedPartH : columnX[i + 1] - columnX[i];
	InsetRect(&bound, 0, 1);
	RectRgn(newClip, &bound);
	SetClip(newClip);
	DrawOneControl(poptionBGPane[id]);
	SetClip(oldClip);
	DisposeRgn(newClip);
	DisposeRgn(oldClip);
}
static void draw_protection_indicator0(Rect *bound, short chromosome) {
	Rect	rect;
	rect.left = bound->right + poptionPrtctMPad;
	rect.right = rect.left + poptionPrtctMWidth;
	rect.top = bound->top + chromosome * poptionButtonSize / 3;
	rect.bottom = bound->top + (chromosome + 1) * poptionButtonSize / 3 - 2;
	PaintRect(&rect);
}	
static void draw_protection_indicator(short part, short chromosome) {
	Rect	bound;
	if (partClosed[(part < DsPercPart)? part : part + 1]) return;
	GetControlBounds(poptionProtectButtons[part], &bound);
	draw_protection_indicator0(&bound, chromosome);
}
static void draw_protection_indicators(short part) {
	Rect	bound;
	short	chromosome, mask;
	if (columnClosed[2]) return;
	GetControlBounds(poptionProtectButtons[part], &bound);
	for (chromosome = 0, mask = 1; chromosome < 3; chromosome ++, mask <<= 1) {
		ForeColor((CurrentBI->protect[CurrentBI->genePart[part][chromosome]] & mask)?
			blackColor : whiteColor);
		draw_protection_indicator0(&bound, chromosome);
	}
}
static void setup_gp_items(short chromosome) {
	short	i, j, gidx;
	unsigned char	mask;
	Boolean	fieldp = (GetWRefCon(TopWindow) == WIDField);
	if (chromosome < 0)
		if (GetBevelButtonMenuValue(poptionGPCMenuButton, &chromosome)) return;
	chromosome --;
	mask = 1 << chromosome;
	for (i = 0; i < NTotalParts; i ++) {
		j = (chromosome == 0 && i == ChordPart + 1)? ChordPart : i;
		if (CurrentBI == &IntegBarInfo && bInfoVary(genePart[j])) {
			set_text(poptionGPCValueTexts[i], "\p???");
		} else {
			gidx = CurrentBI->genePart[j][chromosome];
			SetControlValue(poptionGPCSliders[i], gidx + 1);
			set_digits(poptionGPCValueTexts[i], gidx + 1);
		}
		if (fieldp) set_pv_button_icon(poptionProtectButtons[i],
			(CurrentBI->protect[gidx] & mask)? IcnProtectOn : IcnProtectOff);
		j = (i < DsPercPart)? i : i + 1;
		if (partClosed[j] && CurrentBI->playOn[i])
			redraw_partialBgPane(j, 2, 1);
	}
	if (chromosome == 0) {
		DeactivateControl(poptionGPCSliders[ChordPart + 1]);
		DeactivateControl(poptionGPCValueTexts[ChordPart + 1]);
		DeactivateControl(poptionProtectButtons[ChordPart + 1]);
	} else {
		ActivateControl(poptionGPCSliders[ChordPart + 1]);
		ActivateControl(poptionGPCValueTexts[ChordPart + 1]);
		if (fieldp) ActivateControl(poptionProtectButtons[ChordPart + 1]);
	}
}
void pale_rgb(RGBColor *rgb) {
	rgb->red += (0xffff - rgb->red) * 3 / 4;
	rgb->green += (0xffff - rgb->green) * 3 / 4;
	rgb->blue += (0xffff - rgb->blue) * 3 / 4;
}
static void draw_small_icon(short iconResID, CIconHandle *cicn, short x, short y) {
	Rect	rect;
	if (!*cicn) {
		*cicn = GetCIcon(iconResID);
		if (!*cicn) { error_msg("\pGetCIcon in draw_cicon", iconResID); return; }
	}
	SetRect(&rect, x, y, x + 32, y + 32);
	PlotCIcon(&rect, *cicn);
}
static void setup_control_items(BarInfo *pi, short item) {
	short	i;
	item --;
	for (i = 0; i < NNoteCh; i ++) {
		if (pi == &IntegBarInfo && bInfoVary(control[i]))
			set_text(poptionCntlValueTexts[i], "\p???");
		else if (BarInfoUniq.control[i] != pi->control[i][item]) {
			BarInfoUniq.control[i] = pi->control[i][item];
			SetControlValue(poptionCntlSliders[i], pi->control[i][item]);
			set_digits(poptionCntlValueTexts[i], pi->control[i][item]);
			if (i < DsPercPart && partClosed[i] && pi->playOn[i])
				redraw_partialBgPane(i, 3, 1);
		}
	}
}
#define	oldBInfoNEQnew(x)	(BarInfoUniq.x[i] != pi->x[i])
#define	setBInfoUniq(x)		BarInfoUniq.x[i] = pi->x[i]
#define	integBInfoVary(x)	(pi == &IntegBarInfo && bInfoVary(x))
static void setup_play_items(BarInfo *pi) {
	short	i, k, err;
	for (i = 0; i < NTotalParts; i ++) {
		if (integBInfoVary(playOn[i]))
			set_pv_button_icon(poptionPlayButtons[i], IcnSpeakerUK);
		else if (oldBInfoNEQnew(playOn)) {
			setBInfoUniq(playOn);
			set_pv_button_icon(poptionPlayButtons[i], IcnSpeakerOff + pi->playOn[i]);
			if (partClosed[k = (i < DsPercPart)? i : i + 1])
				redraw_partialBgPane(k, -1, 1);
		}
	}
	err = GetBevelButtonMenuValue(poptionCntlMenuButton, &k);
	setup_control_items(pi, k);
	for (i = 0; i < NNoteCh; i ++) {
		if (integBInfoVary(instrument[i]))
			SetControlTitle(poptionInstruments[i], "\p???");
		else if (oldBInfoNEQnew(instrument)) {
			setBInfoUniq(instrument);
			set_gm_to_inst(pi->instrument[i], poptionInstruments[i]);
		}
		if (i < DsPercPart && partClosed[i]) redraw_partialBgPane(i, 0, 1);
	}
	for (i = 0; i < NDsPercParts; i ++) {
		if (integBInfoVary(drumsInst[i]))
			SetControlTitle(poptionInstruments[NNoteCh+i], "\p???");
		else if (oldBInfoNEQnew(drumsInst)) {
			setBInfoUniq(drumsInst);
			set_ds_inst_name(poptionInstruments[NNoteCh+i], pi->drumsInst[i]);
		}
		if (partClosed[k = i + DsPercPart]) redraw_partialBgPane(k, 0, 1);
	}
	for (i = 0; i < NNoteCh - 1; i ++) {
		if (integBInfoVary(octave[i]))
			SetControlTitle(poptionOctaveMenus[i], "\p?");
		else if (oldBInfoNEQnew(octave)) {
			unsigned char	title[4];
			setBInfoUniq(octave);
			SetBevelButtonMenuValue(poptionOctaveMenus[i], OctaveMax + 1 - pi->octave[i]);
			title[0] = 1; title[1] = '0' + pi->octave[i];
			SetControlTitle(poptionOctaveMenus[i], title);
		}
	}
	for (i = 0; i < NTotalParts; i ++) {
		if (integBInfoVary(unitBeat[i])) {
			ControlButtonContentInfo	cinfo;
			cinfo.contentType = kControlContentTextOnly;
			SetControlData(poptionUnitBeatMenus[i], kControlEntireControl,
				kControlBevelButtonContentTag, sizeof(cinfo), &cinfo);
			SetControlTitle(poptionUnitBeatMenus[i], "\p?");
		} else if (oldBInfoNEQnew(unitBeat)) {
			setBInfoUniq(unitBeat);
			err = SetBevelButtonMenuValue(poptionUnitBeatMenus[i], CurrentBI->unitBeat[i] + 1);
			SetControlTitle(poptionUnitBeatMenus[i], "\p");
			set_pv_button_icon(poptionUnitBeatMenus[i], IcnSixteenth + CurrentBI->unitBeat[i]);
		}
		if (integBInfoVary(iteration[i]))
			SetControlTitle(poptionIterationMenus[i], "\p?");
		else if (oldBInfoNEQnew(iteration)) {
			setBInfoUniq(iteration);
			err = SetBevelButtonMenuValue(poptionIterationMenus[i], CurrentBI->iteration[i] + 1);
			set_menu_button_title(poptionIterationMenus[i]);
		}
	}
	setup_gp_items(-1);
	{
		GrafPtr	oldPort;
		GetPort(&oldPort); SetPort(GetWindowPort(partoptionWin));
		for (i = 0; i < NTotalParts; i ++)
			if (!partClosed[(i < DsPercPart)? i : i + 1])
				draw_protection_indicators(i);
		ForeColor(blackColor);
		SetPort(oldPort);
	}
}
void setup_partoption_cntrl(void) {
	if (!partoptionWin) return;
	if (CurrentVI) setup_view_items(CurrentVI);
	if (CurrentBI) setup_play_items(CurrentBI);
}
static short calc_window_height(void) {
	short	i, n;
	for (i = n = 0; i <= NTotalParts; i ++)
		if (!partClosed[i]) n ++;
	return poptionTitleHeight
		+ poptionPartHeight * n + poptionClosedPartH * (NTotalParts + 1 - n);
}
static pascal void paint_fg(ControlRef control, short) {
	Rect	bound;
	short	id = GetControlReference(control) - poptionBG,
			part = (id > NNoteCh - 1)? id - 1 : id;
	RGBColor	rgb = partColor[part];
	RGBForeColor(&rgb);
	GetControlBounds(control, &bound);
	bound.left += poptionDsclsrWidth;
	FrameRect(&bound);
	InsetRect(&bound, 1, 1);
	pale_rgb(&rgb);
	RGBForeColor(&rgb);
	PaintRect(&bound);
	if (partClosed[id]) {
		unsigned char	name[64];
		short	a, x;
		static	CIconHandle	smallEye = NULL, smallSpeaker = NULL;
		static	RGBColor	blackRGB = {0,0,0};
		RGBForeColor(&blackRGB);
		TextSize(9);
		BlockMove(partIDNames[part], name + 1, 3);
		name[0] = name[3]? 3 : 2;
		MoveTo(poptionX_IDText + poptionIDTextWidth - StringWidth(name) - 1,
			bound.bottom);
		DrawString(name);
		if (CurrentVI) if (CurrentVI->viewOn[part])
			draw_small_icon(IcnSmallEye, &smallEye, poptionX_View + 3, bound.top);
		if (CurrentBI) if (CurrentBI->playOn[part]) {
			draw_small_icon(IcnSmallSpeaker, &smallSpeaker, poptionX_Play + 3, bound.top);
			GetBevelButtonMenuValue(poptionGPCMenuButton, &a);
			x = columnClosed[0]? columnX[0] + poptionClosedPartH : columnX[1];
			if (columnClosed[1]) x += poptionClosedPartH;
			else x += poptionUnitBeatWidth + poptionIterationWidth + poptionHPad * 2;
			if (columnClosed[2]) x += poptionClosedPartH;
			else {
				draw_small_icon(IcnSmallIndicator, &smallIndicator,
					x + poptionButtonSize + poptionGPCVTWidth + poptionHPad * 2 + 1
					+ CurrentBI->genePart[part][a-1], bound.top);
				x += columnX[3] - columnX[2];
			}
			if (id < NNoteCh && !columnClosed[3]) {
				GetBevelButtonMenuValue(poptionCntlMenuButton, &a);
				draw_small_icon(IcnSmallIndicator, &smallIndicator,
					x + poptionCntlVTWidth + poptionPrtctMPad + poptionPrtctMWidth
					+ poptionHPad + 1 + CurrentBI->control[id][a-1], bound.top);
			}
		}
		if (!columnClosed[0]) {
			GetControlTitle(poptionInstruments[id], name);
			x = (poptionInstrumentWidth - StringWidth(name)) / 2;
			if (x < 0) {
				for (a = name[0]; a > 0; a --)
					if (TextWidth(name, 1, a) <=
						poptionInstrumentWidth + poptionButtonSize + poptionHPad) break;
				x = 0;
			} else a = name[0];
			MoveTo(poptionX_Inst + x, bound.bottom);
			DrawText(name, 1, a);
		}
	} else if (CurrentBI)
		draw_protection_indicators(part); // protection indicators in shown part
}
static pascal void paint_bg(ControlRef control, ControlBackgroundPtr info) {
	ControlRef	parent;
	Rect	bound;
	RGBColor	rgb;
	long	id;
	OSErr	err = GetSuperControl(control, &parent);
	id = err? -1 : GetControlReference(parent) - poptionBG;
	if (!info->colorDevice || id < 0 || id > NTotalParts) return;
	rgb = partColor[(id > NNoteCh - 1)? id - 1 : id];
	pale_rgb(&rgb);
	RGBBackColor(&rgb);
	EraseRect(GetControlBounds(control, &bound));
}
static pascal void slider_feedback(ControlRef control, ControlPartCode part) {
	long	value, id = GetControlReference(control);
	switch (id & poptionKindMask) {
		case poptionGPCross: case poptionCntlSld:
		value = GetControlValue(control);
		switch (part) {
			case kControlPageDownPart:	SetControlValue(control, ++value); break;
			case kControlPageUpPart:	SetControlValue(control, --value); break;
		}
		switch (id & poptionKindMask) {
			case poptionGPCross:
			set_digits(poptionGPCValueTexts[id & poptionPartMask], value);
			break;
			case poptionCntlSld:
			set_digits(poptionCntlValueTexts[id & poptionPartMask], value);
		}
	}
}
static ControlRef create_bg(Rect *bound, ControlRef parent,
	ControlUserPaneBackgroundUPP *bgProc) {
	ControlRef	bgPaneTmp;
	OSStatus	status;
	status = CreateUserPaneControl(partoptionWin, bound,
		kControlHasSpecialBackground|kControlSupportsEmbedding, &bgPaneTmp);
	if (status) { error_msg("\pCreateUserPane for background", status); ExitToShell(); }
	if (parent) {
		status = EmbedControl(bgPaneTmp, parent);
		if (status) { error_msg("\pEmbedControl for bg", status); ExitToShell(); }
	}
	SetControlData(bgPaneTmp, kControlEntireControl, kControlUserPaneBackgroundProcTag,
		sizeof(ControlUserPaneBackgroundUPP), bgProc);
	return bgPaneTmp;
}
static void set_menu_small_font(ControlRef button) {
	MenuRef	mh;
	OSStatus	status;
	status = GetBevelButtonMenuHandle(button, &mh);
	if (status) {
		error_msg("\pGetBevelButtonMenuHandle in set_menu_font_size", status);
		return;
	}
	SetMenuFont(mh, 0, (on_MacOS_X? SmallFontSizeX : SmallFontSize9));
}
static ControlRef create_part_menu_button(Rect *bound, short x, short w,
	short menuID, ControlRef parent, long cref) {
	OSStatus	status;
	ControlRef	button;
	ControlButtonContentInfo	bcInfo = { kControlContentTextOnly };
	bound->left = x;
	bound->right = bound->left + w;
	if (menuID == mUnitBeat) {
		bcInfo.contentType = kControlContentCIconRes;
		bcInfo.u.resID = IcnSixteenth;
	}
	status = CreateBevelButtonControl(partoptionWin, bound, NULL,
		kControlBevelButtonSmallBevel, kControlBehaviorPushbutton,
		&bcInfo, menuID, kControlBehaviorSingleValueMenu,
		kControlBevelButtonMenuOnRight, &button);
	if (status) { error_msg("\pCreateBevelButtonControl", status); ExitToShell(); }
	if (menuID != mUnitBeat) {
		ControlFontStyleRec	fstyleSmall;
		fstyleSmall.flags = kControlUseSizeMask | kControlUseJustMask;
		fstyleSmall.size = SmallFontSizeX;
		status = SetControlFontStyle(button, &fstyleSmall);
	}
	set_menu_small_font(button);
	status = EmbedControl(button, parent);
	if (status) { error_msg("\pEmbedControl for length", status); ExitToShell(); }
	SetControlReference(button, cref);
	return button;
}
void open_part_option(void) {
	OSStatus	status;
	if (partoptionWin) {
		BringToFront(partoptionWin);
//		ShowWindow(partoptionWin);
		show_hide_window(partoptionWin, true);
//		activate_root_cntl(partoptionWin);
	} else {
		Rect	bound, rct, bnd2;
		short	i, k, smallFontDv, pHeight, sliderDY, sliderH;
		ControlButtonContentInfo	bcInfo;
		ControlUserPaneDrawUPP	fgProc;
		ControlUserPaneBackgroundUPP	bgProc;
		ControlActionUPP	livefbProc;
		ControlFontStyleRec	fstyleSmall;
		ControlRef	root, button, bgPaneTmp;
//
		for (i = 0; i < DsPercPart; i ++) partClosed[i] = !playDefault[i];
		partClosed[DsPercPart] = true;
		for (; i < NTotalParts; i ++) {
			if (playDefault[i]) partClosed[DsPercPart] = false;
			partClosed[i + 1] = !playDefault[i];
		}
		memset(&BarInfoUniq, -1, sizeof(BarInfoUniq));
//
		SetRect(&bound, 0, 0, poptionWidth, calc_window_height());
		OffsetRect(&bound, 20, -500);
		status = CreateNewWindow(kFloatingWindowClass,
			kWindowCloseBoxAttribute|kWindowHideOnSuspendAttribute,
			&bound, &partoptionWin);
		if (status) {
			error_msg("\pCreateNewWindow for part option", status);
			ExitToShell();
		}
		SetWTitle(partoptionWin, PartOptionTitle);
		SetWRefCon(partoptionWin, WIDPartOption);
		status = CreateRootControl(partoptionWin, &root);
		if (status)
			{ error_msg("\pCreateRootControl for partoption", status); ExitToShell(); }
		fgProc = NewControlUserPaneDrawUPP(paint_fg);
		bgProc = NewControlUserPaneBackgroundUPP(paint_bg);
		livefbProc = NewControlActionUPP(slider_feedback);
		fstyleSmall.flags = kControlUseSizeMask | kControlUseJustMask;
//		fstyleSmall.size = on_MacOS_X? SmallFontSizeX : SmallFontSize9;
		fstyleSmall.size = SmallFontSizeX;
		smallFontDv = (poptionButtonSize - fstyleSmall.size) / 2 - 1;
		sliderH = on_MacOS_X? SliderHeight_X : SliderHeight_9;
		sliderDY = (poptionPartHeight - sliderH) / 2;
// Disclosure button for rows
		{
			ControlButtonGraphicAlignment	align = kControlBevelButtonAlignRight;
			SetRect(&bound, 0, 0, poptionWidth, poptionClosedPartH);
			if ((status = CreateUserPaneControl( partoptionWin, &bound,
				kControlSupportsEmbedding, &poptionColumnDisclosuresRow)))
				{ error_msg("\pCreateUserPane in partoption", status); ExitToShell(); }
			bcInfo.contentType = kControlContentCIconRes;
			for (i = 0; i < NColumnDisclosures; i ++) {
				bound.left = columnX[i]; bound.right = columnX[i + 1];
				bcInfo.u.resID = columnClosed[i]? IcnColumnClosed : IcnColumnOpen;
				status = CreateBevelButtonControl(partoptionWin, &bound, NULL,
					kControlBevelButtonSmallBevel, kControlBehaviorPushbutton,
					&bcInfo, 0, 0, 0, &button);
				SetControlData(button, kControlButtonPart,
					kControlBevelButtonGraphicAlignTag, sizeof(align), &align);
				status = EmbedControl(button, poptionColumnDisclosuresRow);
				SetControlReference(button, poptionRDsclsr + i + columnID(i));
			}
		}
		bcInfo.contentType = kControlContentTextOnly;
		SetRect(&bound, 0, 0, poptionX_Inst, poptionClosedPartH);
		status = CreateBevelButtonControl(partoptionWin, &bound, NULL,
			kControlBevelButtonSmallBevel, kControlBehaviorPushbutton,
			&bcInfo, 0, 0, 0, &button);
		status = EmbedControl(button, root);
		DeactivateControl(button);
// title header
		SetRect(&rct, 0, poptionClosedPartH, poptionWidth, poptionTitleHeight);
		status = CreateWindowHeaderControl(partoptionWin, &rct, false, &poptionTitleHeader);
		status = EmbedControl(poptionTitleHeader, root);
// Disclosure menu
		SetRect(&bound, 0, poptionClosedPartH, poptionX_Inst, poptionTitleHeight);
		bcInfo.contentType = kControlContentTextOnly;
		status = CreateBevelButtonControl(partoptionWin, &bound, PartOptionLDisclosure,
			kControlBevelButtonNormalBevel, kControlBehaviorPushbutton,
			&bcInfo, mDisclosure, kControlBehaviorCommandMenu,
			kControlBevelButtonMenuOnBottom, &poptionDsclsrMenuButton);
		status = SetControlFontStyle(poptionDsclsrMenuButton, &fstyleSmall);
		status = EmbedControl(poptionDsclsrMenuButton, poptionTitleHeader);
		SetControlReference(poptionDsclsrMenuButton, poptionDsclsrMnBt);
		set_menu_small_font(poptionDsclsrMenuButton);
// Button title for Instruments & unit beat
		bnd2 = bound;
		bnd2.left = poptionX_Inst;
		bnd2.right = bnd2.left + poptionInstrumentWidth;
		fstyleSmall.just = teCenter;
		InsetRect(&bnd2, 10, 4);
		status = CreateStaticTextControl(partoptionWin, &bnd2,
			PartOptionInstTitle, &fstyleSmall, &button);
		SetControlReference(button, columnID(0));
		status = EmbedControl(button, poptionTitleHeader);
/*		fstyleSmall.just = teFlushRight;
		bnd2 = bound;
		bnd2.top = (bnd2.top + bnd2.bottom) / 2;
		status = CreateStaticTextControl(partoptionWin, &bnd2,
			PartOptionRangTitle, &fstyleSmall, &button);
		SetControlReference(button, columnID(0));
		status = EmbedControl(button, poptionTitleHeader);*/
// Chromosome type menu
		bound.left = poptionX_Prtct;
		bound.right = poptionX_CntlVT;
		status = CreateBevelButtonControl(partoptionWin, &bound, NULL,
			kControlBevelButtonNormalBevel, kControlBehaviorPushbutton,
			&bcInfo, mChromosome, kControlBehaviorSingleValueMenu,
			kControlBevelButtonMenuOnBottom, &poptionGPCMenuButton);
		status = SetControlFontStyle(poptionGPCMenuButton, &fstyleSmall);
		status = EmbedControl(poptionGPCMenuButton, poptionTitleHeader);
		SetControlReference(poptionGPCMenuButton, poptionGPCMnBt + columnID(2));
		SetBevelButtonMenuValue(poptionGPCMenuButton, 1);
		set_menu_button_title(poptionGPCMenuButton);
		set_control_help_tag(poptionGPCMenuButton, kHMOutsideTopLeftAligned, 164, htChromosomeMenu);
		set_menu_small_font(poptionGPCMenuButton);
// Control menu
		bound.left = poptionX_CntlVT;
		bound.right = poptionWidth - poptionCntlResetWidth;
		status = CreateBevelButtonControl(partoptionWin, &bound, NULL,
			kControlBevelButtonNormalBevel, kControlBehaviorPushbutton,
			&bcInfo, mControl, kControlBehaviorSingleValueMenu,
			kControlBevelButtonMenuOnBottom, &poptionCntlMenuButton);
		status = SetControlFontStyle(poptionCntlMenuButton, &fstyleSmall);
		status = EmbedControl(poptionCntlMenuButton, poptionTitleHeader);
		SetControlReference(poptionCntlMenuButton, poptionCntlMnBt + columnID(3));
		SetBevelButtonMenuValue(poptionCntlMenuButton, 1);
		set_menu_button_title(poptionCntlMenuButton);
		set_control_help_tag(poptionCntlMenuButton, kHMOutsideTopLeftAligned, 164, htControlMenu);
		set_menu_small_font(poptionCntlMenuButton);
// Control reset button
		bound.left = bound.right;
		bound.right = bound.left + poptionCntlResetWidth;
		status = CreateBevelButtonControl(partoptionWin, &bound, PartOptionLReset,
			kControlBevelButtonNormalBevel, kControlBehaviorPushbutton,
			&bcInfo, 0, 0, 0, &poptionResetButton);
		status = SetControlFontStyle(poptionResetButton, &fstyleSmall);
		status = EmbedControl(poptionResetButton, poptionTitleHeader);
		SetControlReference(poptionResetButton, poptionCntlRstBt + columnID(3));
		set_control_help_tag(poptionResetButton, kHMOutsideTopLeftAligned, 164, htResetControl);
///
		rct.top = poptionTitleHeight;
		bound.left = poptionX_Inst;
		bound.right = bound.left + poptionInstrumentWidth;
		bnd2.left = poptionX_Dsclsr;
		bnd2.right = bnd2.left + poptionDsclsrWidth;
		for (i = 0; i <= NTotalParts; i ++, rct.top = rct.bottom) {
// colored background
			rct.bottom = rct.top + (partClosed[i]? poptionClosedPartH : poptionPartHeight);
			if (i == NNoteCh - 1) {	// drums & percusstion title
				Rect	bnd3;
				if ((status = CreateWindowHeaderControl(
					partoptionWin, &rct, false, &poptionBGPane[i])))
					{ error_msg("\pCreateWindowHeader in partoption", status); ExitToShell(); }
				SetRect(&bnd3, poptionX_UnitBt, rct.top + poptionVPad + smallFontDv,
					poptionX_CntlVT - poptionHPad, rct.bottom - poptionVPad - smallFontDv);
				fstyleSmall.just = teCenter;
				status = CreateStaticTextControl(partoptionWin, &bnd3,
					PartOptionDsAndPerc, &fstyleSmall, &button);
				SetControlReference(button, columnID(1));
				status = EmbedControl(button, poptionBGPane[i]);
				if (partClosed[NNoteCh-1]) HideControl(button);
				fstyleSmall.just = teFlushRight;
			} else {
				if ((status = CreateUserPaneControl(
					partoptionWin, &rct, kControlSupportsEmbedding, &poptionBGPane[i])))
					{ error_msg("\pCreateUserPane in partoption", status); ExitToShell(); }
				SetControlData(poptionBGPane[i], kControlEntireControl,
					kControlUserPaneDrawProcTag, sizeof(fgProc), &fgProc);
			}
			SetControlReference(poptionBGPane[i], poptionBG + i);
			status = EmbedControl(poptionBGPane[i], root);
			if (status) { error_msg("\pEmbedControl for user pane", status); ExitToShell(); }
// part disclosure button
			bnd2.top = rct.top;
			bnd2.bottom = rct.bottom;
			bcInfo.contentType = kControlContentCIconRes;
			bcInfo.u.resID = partClosed[i]? IcnPartClosed : IcnPartOpen;
			status = CreateBevelButtonControl(partoptionWin, &bnd2, NULL,
				kControlBevelButtonSmallBevel, kControlBehaviorPushbutton,
				&bcInfo, 0, 0, 0, &poptionDsclsrButtons[i]);
			if (status) {
				error_msg("\pCreateStaticTextControl for Disclosure", status);
				ExitToShell();
			}
			status = EmbedControl(poptionDsclsrButtons[i], poptionBGPane[i]);
			if (status) { error_msg("\pEmbedControl for Disclosure", status); ExitToShell(); }
			SetControlReference(poptionDsclsrButtons[i], poptionDsclsr + i);
// Instrument button
			bound.top = rct.top + poptionVPad;
			bound.bottom = bound.top + poptionButtonSize;
			bcInfo.contentType = kControlContentTextOnly;
			status = CreateBevelButtonControl(partoptionWin, &bound, NULL,
				kControlBevelButtonSmallBevel, kControlBehaviorPushbutton,
				&bcInfo, 0, 0, 0, &poptionInstruments[i]);
			if (status) {
				error_msg("\pCreateBevelButtonControl for instrument button", status);
				ExitToShell();
			}
			status = SetControlFontStyle(poptionInstruments[i], &fstyleSmall);
			status = EmbedControl(poptionInstruments[i], poptionBGPane[i]);
			if (status) { error_msg("\pEmbedControl for inst", status); ExitToShell(); }
			SetControlReference(poptionInstruments[i], poptionInst + i + columnID(0));
			set_control_help_tag(poptionInstruments[i], kHMOutsideTopLeftAligned, 164,
				(i < DsPercPart)? htInstrument : ((i == DsPercPart)? htDrumKit : htPercussion));
			if (partClosed[i]) HideControl(poptionInstruments[i]);
		}
//
		bound.top = poptionTitleHeight + poptionVPad;
		for (i = k = 0; i < NTotalParts; i ++) {
			pHeight = partClosed[k]? poptionClosedPartH : poptionPartHeight;
			bound.bottom = bound.top + poptionButtonSize;
// part ID label text
			bound.left = poptionX_IDText;
			bound.right = bound.left + poptionIDTextWidth - 1;
			bnd2 = bound;
			InsetRect(&bnd2, 0, smallFontDv);
			bgPaneTmp = create_bg(&bnd2, poptionBGPane[k], &bgProc);
			status = CreateStaticTextControl(partoptionWin, &bnd2,
				CFStringCreateWithCString(NULL, partIDNames[i], kCFStringEncodingMacRoman),
				&fstyleSmall, &button);
			if (status) {
				error_msg("\pCreateStaticTextControl for part ID", status);
				ExitToShell();
			}
			status = EmbedControl(button, bgPaneTmp);
			if (status) { error_msg("\pEmbedControl for GPCVT", status); ExitToShell(); }
			if (partClosed[k]) HideControl(bgPaneTmp);
// View buttons
			bound.left = poptionX_View;
			bound.right = bound.left + poptionButtonSize;
			bcInfo.contentType = kControlContentCIconRes;
			bcInfo.u.resID = (i < NDispParts)? IcnEyeOpen : IcnEyeClosed;
			status = CreateBevelButtonControl(partoptionWin, &bound, NULL,
				kControlBevelButtonSmallBevel, kControlBehaviorPushbutton,
				&bcInfo, 0, 0, 0, &poptionViewButtons[i]);
			if (status) {
				error_msg("\pCreateBevelButtonControl for view button", status);
				ExitToShell();
			}
			status = EmbedControl(poptionViewButtons[i], poptionBGPane[k]);
			if (status) { error_msg("\pEmbedControl for view", status); ExitToShell(); }
			SetControlReference(poptionViewButtons[i], poptionView + i);
			set_control_help_tag(poptionViewButtons[i], kHMOutsideTopLeftAligned, 164, htView);
			if (partClosed[k]) HideControl(poptionViewButtons[i]);
// Play buttons
			bound.left = poptionX_Play;
			bound.right = bound.left + poptionButtonSize;
			bcInfo.u.resID = IcnSpeakerOn;
			status = CreateBevelButtonControl(partoptionWin, &bound, NULL,
				kControlBevelButtonSmallBevel, kControlBehaviorPushbutton,
				&bcInfo, 0, 0, 0, &poptionPlayButtons[i]);
			if (status) {
				error_msg("\pCreateBevelButtonControl for play button", status);
				ExitToShell();
			}
			status = EmbedControl(poptionPlayButtons[i], poptionBGPane[k]);
			if (status) { error_msg("\pEmbedControl for play", status); ExitToShell(); }
			SetControlReference(poptionPlayButtons[i], poptionPlay + i);
			set_control_help_tag(poptionPlayButtons[i], kHMOutsideTopLeftAligned, 164, htPlayOn);
			if (partClosed[k]) HideControl(poptionPlayButtons[i]);
// Protection buttons
			bound.left = poptionX_Prtct;
			bound.right = bound.left + poptionButtonSize;
			bcInfo.u.resID = IcnProtectOff;
			status = CreateBevelButtonControl(partoptionWin, &bound, NULL,
				kControlBevelButtonSmallBevel, kControlBehaviorPushbutton,
				&bcInfo, 0, 0, 0, &poptionProtectButtons[i]);
			if (status) {
				error_msg("\pCreateBevelButtonControl for protect button", status);
				ExitToShell();
			}
			status = EmbedControl(poptionProtectButtons[i], poptionBGPane[k]);
			if (status) { error_msg("\pEmbedControl for protect", status); ExitToShell(); }
			SetControlReference(poptionProtectButtons[i], poptionProtect + i + columnID(2));
			set_control_help_tag(poptionProtectButtons[i], kHMOutsideTopLeftAligned, 164, htProtectPart);
			if (partClosed[k]) HideControl(poptionProtectButtons[i]);
// Length menus
			poptionUnitBeatMenus[i] = create_part_menu_button(&bound,
				poptionX_UnitBt, poptionUnitBeatWidth, mUnitBeat,
				poptionBGPane[k], poptionUnitBeat + i + columnID(1));
			set_control_help_tag(poptionUnitBeatMenus[i], kHMOutsideTopLeftAligned, 164, htUnitBeat);
			if (partClosed[k]) HideControl(poptionUnitBeatMenus[i]);
			poptionIterationMenus[i] = create_part_menu_button(&bound,
				poptionX_Iterat, poptionIterationWidth, mIteration,
				poptionBGPane[k], poptionIterate + i + columnID(1));
			set_control_help_tag(poptionIterationMenus[i], kHMOutsideTopLeftAligned, 164, htIteration);
			if (partClosed[k]) HideControl(poptionIterationMenus[i]);
// Gene/Part cross value text
			bound.left = poptionX_GPCVT;
			bound.right = bound.left + poptionGPCVTWidth - 1;
			bnd2 = bound;
			InsetRect(&bnd2, 0, smallFontDv);
			if (!on_MacOS_X) OffsetRect(&bnd2, 0, -1);
			bgPaneTmp = create_bg(&bnd2, poptionBGPane[k], &bgProc);
			SetControlReference(bgPaneTmp, columnID(2));
			status = CreateStaticTextControl(partoptionWin, &bnd2, CFSTR("1"),
				&fstyleSmall, &poptionGPCValueTexts[i]);
			if (status) {
				error_msg("\pCreateStaticTextControl for GPCross", status);
				ExitToShell();
			}
			status = EmbedControl(poptionGPCValueTexts[i], bgPaneTmp);
			if (status) { error_msg("\pEmbedControl for GPCVT", status); ExitToShell(); }
			if (partClosed[k]) HideControl(bgPaneTmp);
// Gene/Part cross slider
			bnd2.left = poptionX_GPCSld;
			bnd2.right = bnd2.left + poptionGPCSldWidth;
			bnd2.top = bound.top + sliderDY - poptionVPad;
			bnd2.bottom = bnd2.top + sliderH;
			bgPaneTmp = create_bg(&bnd2, poptionBGPane[k], &bgProc);
			SetControlReference(bgPaneTmp, columnID(2));
			status = CreateSliderControl(partoptionWin, &bnd2, 1, 1, NTotalParts,
				kControlSliderDoesNotPoint, 0,
				true, livefbProc, &poptionGPCSliders[i]);
			if (status) {
				error_msg("\pCreateSliderControl for GPCross", status); ExitToShell();
			}
			SetControlReference(poptionGPCSliders[i], poptionGPCross + i);
			status = EmbedControl(poptionGPCSliders[i], bgPaneTmp);
			if (status) { error_msg("\pEmbedControl for GPCSld", status); ExitToShell(); }
			set_control_help_tag(poptionGPCSliders[i], kHMOutsideTopLeftAligned, 164, htGPCSld);
			if (partClosed[k]) HideControl(bgPaneTmp);
// set next Y
			if (i == DsPercPart - 1) {
				k += 2; bound.top += pHeight + poptionPartHeight;
			} else { k ++; bound.top += pHeight; }
		}
// Octave menu
		bound.top = poptionTitleHeight + poptionVPad;
		for (i = 0; i < NSoloParts+NChordParts; i ++) {
			bound.bottom = bound.top + poptionButtonSize;
			poptionOctaveMenus[i] = create_part_menu_button(&bound,
				poptionX_Octave, poptionOctaveWidth, mOctave,
				poptionBGPane[i], poptionOctave + i + columnID(0));
			set_control_help_tag(poptionOctaveMenus[i], kHMOutsideTopLeftAligned, 164, htOctave);
			if (partClosed[i]) {
				HideControl(poptionOctaveMenus[i]);
				bound.top += poptionClosedPartH;
			} else bound.top += poptionPartHeight;
		}
// Control value text and slider
		bound.top = poptionTitleHeight;
		bnd2.left = poptionX_CntlVT + 1;
		bnd2.right = poptionX_CntlVT + poptionCntlVTWidth - 1;
		rct.left = poptionX_CntlSld;
		rct.right = poptionX_CntlSld + poptionCntlSldWidth;
		for (i = 0; i < NNoteCh; i ++) {
			rct.top = bound.top + sliderDY;
			rct.bottom = rct.top + sliderH;
			bnd2.top = bound.top + poptionVPad + smallFontDv;
			bnd2.bottom = bound.top + poptionPartHeight - smallFontDv;
			if (!on_MacOS_X) OffsetRect(&bnd2, 0, -1);
			if (i < NNoteCh - 1) {
				bgPaneTmp = create_bg(&bnd2, poptionBGPane[i], &bgProc);
				SetControlReference(bgPaneTmp, columnID(3));
			}
			status = CreateStaticTextControl(partoptionWin, &bnd2, CFSTR("0"),
				&fstyleSmall, &poptionCntlValueTexts[i]);
			if (status) {
				error_msg("\pCreateStaticTextControl for Control", status);
				ExitToShell();
			}
			if (i < NNoteCh - 1)
				status = EmbedControl(poptionCntlValueTexts[i], bgPaneTmp);
			else {
				status = EmbedControl(poptionCntlValueTexts[i], poptionBGPane[i]);
				SetControlReference(poptionCntlValueTexts[i], columnID(3));
			}
			if (status) { error_msg("\pEmbedControl for CntlVT", status); ExitToShell(); }
			if (partClosed[i])
				HideControl((i < NNoteCh - 1)? bgPaneTmp : poptionCntlValueTexts[i]);
			if (i < NNoteCh - 1) {
				bgPaneTmp = create_bg(&rct, poptionBGPane[i], &bgProc);
				SetControlReference(bgPaneTmp, columnID(3));
			}
			status = CreateSliderControl(partoptionWin, &rct, 0, 0, 127,
				kControlSliderDoesNotPoint, 0,
				true, livefbProc, &poptionCntlSliders[i]);
			if (status) {
				error_msg("\pCreateSliderControl for Control", status);
				ExitToShell();
			}
			if (i < NNoteCh - 1) {
				status = EmbedControl(poptionCntlSliders[i], bgPaneTmp);
				SetControlReference(poptionCntlSliders[i], poptionCntlSld + i);
			} else {
				status = EmbedControl(poptionCntlSliders[i], poptionBGPane[i]);
				SetControlReference(poptionCntlSliders[i], poptionCntlSld + i + columnID(3));
			}
			if (status) { error_msg("\pEmbedControl for CntlSld", status); ExitToShell(); }
			set_control_help_tag(poptionCntlSliders[i], kHMOutsideTopLeftAligned, 164, htCntlSld);
			if (partClosed[i]) {
				HideControl((i < NNoteCh - 1)? bgPaneTmp : poptionCntlSliders[i]);
				bound.top += poptionClosedPartH;
			} else bound.top += poptionPartHeight;
		}
		setup_partoption_cntrl();
		move_win_right(partoptionWin);
	}
	CheckMenuItem(GetMenuHandle(mWindow), iPartOption, true);
	/*{
		FSSpec	fss;
		FSMakeFSSpec(0,0,"\pCntlHierachy",&fss);
		DumpControlHierarchy(GetDialogWindow(partoptionDlg), &fss);
	}*/
}
void close_part_option(void) {
	show_dialog_win(partoptionWin, false, iPartOption);
}
extern	Boolean	PlayLoop;
typedef struct {
	char		version[16];
	Boolean		playLoop;
	BarInfo		b;
	ViewInfo	v;
}	PartOptionImage;

void save_options(short ref) {
	long	n;
	PartOptionImage	img;
	OSErr	err;
	sprintf(img.version, "SBEAT%s\r", ProductVersionShort);
	img.playLoop = PlayLoop;
	img.b = *CurrentBI;
	img.v = *CurrentVI;
	n = sizeof(img);
	if ((err = FSWrite(ref, &n, &img)) != noErr)
		error_msg("\pwrite file", err);
}
Boolean read_options(FSSpec *fss, SBField *sf) {
	long	n;
	PartOptionImage	img;
	OSErr	err;
	short	ref;
	err = FSpOpenDF(fss, fsRdWrPerm, &ref);
	if (err) { res_msg(0, fss->name, 2, err); return 0; }
	n = sizeof(img);
	err = FSRead(ref, &n, &img);
	if (!err && sf) {
		n = sizeof(Individual) * PopulationSize;
		err = FSRead(ref, &n, sf->pop);
	}
	if (err != noErr)
		{ res_msg(0, fss->name, 3, err); goto error; }
	if (strncmp(img.version, "SBEAT", 5)
	 || strncmp(img.version+5, ProductVersionShort, 2))
		{ res_msg(0, fss->name, 4, 0); goto error; }
	if (img.b.keyScale > KeyMax) img.b.keyScale = KeyInitVal;
	if (PlayLoop != img.playLoop) {
		PlayLoop = img.playLoop;
		if (playerDlg) {
			ControlHandle	control;
			GetDialogItemAsControl(playerDlg, SFGroup, &control);
			SetControlValue(control, PlayLoop? 1 : 2);
		}
	}
	if (sf) {	// open field
		sf->b = img.b; sf->v = img.v;
		sf->refNum = ref;
	} else {	// load option
		FSClose(ref);
		enque_binfo_history();
		if (CurrentBI) *CurrentBI = img.b;
		if (CurrentVI) *CurrentVI = img.v;
		inject_controls(&img.b);
	}
	return true;
error:
	FSClose(ref);
	return false;
}
void load_options(FSSpec *fss) {
	if (!read_options(fss, NULL)) return;
	setup_partoption_cntrl();
	if (playerDlg) setup_player_cntrl();
	switch (GetWRefCon(TopWindow)) {
		case WIDField: devlop_all_score(FindField(TopWindow)); break;
		case WIDgedit: gedit_bi_changed(TopWindow, -1); break;
		case WIDIntegrator: {
			SBIntegrator	*si = FindIntegrator(TopWindow);
			if (CurrentBI && si && si->sel >= 0) {
				HLock((Handle)si->scoreHandle);
				(*si->scoreHandle)[si->sel].b = IntegBarInfo;
				HUnlock((Handle)si->scoreHandle);
			}
			update_integrator(si);
		} break;
	}
	play_restart();
	return;
}
void setup_current_po(BarInfo *bi, ViewInfo *vi) {
/* be called from "activate_field_control" and "activate_gedit" */
	if (bi != CurrentBI || vi != CurrentVI) {
		CurrentBI = bi;
		CurrentVI = vi;
		setup_partoption_cntrl();
		setup_tempo_and_key();
	}
}
#define	checkItgSame(x)	if (bInfoVary(x)) setBInfoNotSetYet(x)
#define	checkItgUniq(x)	if (IntegBarInfo.x != scorep[i].b.x) setBInfoVary(x)
#define	checkItgUniq2(x)	if (IntegBarInfo.x[k] != scorep[i].b.x[k]) setBInfoVary(x)
void set_current_integ_pi(SBIntegrator *si) {
	Boolean	on;
	short	i, j, k, nsecs = si->nSectionsH * si->nSectionsW;
	integScorePtr	scorep;
	HLock((Handle)si->scoreHandle);
	scorep = *si->scoreHandle;
	for (i = 0, on = false; i < nsecs; i ++) if (ItgSelectedOnP(scorep,i)) {
		if (!on) {
			IntegBarInfo = scorep[i].b;
			on = true;
			for (j = 0; j < NTotalParts; j ++) {
				checkItgSame(genePart[j]); checkItgSame(unitBeat[j]);
				checkItgSame(iteration[j]); checkItgSame(playOn[j]);
			}
			for (j = 0; j < NNoteCh; j ++)
				{ checkItgSame(instrument[j]); checkItgSame(control[j]); }
			for (j = 0; j < NNoteCh - 1; j ++) checkItgSame(octave[j]);
			for (j = 0; j < NDsPercParts; j ++) checkItgSame(drumsInst[j]);
			checkItgSame(keyNote);
			checkItgSame(tempo);
			checkItgSame(keyScale);
			checkItgSame(ETScale);
		} else {
			GetBevelButtonMenuValue(poptionGPCMenuButton, &k); k --;
			for (j = 0; j < NTotalParts; j ++) {
				checkItgUniq2(genePart[j]); checkItgUniq(unitBeat[j]);
				checkItgUniq(iteration[j]); checkItgUniq(playOn[j]);
			}
			GetBevelButtonMenuValue(poptionCntlMenuButton, &k); k --;
			for (j = 0; j < NNoteCh; j ++)
				{ checkItgUniq(instrument[j]); checkItgUniq2(control[j]); }
			for (j = 0; j < NNoteCh - 1; j ++) checkItgUniq(octave[j]);
			for (j = 0; j < NDsPercParts; j ++) checkItgUniq(drumsInst[j]);
			checkItgUniq(keyNote);
			checkItgUniq(tempo);
			if ((IntegBarInfo.keyScale ^ scorep[i].b.keyScale) & M8ScaleMask)
				BarInfoUniq.keyScale = -1;
			if ((IntegBarInfo.keyScale ^ scorep[i].b.keyScale) & ETScaleMask)
				BarInfoUniq.ETScale = -1;
		}
	}
	HUnlock((Handle)si->scoreHandle);
	if (on) {
		CurrentBI = &IntegBarInfo;
		if (partoptionWin) setup_play_items(CurrentBI);
	} else CurrentBI = NULL;
	setup_tempo_and_key();
}
void set_current_integ_po(SBIntegrator *si) {
	CurrentVI = &si->vInfo;
	set_current_integ_pi(si);
	if (partoptionWin) setup_view_items(CurrentVI);
}
void setup_part_menu_item(short part, MenuRef mh) {
	char	*p, *q;
	unsigned char	text[64];
	if (!mh) mh = get_gedit_part_menu();
	text[0] = 0;
	for (p = (char *)text + 1, q = partIDNames[part]; *q; p++, q++)
		{ p[0] = q[0]; text[0] ++; }
	GetControlTitle(poptionInstruments[(part < DsPercPart)? part : part + 1],
		(unsigned char *)p);
	text[0] += p[0] + 1;
	p[0] = ' ';
	SetMenuItemText(mh, part + 1, text);
}
void click_display_sw(short part) {
	ControlHandle	button;
	short	k, i, j, min, pre;
	unsigned char	*que = CurrentVI->viewQue;
	Boolean	modified = false;
	long	winType = GetWRefCon(TopWindow);
	button = poptionViewButtons[part];
	if (CurrentVI->viewOn[part]) {
		if (que[NDispParts - 1] == part) return;	// half open eye -> no action
		if (winType == WIDgedit) {
			SBgedit	*sg = FindGedit(TopWindow);
			if (!sg || sg->editPart == part) return;	// gedit part -> no action
		}
		set_pv_button_icon(poptionViewButtons[que[NDispParts - 1]], IcnEyeOpen);
		for (i = j = 0; i < NDispParts - 1; i ++)
			if (j || que[i] == part) { que[i] = que[i + 1]; j = 1; }
		if (j) que[NDispParts - 1] = part;
		set_pv_button_icon(button, IcnEyeHalf);
		modified = true;
	} else {
		set_pv_button_icon(button, IcnEyeOpen);
		CurrentVI->viewOn[part] = true;
		k = que[NDispParts - 1];
		set_pv_button_icon(poptionViewButtons[k], IcnEyeClosed);
		CurrentVI->viewOn[k] = false;
		if (k >= DsPercPart) k ++;
		if (partClosed[k]) redraw_partialBgPane(k, -1, 1);
		for (i = NDispParts - 1; i > 0; i --) que[i] = que[i - 1];
		que[0] = part;
		pre = -9999;
		for (i = 0; i < NDispParts; i ++) {
			min = 9999;
			for (j = 0; j < NDispParts; j ++)
				if (pre < que[j] && que[j] < min) min = que[j];
			CurrentVI->viewPartID[i] = pre = min;
		}
		if (winType == WIDgedit) {
			SBgedit	*sg = FindGedit(TopWindow);
			if (sg && sg->editPart == que[NDispParts - 1]) {
				que[NDispParts - 1] = que[NDispParts - 2];
				que[NDispParts - 2] = sg->editPart;
			}
			k = (part >= DsPercPart)? part + 1 : part;
			if (partClosed[k]) redraw_partialBgPane(k, -1, 1);
		}
		set_pv_button_icon(poptionViewButtons[que[NDispParts - 1]], IcnEyeHalf);
		switch (winType) {
			case WIDField: draw_window(FindField(TopWindow)); break;
			case WIDIntegrator: update_integrator(FindIntegrator(TopWindow)); break;
			case WIDgedit: update_gedit(TopWindow);
		}
		modified = true;
	}
	if (modified) switch (winType) {
		case WIDField: field_modified(FindField(TopWindow)); break;
		case WIDIntegrator: integ_modified(FindIntegrator(TopWindow)); break;
		case WIDgedit: gedit_modified(FindGedit(TopWindow));
	}
}
typedef	enum	{
	ChItgInfoNone, ChItgInfoRedraw, ChItgInfoRedevelop
}	ChItgInfoOption;
static void change_integ_info(
	short part, void *addr, short size, ChItgInfoOption option) {
	SBIntegrator	*si = FindIntegrator(TopWindow);
	integScorePtr	scorep;
	short	i, row, column, idx = (char *)addr - (char *)CurrentBI;
	Boolean	modified = false, row_modified;
	GrafPtr	oldPort;
	if (!si) return;
	GetPort(&oldPort); SetPort(GetWindowPort(TopWindow));
	HLock((Handle)si->scoreHandle);
	scorep = *si->scoreHandle;
	for (row = i = 0; row < si->nSectionsH; row ++) {
		row_modified = false;
		for (column = 0; column < si->nSectionsW; column ++, i ++)
		if (ItgSelectedOnP(scorep,i)) {
			BlockMove(addr, (char *)(&scorep[i].b) + idx, size);
			if (option == ChItgInfoRedevelop)
				develop_score(&scorep[i].i, &scorep[i].b);
			modified = row_modified = true;
		}
		if (row_modified && option != ChItgInfoNone && si->vInfo.viewOn[part])
			check_integ_score_head(si, scorep, row);
			for (column = 0; column < si->nSectionsW; column ++)
				draw_integ_ind(si, row * si->nSectionsW + column, true);
	}
	HUnlock((Handle)si->scoreHandle);
	SetPort(oldPort);
	if (modified) integ_modified(si);
}
static void click_playOn(short part) {
	SBField	*sf;
	SBgedit	*sg;
	enque_binfo_history();
	CurrentBI->playOn[part] = BarInfoUniq.playOn[part] = !CurrentBI->playOn[part];
	set_pv_button_icon(poptionPlayButtons[part],
		IcnSpeakerOff + CurrentBI->playOn[part]);
	switch (GetWRefCon(TopWindow)) {
		case WIDIntegrator: change_integ_info(part, &CurrentBI->playOn[part],
			sizeof(CurrentBI->playOn[0]), ChItgInfoRedraw);
		break;
		case WIDField: sf = FindField(TopWindow); if (!sf) break;
		field_modified(sf); if (sf->v.viewOn[part]) draw_window(sf); break;
		case WIDgedit: sg = FindGedit(TopWindow); if (!sg) break;
		gedit_modified(sg); if (sg->v.viewOn[part]) update_gedit(TopWindow);
	}
	play_restart();
}
static void click_protect(short part, short modifiers) {
	short	chromosome, geneIdx, iconID, i, chrmsm;
	unsigned char	mask;
	OSErr	err;
	err = GetBevelButtonMenuValue(poptionGPCMenuButton, &chromosome);
	if (err) { error_msg("\pGetBevelButtonMenuValue in click_protect", err); return; }
	enque_binfo_history();
	chromosome --;
	geneIdx = CurrentBI->genePart[part][chromosome];
	mask = 1 << chromosome;
	CurrentBI->protect[geneIdx] ^= mask;
	iconID = (CurrentBI->protect[geneIdx] & mask)? IcnProtectOn : IcnProtectOff;
	ForeColor((iconID == IcnProtectOn)? blackColor : whiteColor);
	for (i = 0; i < NTotalParts; i ++)
	if (CurrentBI->genePart[i][chromosome] == geneIdx) {
		set_pv_button_icon(poptionProtectButtons[i], iconID);
		draw_protection_indicator(i, chromosome);
	}
	if (modifiers & shiftKey)
		for (chrmsm = 0, mask = 1; chrmsm < 3; chrmsm ++, mask <<= 1)
		if (chrmsm != chromosome) {
			geneIdx = CurrentBI->genePart[part][chrmsm];
			switch (iconID) {
				case IcnProtectOn: CurrentBI->protect[geneIdx] |= mask; break;
				case IcnProtectOff: CurrentBI->protect[geneIdx] &= ~mask;
			}
			for (i = 0; i < NTotalParts; i ++)
			if (CurrentBI->genePart[i][chrmsm] == geneIdx)
				draw_protection_indicator(i, chrmsm);
		}
	if (iconID == IcnProtectOff) ForeColor(blackColor);
	field_modified(FindField(TopWindow));
}
static void click_instrument_menu(short part) {
	long	oldGM, newGM;
	reset_my_cursor();
	oldGM = CurrentBI->instrument[part];
	newGM = new_instrument(partIDNames[part], oldGM, poptionInstruments[part]);
	if (newGM < 0) return;
	if (newGM != oldGM
	 || (CurrentBI == &IntegBarInfo && bInfoVary(instrument[part]))) {
		long	winType = GetWRefCon(TopWindow);
		enque_binfo_history();
		CurrentBI->instrument[part] = BarInfoUniq.instrument[part] = newGM; //@@@@
		switch (winType) {
			case WIDIntegrator: change_integ_info(part, &CurrentBI->instrument[part],
				sizeof(CurrentBI->instrument[0]), ChItgInfoNone);
			break;
			case WIDField: field_modified(FindField(TopWindow)); break;
			case WIDgedit: setup_part_menu_item(part, NULL);
			gedit_modified(FindGedit(TopWindow));
		}
		if ((TopWindow == PlayWindow || !PlayWindow) && winType != WIDIntegrator) {
			change_instrument(part, CurrentBI);
			play_restart();
		}
	}
}
static void click_drumsInst_menu(short id) {
	SBField	*sf;
	SBgedit	*sg;
	long	oldKit, newKit = -1;
	short	part = id - NNoteCh;
	set_my_cursor(CURSsystem);
	oldKit = CurrentBI->drumsInst[part];
	newKit = new_drums_instrument(partIDNames[id-1], oldKit, poptionInstruments[id]);
	if (newKit < 0) return;
	if (newKit != oldKit
	 || (CurrentBI == &IntegBarInfo && bInfoVary(drumsInst[part]))) {
		long	winType = GetWRefCon(TopWindow);
		enque_binfo_history();
		CurrentBI->drumsInst[part] = BarInfoUniq.drumsInst[part] = newKit; //@@@@
		switch (winType) {
			case WIDIntegrator: change_integ_info(part, &CurrentBI->drumsInst[part],
				sizeof(CurrentBI->drumsInst[0]), ChItgInfoRedraw);
			break;
			case WIDField: sf = FindField(TopWindow); if (!sf) break;
			field_modified(sf); if (sf->v.viewOn[part+DsPercPart]) draw_window(sf); break;
			case WIDgedit: sg = FindGedit(TopWindow); if (!sg) break;
			setup_part_menu_item(id - 1, NULL);
			gedit_modified(sg); if (sg->v.viewOn[part+DsPercPart]) update_gedit(TopWindow);
		}
		if ((TopWindow == PlayWindow || !PlayWindow) && winType != WIDIntegrator) {
			play_restart();
		}
	}
}
static void click_octave_menu(short part) {
	unsigned char	title[4];
	short	value;
	OSErr	err = GetBevelButtonMenuValue(poptionOctaveMenus[part], &value);
	reset_my_cursor();
	if (err) return;
	enque_binfo_history();
	value = OctaveMax + 1 - value;
	CurrentBI->octave[part] = BarInfoUniq.octave[part] = value;
	title[0] = 1; title[1] = '0' + value;
	SetControlTitle(poptionOctaveMenus[part], title);
	switch (GetWRefCon(TopWindow)) {
		case WIDIntegrator: change_integ_info(part, &CurrentBI->octave[part],
			sizeof(CurrentBI->octave[0]), ChItgInfoRedraw);
		break;
		case WIDField: field_modified(FindField(TopWindow));
		case WIDgedit:
		if (TopWindow == PlayWindow) play_restart();
	}
}
static void click_control_value(short part) {
	long	value = GetControlValue(poptionCntlSliders[part]);
	short	cntl;
	OSErr	err = GetBevelButtonMenuValue(poptionCntlMenuButton, &cntl);
	if (err) return;
	enque_binfo_history();
	cntl --;
	CurrentBI->control[part][cntl] = BarInfoUniq.control[part] = value;
	set_digits(poptionCntlValueTexts[part], value);
	switch (GetWRefCon(TopWindow)) {
		case WIDIntegrator: change_integ_info(part, &CurrentBI->control[part][cntl],
			sizeof(CurrentBI->control[0][0]), ChItgInfoNone);
		break;
		case WIDField: field_modified(FindField(TopWindow));
		case WIDgedit:
		if (TopWindow == PlayWindow)
			{ set_qtma_control(cntl, part, value); play_restart(); }
	}
}
static void breed_info_changed(short part, void *addr, short size) {
	SBField	*sf;
	switch (GetWRefCon(TopWindow)) {
		case WIDIntegrator:
		change_integ_info(part, addr, size, ChItgInfoRedevelop);
		break;
		case WIDField: sf = FindField(TopWindow);
		if (!sf) return;
		devlop_all_score(sf);
		play_restart();
		field_modified(sf);
		break;
		case WIDgedit: gedit_bi_changed(TopWindow, part);
	}
}
static void click_unitBeat_menu(short part) {
	short	value;
	OSErr	err;
	ControlRef	button = poptionUnitBeatMenus[part];
	reset_my_cursor();
	err = GetBevelButtonMenuValue(button, &value);
	if (err) return;
	enque_binfo_history();
	CurrentBI->unitBeat[part] = BarInfoUniq.unitBeat[part] = value - 1;
	set_pv_button_icon(button, IcnSixteenth + value - 1);
	breed_info_changed(part, &CurrentBI->unitBeat[part], sizeof(CurrentBI->unitBeat[0]));
}
static void click_iteration_menu(short part) {
	short	value;
	OSErr	err;
	ControlRef	button = poptionIterationMenus[part];
	reset_my_cursor();
	err = GetBevelButtonMenuValue(button, &value);
	if (err) return;
	enque_binfo_history();
	CurrentBI->iteration[part] = BarInfoUniq.iteration[part] = value - 1;
	set_menu_button_title(button);
	breed_info_changed(part, &CurrentBI->iteration[part], sizeof(CurrentBI->iteration[0]));
}
static void click_gpcross_value(short part) {
	long	value = GetControlValue(poptionGPCSliders[part]);
	short	chromosome;
	unsigned char	gpOld, gpNew, mask;
	OSErr	err = GetBevelButtonMenuValue(poptionGPCMenuButton, &chromosome);
	if (err) return;
	enque_binfo_history();
	chromosome --;
	gpOld = CurrentBI->genePart[part][chromosome];
	gpNew = value - 1;
	mask = 1 << chromosome;
	if ((CurrentBI->protect[gpOld] ^ CurrentBI->protect[gpNew]) & mask) {
		short iconID = (CurrentBI->protect[gpNew] & mask)? IcnProtectOn : IcnProtectOff;
		set_pv_button_icon(poptionProtectButtons[part], iconID);
		if (iconID == IcnProtectOff) ForeColor(whiteColor);
		draw_protection_indicator(part, chromosome);
		if (iconID == IcnProtectOff) ForeColor(blackColor);
	}
	CurrentBI->genePart[part][chromosome] = BarInfoUniq.genePart[part] = gpNew;
	set_digits(poptionGPCValueTexts[part], value);
	if (chromosome == 0 && part == ChordPart) {
		SetControlValue(poptionGPCSliders[ChordPart + 1], value);
		set_digits(poptionGPCValueTexts[ChordPart + 1], value);
	}
	breed_info_changed(part,
		&CurrentBI->genePart[part][chromosome], sizeof(CurrentBI->genePart[0][0]));
}
static void show_sub_controls(short id, Boolean show) {
	short	i, column, err;
	unsigned short	n;
	ControlRef	button, subControl;
	button = poptionDsclsrButtons[id];
	if ((err = CountSubControls(poptionBGPane[id], &n)) != noErr)
		{ error_msg("\pCountSubControls in show_sub_controls", err); return; }
	for (i = 1; i <= n; i ++) {
		err = GetIndexedSubControl(poptionBGPane[id], i, &subControl);
		if (err == noErr && subControl != button) {
			column = getColumnID(GetControlReference(subControl));
			if (column < 0 || !columnClosed[column]) {
				if (show) ShowControl(subControl);
				else HideControl(subControl);
			}
		}
	}
	if (show) {
		SizeControl(button, poptionDsclsrWidth, poptionPartHeight);
		set_pv_button_icon(button, IcnPartOpen);
	} else {
		set_pv_button_icon(button, IcnPartClosed);
		SizeControl(button, poptionDsclsrWidth, poptionClosedPartH);
	}
}
static void offset_control(ControlRef pane, short dh, short dv) {
	Rect	bound;
	GetControlBounds(pane, &bound);
	MoveControl(pane, bound.left + dh, bound.top + dv);
}
static void click_disclosure(short id) {
	short	i, dv = poptionPartHeight - poptionClosedPartH;
	Rect	bound;
	partClosed[id] = !partClosed[id];
	GetWindowPortBounds(partoptionWin, &bound);
	if (partClosed[id]) {
		show_sub_controls(id, false);
		SizeControl(poptionBGPane[id], poptionWidth, poptionClosedPartH);
		for (i = id + 1; i <= NTotalParts; i ++)
			offset_control(poptionBGPane[i], 0, -dv);
		SizeWindow(partoptionWin, bound.right, bound.bottom - dv, false);
	} else {
		SizeWindow(partoptionWin, bound.right, bound.bottom + dv, false);
		for (i = NTotalParts; i > id; i --)
			offset_control(poptionBGPane[i], 0, dv);
		SizeControl(poptionBGPane[id], poptionWidth, poptionPartHeight);
		show_sub_controls(id, true);
	}
}
static void show_column_sub_controls(ControlRef parent, short column, Boolean show) {
	short	i, err;
	unsigned short	n;
	ControlRef	subControl;
	if ((err = CountSubControls(parent, &n)) != noErr)
		{ error_msg("\pCountSubControls in show_column_sub_controls", err); return; }
	for (i = 1; i <= n; i ++)
		if ((err = GetIndexedSubControl(parent, i, &subControl)) == noErr)
			if (getColumnID(GetControlReference(subControl)) == column) {
				if (show) ShowControl(subControl);
				else HideControl(subControl);
			}
}
static void show_column_controls(short column, Boolean show) {
	short	id;
	show_column_sub_controls(poptionTitleHeader, column, show);
	for (id = 0; id < NTotalParts + 1; id ++) if (!partClosed[id])
		show_column_sub_controls(poptionBGPane[id], column, show);
}
static void shift_column_sub_controls(ControlRef parent, short column, short dh) {
	short	i, err;
	unsigned short	n;
	ControlRef	subControl;
	if ((err = CountSubControls(parent, &n)) != noErr)
		{ error_msg("\pCountSubControls in shift_column_sub_controls", err); return; }
	for (i = 1; i <= n; i ++)
		if ((err = GetIndexedSubControl(parent, i, &subControl)) == noErr)
			if (getColumnID(GetControlReference(subControl)) > column)
				offset_control(subControl, dh, 0);
}
static void shift_column_controls(short column, short dh) {
	short	id;
	shift_column_sub_controls(poptionColumnDisclosuresRow, column, dh);
	shift_column_sub_controls(poptionTitleHeader, column, dh);
	for (id = 0; id < NTotalParts + 1; id ++)
		shift_column_sub_controls(poptionBGPane[id], column, dh);
}
static void click_column_disclosure(ControlRef button, short id) {
	short	expandedW = columnX[id + 1] - columnX[id],
			dh = expandedW - poptionClosedPartH;
	Rect	bound, buttonRect, scroll;
	Point	pt;
	short	i;
	columnClosed[id] = !columnClosed[id];
	GetWindowPortBounds(partoptionWin, &bound);
	GetControlBounds(button, &buttonRect);
	SetRect(&scroll, buttonRect.right, 0, bound.right, bound.bottom);
	pt.h = pt.v = 0;
	LocalToGlobal(&pt);
	if (columnClosed[id]) {
		set_pv_button_icon(button, IcnColumnClosed);
		SizeControl(button, poptionClosedPartH, poptionClosedPartH);
		show_column_controls(id, false);
		ScrollWindowRect(partoptionWin, &scroll, -dh, 0, 0, NULL);
		shift_column_controls(id, -dh);
		MoveWindow(partoptionWin, pt.h + dh, pt.v, false);
		SizeWindow(partoptionWin, bound.right - dh, bound.bottom, false);
		if (id == 0) for (i = 0; i < NTotalParts + 1; i ++)
			if (i != DsPercPart && partClosed[i]) redraw_partialBgPane(i, 0, 1);
	} else {
		MoveWindow(partoptionWin, pt.h - dh, pt.v, false);
		SizeWindow(partoptionWin, bound.right + dh, bound.bottom, false);
		ScrollWindowRect(partoptionWin, &scroll, dh, 0, 0, NULL);
		shift_column_controls(id, dh);
		SizeControl(button, expandedW, poptionClosedPartH);
		set_pv_button_icon(button, IcnColumnOpen);
		SetRect(&scroll, buttonRect.left, poptionClosedPartH,
			buttonRect.left + expandedW, bound.bottom);
		InvalWindowRect(partoptionWin, &scroll);
		show_column_controls(id, true);
	}
}
static void show_all_parts(void) {
	short	i;
	Rect	bound;
	GetWindowPortBounds(partoptionWin, &bound);
	SizeWindow(partoptionWin, bound.right, poptionHeight, false);
	SetRect(&bound, 0, poptionHeight - poptionPartHeight,
		poptionWidth, poptionHeight);
	for (i = NTotalParts; i >= 0; i --) {
//		SetControlBounds(poptionBGPane[i], &bound);
		MoveControl(poptionBGPane[i], bound.left, bound.top);
		SizeControl(poptionBGPane[i], poptionWidth, poptionPartHeight);
		OffsetRect(&bound, 0, -poptionPartHeight);
		if (partClosed[i]) {
			partClosed[i] = false;
			show_sub_controls(i, true);
		}
	}
	DrawControls(partoptionWin);
}
static void show_played_parts(void) {
#define	shouldDisclose	2
	short	i, n, y;
	Rect	bound;
	if (partClosed[DsPercPart]) for (i = DsPercPart; i < NTotalParts; i ++)
		if (CurrentBI->playOn[i]) partClosed[DsPercPart] = shouldDisclose;
	for (i = DsPercPart; i < NTotalParts; i ++)
		if (CurrentBI->playOn[i] && partClosed[i + 1]) partClosed[i + 1] = shouldDisclose;
	for (i = 0; i < NNoteCh; i ++)
		if (CurrentBI->playOn[i] && partClosed[i]) partClosed[i] = shouldDisclose;
	for (i = n = 0; i <= NTotalParts; i ++)
		if (partClosed[i] == true) n ++;
	y = poptionTitleHeight
		+ poptionClosedPartH * n + poptionPartHeight * (NTotalParts + 1 - n);
	GetWindowPortBounds(partoptionWin, &bound);
	SizeWindow(partoptionWin, bound.right, y, false);
	for (i = NTotalParts; i >= 0; i --) {
		switch (partClosed[i]) {
			case false: case shouldDisclose: y -= poptionPartHeight; break;
			case true: y -= poptionClosedPartH; break;
		}
		MoveControl(poptionBGPane[i], 0, y);
		if (partClosed[i] == shouldDisclose) {
			partClosed[i] = false;
			SizeControl(poptionBGPane[i], poptionWidth, poptionPartHeight);
			show_sub_controls(i, true);
		}
	}
}
static void hide_silent_parts(void) {
#define	shouldClose	3
	short	i, n, y;
	Rect	bound;
	Boolean	moveFlag;
	if (!partClosed[DsPercPart]) {
		for (i = DsPercPart; i < NTotalParts; i ++)
			if (CurrentBI->playOn[i]) break;
		if (i >= NTotalParts) partClosed[DsPercPart] = shouldClose;
	}
	for (i = DsPercPart; i < NTotalParts; i ++)
		if (!CurrentBI->playOn[i] && !partClosed[i + 1]) partClosed[i + 1] = shouldClose;
	for (i = 0; i < NNoteCh; i ++)
		if (!CurrentBI->playOn[i] && !partClosed[i]) partClosed[i] = shouldClose;
	for (i = n = 0; i <= NTotalParts; i ++)
		if (partClosed[i] == false) n ++;
	y = poptionTitleHeight;
	moveFlag = false;
	for (i = 0; i <= NTotalParts; i ++) {
		if (moveFlag) MoveControl(poptionBGPane[i], 0, y);
		if (partClosed[i] == shouldClose) {
			show_sub_controls(i, false);
			SizeControl(poptionBGPane[i], poptionWidth, poptionClosedPartH);
			partClosed[i] = true;
			moveFlag = true;
		}
		y += partClosed[i]? poptionClosedPartH : poptionPartHeight;
	}
	GetWindowPortBounds(partoptionWin, &bound);
	SizeWindow(partoptionWin, bound.right, y, false);
}
static void hide_all_parts(void) {
	short	i;
	Rect	bound;
	SetRect(&bound, 0, poptionTitleHeight,
		poptionWidth, poptionTitleHeight + poptionClosedPartH);
	for (i = 0; i <= NTotalParts; i ++) {
		if (!partClosed[i]) {
			partClosed[i] = true;
			show_sub_controls(i, false);
		}
//		SetControlBounds(poptionBGPane[i], &bound);
		MoveControl(poptionBGPane[i], bound.left, bound.top);
		SizeControl(poptionBGPane[i], poptionWidth, poptionClosedPartH);
		OffsetRect(&bound, 0, poptionClosedPartH);
	}
	GetWindowPortBounds(partoptionWin, &bound);
	SizeWindow(partoptionWin, bound.right, poptionTitleHeight
		+ poptionClosedPartH * (NTotalParts + 1), false);
	DrawControls(partoptionWin);
}
static Boolean temp_play(short id, Fixed volume) {
	EventRecord	event;
	if (!PlayWindow) return false;
	if (CurrentBI) if (!CurrentBI->playOn[id]) return false;
	play_one_channel(id, volume);
	do {
		WaitNextEvent(mUpMask|updateMask, &event, 4, NULL);
		switch (event.what) {
			case updateEvt: DoUpdate(&event); break;
			case nullEvent: check_playing(); check_integ_sel();
		}
	} while (event.what != mouseUp);
	play_one_channel(id, 0x10000);
	return true;
}
static void push_play_button(EventRecord *e, short id, ControlRef button) {
	Boolean	doClick = false;
	Boolean	active = IsControlActive(button);
	if (active) HiliteControl(button, kControlButtonPart);
	if (e->modifiers & (controlKey|shiftKey)) temp_play(id, 0);
	else if (e->modifiers & optionKey) temp_play(id, 0x04000);
	else if (e->modifiers & cmdKey) temp_play(id, 0x08000);
	else {
		EventRecord	event;
		WaitNextEvent(mUpMask, &event, GetDblTime(), NULL);
		switch (event.what) {
			case nullEvent: temp_play(id, 0); break;
			case mouseUp: doClick = true;
		}
	}
	if (active) {
		HiliteControl(button, kControlNoPart);
		if (doClick) click_playOn(id);
	}
}
static void check_part_option(EventRecord *e, Point pt, short part,
	ControlRef control, long ckind, short id) {
	short	item, err;
	if (part != kControlNoPart) {
		part = HandleControlClick(control, pt, e->modifiers, (ControlActionUPP)-1);
		if (part <= kControlNoPart || kControlDisabledPart <= part) return;
	}
	switch (ckind) {
		case poptionTitle:
		switch (id) {
			case poptionGPCMnBt:
			item = set_menu_button_title(poptionGPCMenuButton);
			if (item > 0) setup_gp_items(item);
			reset_my_cursor();
			break;
			case poptionCntlMnBt:
			item = set_menu_button_title(poptionCntlMenuButton);
			if (item > 0) setup_control_items(CurrentBI, item);
			reset_my_cursor();
			break;
			case poptionCntlRstBt: init_part_control(CurrentBI);
			err = GetBevelButtonMenuValue(poptionCntlMenuButton, &item);
			if (!err) setup_control_items(CurrentBI, item);
			break;
			case poptionDsclsrMnBt:
			err = GetBevelButtonMenuValue(poptionDsclsrMenuButton, &item);
			reset_my_cursor();
			if (!err) switch (item) {
				case iShowAllParts: show_all_parts(); break;
				case iShowOnlyPlayParts: hide_silent_parts();
				case iShowPlayParts: show_played_parts(); break;
				case iHideSilentParts: hide_silent_parts(); break;
				case iHideAllParts: hide_all_parts();
			}
		} break;
		case poptionView: click_display_sw(id); break;
		case poptionProtect: click_protect(id, e->modifiers); break;
		case poptionUnitBeat: click_unitBeat_menu(id); break;
		case poptionIterate: click_iteration_menu(id); break;
		case poptionGPCross: click_gpcross_value(id); break;
		case poptionInst:
		if (id < NNoteCh) click_instrument_menu(id);
		else click_drumsInst_menu(id);
		break;
		case poptionOctave: click_octave_menu(id); break;
		case poptionCntlSld: click_control_value(id); break;
		case poptionDsclsr: click_disclosure(id); break;
		case poptionRDsclsr: click_column_disclosure(control, id); break;
	}
}
static void check_surround(Rect *rct, ControlRef button) {
	Rect	box;
	if (!IsControlActive(button)) return;
	GetControlBounds(button, &box);
	if (box.bottom > rct->top && box.top < rct->bottom
	 && box.right > rct->left && box.left < rct->right)
		HiliteControl(button, kControlButtonPart);
	else HiliteControl(button, kControlNoPart);
}
static Boolean get_rectangle(EventRecord *e, Point startpt) {
	Point	currentPt;
	Rect	mouseRct, rct;
	EventRecord	event;
	RgnHandle	mouseRgn;
	short	i, chromosome;
	Pattern	grayPat;
	unsigned long	mask, protectFlag = 0;

	PenMode(patXor); PenSize(2,2);
	PenPat(GetQDGlobalsGray(&grayPat));
	ForeColor(blackColor);
	SetRect(&rct, startpt.h, startpt.v, startpt.h, startpt.v);
	SetRect(&mouseRct, e->where.h, e->where.v, e->where.h+1, e->where.v+1);
	mouseRgn = NewRgn();
	do {
		RectRgn(mouseRgn, &mouseRct);
		WaitNextEvent(mUpMask|osMask|updateMask, &event, 4, mouseRgn);
		switch (event.what) {
			case osEvt: if ((event.message >> 24) != mouseMovedMessage) break;
			currentPt = event.where;
			GlobalToLocal(&currentPt);
			FrameRect(&rct);
			for (i = 0; i < NTotalParts; i ++)
			if (!partClosed[(i < NNoteCh - 1)? i : i + 1]) {
				check_surround(&rct, poptionPlayButtons[i]);
				check_surround(&rct, poptionViewButtons[i]);
				check_surround(&rct, poptionProtectButtons[i]);
			}
			if (startpt.h < currentPt.h) { rct.left = startpt.h; rct.right = currentPt.h; }
			else { rct.right = startpt.h; rct.left = currentPt.h; }
			if (startpt.v < currentPt.v) { rct.top = startpt.v; rct.bottom = currentPt.v; }
			else { rct.bottom = startpt.v; rct.top = currentPt.v; }
			FrameRect(&rct);
			SetRect(&mouseRct, event.where.h, event.where.v, event.where.h+1, event.where.v+1);
			break;
			case updateEvt: DoUpdate(&event); break;
			case nullEvent: check_playing(); check_integ_sel();
		}
	} while (event.what != mouseUp);
	FrameRect(&rct);
	DisposeRgn(mouseRgn);
	PenNormal();
	if (rct.top == rct.bottom && rct.left == rct.right) return false;
	GetBevelButtonMenuValue(poptionGPCMenuButton, &chromosome); chromosome --;
	for (i = 0; i < NTotalParts; i ++) if (!partClosed[(i < NNoteCh - 1)? i : i + 1]) {
		if (GetControlHilite(poptionPlayButtons[i]) == kControlButtonPart) {
			HiliteControl(poptionPlayButtons[i], kControlNoPart);
			click_playOn(i);
		}
		if (GetControlHilite(poptionViewButtons[i]) == kControlButtonPart) {
			HiliteControl(poptionViewButtons[i], kControlNoPart);
			click_display_sw(i);
		}
		if (GetControlHilite(poptionProtectButtons[i]) == kControlButtonPart) {
			HiliteControl(poptionProtectButtons[i], kControlNoPart);
			mask = 1 << CurrentBI->genePart[i][chromosome];
			if (!(protectFlag & mask)) {
				click_protect(i, event.modifiers);
				protectFlag |= mask;
			}
		}
	}
	return true;
}
void click_part_option(EventRecord *e) {
	Point	localPt = e->where;
	ControlRef	control;
	long	ckind;
	short	pc, id;
	GrafPtr	oldPtr;
	GetPort(&oldPtr);
	SetPort(GetWindowPort(partoptionWin));
	GlobalToLocal(&localPt);
	control = FindControlUnderMouse(localPt, partoptionWin, &pc);
	ckind = control? GetControlReference(control) : 0;
	id = ckind & poptionPartMask;
	ckind &= poptionKindMask;
	if (ckind == poptionPlay) push_play_button(e, id, control);
	else if (kControlNoPart < pc && pc < kControlInactivePart && control)
		check_part_option(e, localPt, pc, control, ckind, id);
	else if (!get_rectangle(e, localPt))
		if (ckind == poptionBG && partClosed[id]) click_disclosure(id);
	SetPort(oldPtr);
}
static void activate_controls(ControlRef *cntl, short n, Boolean activate) {
	short	i;
	for (i = 0; i < n; i ++) {
		if (activate) ActivateControl(cntl[i]);
		else DeactivateControl(cntl[i]);
	}
}
static void activate_dialog_item(DialogPtr dlg, short itemID, Boolean activate) {
	ControlHandle	ch;
	if (!dlg) return;
	GetDialogItemAsControl(dlg, itemID, &ch);
	if (activate) ActivateControl(ch);
	else DeactivateControl(ch);
}
void activate_poption_items(Boolean activate) {
	static	Boolean	prevActivation = true;
	if (!partoptionWin || activate == prevActivation) return;
	prevActivation = activate;
	activate_controls(poptionTitleButtons, NtitleButtons, activate);
	activate_controls(poptionPlayButtons, NTotalParts, activate);
	activate_controls(poptionUnitBeatMenus, NTotalParts, activate);
	activate_controls(poptionIterationMenus, NTotalParts, activate);
	activate_controls(poptionGPCValueTexts, NTotalParts, activate);
	activate_controls(poptionGPCSliders, NTotalParts, activate);
	activate_controls(poptionInstruments, NTotalParts+1, activate);
	activate_controls(poptionOctaveMenus, NSoloParts+NChordParts, activate);
	activate_controls(poptionCntlValueTexts, NNoteCh, activate);
	activate_controls(poptionCntlSliders, NNoteCh, activate);
/*	{
		ControlRef	root;
		GetRootControl(partoptionWin, &root);
		if (activate) ActivateControl(root);
		else DeactivateControl(root);
		activate_controls(poptionViewButtons, NTotalParts, true);
	}*/
	activate_dialog_item(playerDlg, STPane, activate);
	activate_dialog_item(playerDlg, SKPane, activate);
}
void activate_protection(Boolean activate) {
	static	Boolean	prevActivation = true;
	if (!partoptionWin || activate == prevActivation) return;
	prevActivation = activate;
	activate_controls(poptionProtectButtons, NTotalParts, activate);
}
