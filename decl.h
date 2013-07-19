//#define  DEBUG

#include	"ftype.h"

#define	kBaseResID	128
#define	kMoveToFront		(WindowRef)-1L
#define mApple				kBaseResID
#define	mFile		(mApple+1)
#define	iSaveFile	5
#define	iSaveAs		(iSaveFile+1)
#define	iSaveAsSMF	(iSaveAs+1)
#define	iSaveOption	(iSaveAsSMF+1)
#define	iReset		(iSaveOption+2)
#define	mEdit		(mFile+1)
#define	iUndo		1
#define	iCut		(iUndo+2)
#define	iCopy		(iCut+1)
#define	iPaste		(iCopy+1)
#define	iPasteOpt	(iPaste+1)
#define	iClear		(iPasteOpt+1)
#define	iSelectAll	(iClear+1)
#define	iGeneEdit	(iSelectAll+2)
#define	iScoreEdit	(iGeneEdit+1)
#define	nMItemsEdit	iScoreEdit
#define	mBreed		(mEdit+1)
#define	iDoItAgain	1
#define	iNextThis	3
#define	iNextNew	4
#define	iMutate		6
#define	mWindow		(mBreed+1)
#define	iPartOption	1
#define	iPlayer		(iPartOption+1)
#define	iWindowList	(iPlayer+2)
#define	mHelp		(mWindow+1)
#define	iManual		1
#define	iWebSite	(iManual+1)
#define	mScoreEdit	(mHelp+1)
#define	iNoteUp		1
#define	iNoteDown	(iNoteUp+1)
#define	iShiftRight	(iNoteDown+1)
#define	iShiftLeft	(iShiftRight+1)
#define	iExpand		(iShiftLeft+1)
#define	iShrink		(iExpand+1)
#define	nMItemsScoreEdit	iShrink
enum {
	mUnitBeat = mScoreEdit+1,
	mControl, mChromosome, mOctave,
	mKeyNote, mMelodyShft, mDrumStickTimbres,
	mDisclosure, mPartNames,
	mIteration, mPlayerKeyNote,
	mTomTomTimbres, mCrashTimbres
};
/* item number in alert box */
#define	itemOK		1
#define	itemCancel	2
#define	itemNoOp	3

enum {	WIDField =		1,
		WIDIntegrator,
		WIDgedit,
		WIDPlayer,
		WIDPartOption,
		WIDgeditDlg,
		WIDModalDlg,
		WIDOpenning,
		WIDIntegSel2 =	-1
};
#define	FieldNColumns	3
#define	FieldNRows		3
#define	PopulationSize	(FieldNColumns*FieldNRows)
#define	ScoreLinePad	4
#define	NoteWUnit		10
#define	FldControlH		18
#define	FldControlPad	2
#define	IndControlH		(FldControlH+FldControlPad*2)
#define	PartHeight		(ScoreLinePad*10)
#define	NDispParts		4
#define	SubWinWidth		((NShortBeats+1)*NoteWUnit)
#define	SubWinPHeight	(PartHeight*NDispParts)
#define	SubWinHeight	(IndControlH+SubWinPHeight)
#define	WinLeftSpace	(NoteWUnit*3)
#define	WinTopSpace		(FldControlH+FldControlPad*2)
#define	SubWinBorder	3
#define	SubWinPad		2
#define	WinWidth	((SubWinWidth+SubWinBorder*2+SubWinPad)*FieldNColumns-SubWinPad+WinLeftSpace)
#define	WinHeight	((SubWinHeight+SubWinBorder*2+SubWinPad)*FieldNRows-SubWinPad+WinTopSpace)
#define	SubWinYUnit	(SubWinHeight+SubWinBorder*2+SubWinPad)
#define	SubWinXUnit	(SubWinWidth+SubWinBorder*2+SubWinPad)
#define	IntegYPad		10
#define	IntegYUnit		(SubWinPHeight+IntegYPad)
#define	DefaultDispScale	2

#define	IntControlH		18
#define	IntControlW		22
#define	IntControlPadH	2
#define	IntControlPadW	6
#define	intWinTopSpace		(IntControlH+IntControlPadH*2+1)
#define	ScrollBarWidth	16
#define	NSectionsW	4
#define	SelFrameWidth	1
#define	IntWindowWidth	(SubWinWidth*NSectionsW+WinLeftSpace)
#define	IntWindowHeight	(IntegYUnit)
#define	NintButtons	13
enum {
		CntlPalm =	1, CntlSelArrow,	
		CntlPlay, CntlStop, CntlPause,
		CntlNoteUp, CntlNoteDn,
		CntlShiftL, CntlShiftR,
		CntlShrink, CntlExpand,
		CntlLarger, CntlSmaller
};
#define	GrowIconSize	15

#define	MaxPolyPhony	4
#define	NSoloParts		13
#define	NChordParts		2
#define	NDsPercParts	8
#define	NTotalParts		(NSoloParts+NChordParts+NDsPercParts)
#define	NNoteCh			(NSoloParts+NChordParts+1)
#define	NShortBeats		16

#define	NoteRemain	-1
#define	NoteRest	-2
#define	SoloPart	0
#define	ChordPart	NSoloParts
#define	DsPercPart	(ChordPart+NChordParts)	/* 2 bytes */
#define	GnFraction	(NSoloParts+NChordParts+NDsPercParts)
#define	BytesParNote	(NTotalParts*2)

#define	ControlMax	127
#define	OctaveMax	8
#define	TempoMin	20
#define	TempoMax	180
#define	TempoInitVal	100
#define	KeyMin	0
#define	KeyMax	14
#define	KeyInitVal	7
enum {
	KeyNoteVariable = 1,
	KeyNoteNotUniq,
	KeyNoteMin,
	KeyNoteMax = KeyNoteMin + 7
};
enum {
	CURSsystem = -1,
	CURSfinger = 0,
	CURSgrabber,
	CURSclosed,
	CURSpick,
	CURSselect,
	CURSselArrw,
	CURSeditText,
	NCursors
};

#define	NControls	5
typedef struct {
	Boolean	viewOn[NTotalParts];
	unsigned char	viewPartID[NDispParts], viewQue[NDispParts];
}	ViewInfo;
typedef	struct {
	unsigned char	protect[NTotalParts];
	unsigned char	genePart[NTotalParts][3];
	unsigned char	unitBeat[NTotalParts];
	unsigned char	iteration[NTotalParts];
	unsigned char	keyNote;
	Boolean	playOn[NTotalParts];
	long	instrument[NNoteCh];
	long	drumsInst[NDsPercParts];
	unsigned char	octave[NNoteCh-1];
	unsigned char	control[NNoteCh][NControls];
	unsigned char	tempo, keyScale;
}	BarInfo;

typedef	unsigned char	Gene;
typedef struct	{
	short	note;
	char	velocity;
	char	effect;
}	Score;
typedef struct {
	Gene	gene[BytesParNote][NShortBeats];
	Score	score[NTotalParts][NShortBeats];
}	Individual;
typedef struct	{
	Individual	i;
	BarInfo		b;
	unsigned char	flag;
}	integScoreRec;
typedef	integScoreRec *	integScorePtr;
typedef	integScorePtr *	integScoreHandle;
typedef	void (*ScoreProc)(Score *, BarInfo *, short *, short);
#define	ItgSelectedFlag	1
#define	ItgOnFlag		2
#define	ItgTmpSelectedFlag	4
#define	ItgSelectedP(s,ns)		((s)[ns].flag & ItgSelectedFlag)
#define	ItgOnP(s,ns)			((s)[ns].flag & ItgOnFlag)
#define	ItgSelectedOnP(s,ns)	(((s)[ns].flag & (ItgSelectedFlag|ItgOnFlag)) == (ItgSelectedFlag|ItgOnFlag))
#define	ItgTmpSelectedP(s,ns)	((s)[ns].flag & ItgTmpSelectedFlag)

#define	SelectedFlag	1
#define	HadSelectedFlag	2
#define	ProtectedFlag	4
#define	SelectedP(sf,id)		((sf)->sel[id] & SelectedFlag)
#define	ProtectedP(sf,id)		((sf)->sel[id] & ProtectedFlag)
#define	SetSelected(sf,id)		((sf)->sel[id] |= SelectedFlag)
#define	ResetSelected(sf,id)	((sf)->sel[id] &= ~SelectedFlag)
#define	SetProtected(sf,id)		((sf)->sel[id] |= ProtectedFlag)
#define	ResetProtected(sf,id)	((sf)->sel[id] &= ~ProtectedFlag)
typedef	enum { NewFile, NotModifiedFile, ModifiedFile } FileMode;
typedef enum { GOpNothing, GOpNextInThis, GOpNextInNew, GOpMutateIt } GeneratingOp;

typedef struct SBField_rec {
	struct SBField_rec	*pre, *post;
	unsigned long	act;	/* to find window activated latest */
	WindowRef	win;
	Individual	pop[PopulationSize];
	ControlHandle	root, next_pane, all_speaker,
		speaker[PopulationSize],
		selButton[PopulationSize],
		ptcButton[PopulationSize];
	unsigned char	sel[PopulationSize];
	Boolean	PlayAll;
	short	PlayID;
	short	BtnOnID;
	FSSpec	fsSpec;
	short	refNum;
	FileMode	fmode;
	GeneratingOp	prevOp;
	BarInfo		b;
	ViewInfo	v;
}	SBField;

typedef	struct SBInteg_rec {
	struct SBInteg_rec	*pre, *post;
	unsigned long	act;
	WindowRef	win;
	ControlHandle	root, scrollBar, bottomBar, bottomText,
					button[NintButtons];
	integScoreHandle	scoreHandle;
	short	PlayID;
	short	sel;		/* bar ID of primary selection, 0- */
	short	cursor_mode;
	short	scroll;
	short	nSectionsH;	/* the number of rows */
	short	nSectionsW;	/* the number of columns */
	short	nBars;		/* the number of bars in the space */
	short	nSelBars;	/* the number of selected bars */
	short	winHeight;
	short	displayScale, barHeight, barWidth;
	short	animeTime;
	short	origWinHeight;
	Point	winPosition;
	FSSpec	fsSpec;
	short	refNum;
	FileMode	fmode;
	ViewInfo	vInfo;
}	SBIntegrator;

typedef	struct SBgedit_rec {
	struct	SBgedit_rec	*pre, *post;
	unsigned long	act;
	WindowRef	win;
	SBField			*sf;
	SBIntegrator	*si;
	short		id, editPart;
	Individual	ind;
	Boolean		playing;
	FileMode	fmode;
	BarInfo		b;
	ViewInfo	v;
}	SBgedit;

#define	PlayTypeFieldP(win)	(GetWRefCon(win) == WIDField)

typedef	enum { HistEmpty,
	HistFInd, HistFPop, HistFBarInfo,
	HistIOneBar, HistIBars, HistIBarInfo,
	HistILeft, HistIRight, HistIShrink, HistNoteShift,
	HistGInd, HistGBarInfo
}	HistEventType;

typedef struct {
	Str31	instName;
	BigEndianLong	instNumber;
}	InstrumentList;

typedef enum {
	PstOptNoNeed, PstOptDone, PstOptNeedRedraw, PstOptNothing
}	PasteOptionRet;

typedef	struct {
	Boolean	adjust;
	short	pause;
	unsigned char	copyRight[64];
}	smfOptionalInfo;

/* copy_paste.c */
void rm_history(WindowRef);
void enque_field_pop_history(SBField *);
void enque_field_ind_history(SBField *, short);
void enque_gedit_history(SBgedit *);
void enque_binfo_history(void);
void enque_note_shift_history(SBIntegrator *, integScorePtr);
void enque_integ_history(HistEventType, SBIntegrator *, short, short);
void UndoCB(void);
void CutCB(void);
void CopyCB(void);
void PasteCB(void);
void ClearCB(void);
void SelectAllCB(void);
PasteOptionRet copy_option_f(short,
	ViewInfo *, BarInfo *,
	ViewInfo *, BarInfo *);
void PasteOptionCB(void);

/* drag.c */
void DropFileInit(void);
void field_part_to_rect(short, short, Rect *);
void set_drag_callback(WindowRef);
void remove_drag_callback(WindowRef);
void gedit_part_to_rect(short, Rect *);
void integ_part_to_rect(short, short, SBIntegrator *, Rect *);
void drag_field_ind(SBField *, EventRecord *);
void drag_gedit_ind(SBgedit *, EventRecord *);
void drag_integ_ind(SBIntegrator *, EventRecord *);

/* draw.c */
void draw_color_frame(short, short);
void draw_frame(SBField *, short);
void inverse_playing_mark(void);
void draw_window(SBField *);
void update_field(SBField *);
void draw_field_ind(SBField *, short);
void begin_integ_update(void);
void end_integ_update(void);
void draw_integ_ind(SBIntegrator *, short, Boolean);
void draw_integ_scores(SBIntegrator *);
void check_integ_score_head(SBIntegrator *, integScorePtr, short);
void draw_gedit(SBgedit *);

/* evolv.c */
void pop_init(SBField *);
void check_disable_DoItAgain(SBField *);
void reset_pop(SBField *);
void MutateCB(void);
void next_generation(SBField *, SBField *);
void DoItAgainCB(void);

/* field.c */
void set_win_menu_item(WindowRef);
void select_win_menu_item(short);
void check_win_menu_item(WindowRef);
void clear_speaker_icon(SBField *);
void set_control_help_tag(ControlHandle, short, short, short);
void untitled_str(unsigned char *, long *);
void setup_new_field(SBField *, unsigned char *);
void NewFieldCB(void);
void next_in_new(SBField *);
Boolean equiv_fss(FSSpec *, FSSpec *);
void open_field(FSSpec *);
Boolean save_field_as(SBField *);
Boolean save_field(SBField *);
Boolean draw_temp_drag_frame(Rect *, Point);
void close_sb_window(WindowRef);
void close_field(SBField *);
void field_modified(SBField *);
short find_primary(SBField *);
void field_ind_changed(SBField *, short);
void activate_field_controls(SBField *, Boolean);
void flip_sel_mode(SBField *, short);
void click_field(SBField *, EventRecord *);
void keyin_field(SBField *, EventRecord *);

/* file.c */
SBField *FindField(WindowRef);
SBIntegrator *FindIntegrator(WindowRef);
SBField *new_field_rec(void);
SBIntegrator *new_integrator_rec(void);
void free_field(SBField *);
void free_integrator(SBIntegrator *);
void for_all_fields(void (*proc)(SBField *));
void open_from_fss(FSSpec *, OSType);
Boolean open_from_AE_data(AEDescList *);
void OpenCB(void);
void SaveCB(void);
void SaveAsCB(void);
void SaveAsSMFCB(void);
void SaveOptionsCB(void);
Boolean save_as_setup(WindowRef, FSSpec *, short *, OSType, unsigned char t[]);

/* gedit.c */
SBgedit *FindGedit(WindowRef);
Boolean setup_gedit(SBgedit *);
void open_gedit(void);
void close_gedit(SBgedit *);
void update_gedit(WindowRef);
void click_gedit(SBgedit *, EventRecord *);
void switch_GEplay_button(Boolean);
MenuHandle get_gedit_part_menu(void);
void activate_gedit(WindowRef, Boolean);
void paste_gedit(WindowRef, Individual *, BarInfo *, ViewInfo *);
Boolean save_gedit_option(SBgedit *);
void check_gedit_dlg(EventRecord *);
void click_gedit_dlg(EventRecord *);
void gedit_modified(SBgedit *);
void gedit_bi_changed(WindowRef, short);

/* integrator.c */
void NewIntegratorCB(void);
void open_integrator(FSSpec *);
Boolean save_integrator_as(SBIntegrator *);
Boolean save_integrator(SBIntegrator *);
void close_integrator(SBIntegrator *);
void reset_integrator(SBIntegrator *);
void check_integ_sel(void);
void draw_sel_frame(SBIntegrator *, Boolean);
void activate_integ_controls(SBIntegrator *, Boolean);
void update_integrator(SBIntegrator *);
void activate_integ_sel_ctrl(SBIntegrator *, Boolean, Boolean);
Boolean check_integ_selected_on(SBIntegrator *, Boolean *);
void activate_integ_sel2_ctrl(SBIntegrator *);
short get_cursor_mode(SBIntegrator *, short);
void check_integ_playing(SBIntegrator *);
void drag_integ_scroll(SBIntegrator *, short, short);
void click_integrator(SBIntegrator *, EventRecord *);
void keyin_integrator(SBIntegrator *, EventRecord *);
void integ_modified(SBIntegrator *);
Boolean set_integ_pm_rect(SBIntegrator *, Rect *);
void grow_integrator(SBIntegrator *, EventRecord *);
void zoom_integ(SBIntegrator *);
Boolean expand_integ(SBIntegrator *, short);
void score_edit(short);
void integ_select_all(SBIntegrator *);

/* main.c */
void hilite_button(DialogRef, short);
void res_msg(short, unsigned char *, short, short);
void error_msg(unsigned char *, OSErr);
short save_change_alert(WindowRef, short);
Boolean is_checked(short);
void DoUpdate(EventRecord *);
Boolean front_is_dialog(void);
void setup_edit_menu(Boolean, Boolean, short);
void setup_field_menu(SBField *);
void reset_topwindow(void);
void set_activate_count(unsigned long *);
void reset_my_cursor(void);
void set_my_cursor(short);
void activate_root_cntl(WindowRef);
#define	activateRootCntl(dlg)	activate_root_cntl(GetDialogWindow(dlg))
#ifdef	DEBUG
void event_monitor(char *, EventRecord *);
#endif

/* partoption.c */
void show_hide_window(WindowRef, Boolean);
void check_no_window(void);
void check_first_window(void);
void set_default_poption(SBField *);
void pale_rgb(RGBColor *);
void setup_partoption_cntrl(void);
void open_part_option(void);
void close_part_option(void);
void save_options(short);
Boolean read_options(FSSpec *, SBField *);
void load_options(FSSpec *);
void setup_current_po(BarInfo *, ViewInfo *);
void set_current_integ_pi(SBIntegrator *);
void set_current_integ_po(SBIntegrator *);
void setup_part_menu_item(short, MenuHandle);
void click_display_sw(short);
void click_part_option(EventRecord *);
void activate_poption_items(Boolean);
void activate_protection(Boolean);

/* player.c */
typedef struct {
	ControlRef	control;
	short	itemID, partCode;
}	dialogSelectRec;
void move_win_right(WindowRef);
void set_key_note_txt(unsigned char, unsigned char *);
void setup_key_note_menu(MenuHandle, short, short *);
void setup_tempo_and_key(void);
void setup_player_cntrl(void);
void set_small_font(DialogRef, short, short, short);
void open_player(void);
void close_player(void);
void player_text_changed(void);
Boolean keyin_player(EventRecord *);
Boolean click_player(EventRecord *);
Boolean dialog_select(EventRecord *, DialogRef, dialogSelectRec *);
Boolean check_player(EventRecord *);

/* score.c */
void develop_score(Individual *, BarInfo *);
void devlop_all_score(SBField *);
void proc_score(Score s[NTotalParts][NShortBeats], BarInfo *, ScoreProc);

/* seq.c */
Boolean cymbalP(short, short, BarInfo *);
short my_note(short, short, BarInfo *);
Boolean init_tp(void);
short get_timbre(short, short, BarInfo *);
short get_timbre_id(short, short, BarInfo *);
void play_sequence(WindowRef, short);
void play_selected_score(WindowRef);
void stop_tune(void);
void change_play_loop(void);
void revise_tune_queue(WindowRef, short);
void play_restart(void);
void change_instrument(short, BarInfo *);
void set_gm_to_inst(long, ControlRef);
long new_instrument(char *, long, ControlRef);
void set_ds_inst_name(ControlRef, long);
long new_drums_instrument(char *, long, ControlRef);
void init_part_control(BarInfo *);
void inject_controls(BarInfo *);
void play_one_channel(short, Fixed);
void set_qtma_control(short, short, short);
void check_playing(void);

/* smf.c */
void write_smf(WindowRef, short, smfOptionalInfo *);

/* preference.c */
void setup_preferences(void);

#define	EnvInit()	InitCursor()
#define	myGetWPort(win)	GetWindowPort(win)
#define	myGetDPort(win)	GetDialogPort(win)
#define	myHMGetHelpMenu(mh, item)	HMGetHelpMenu(&mh, &item)
