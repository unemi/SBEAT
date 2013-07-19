#include  <stdio.h>
#include	<string.h>
#include	<QuickTimeMusic.h>
#include	"decl.h"
#include	"timbre.h"
#include	"vers.h"
#define	SoloNoteW	1
#define	ChordNoteW	3
#define	DsPercNoteW	2
#define	MaxNNotes	(SoloNoteW*NSoloParts+ChordNoteW*NChordParts+DsPercNoteW*NDsPercParts)
#define	TuneSize	(NShortBeats*(MaxNNotes+1)+NNoteCh*(2+NControls))
#define	MaxInstMenuSize	48
#define	InstMenuID	200
#define	FirstDsPitch	26

static	TunePlayer tp = 0;
static	NoteAllocator	NoteAlct = 0;
static	NoteChannel	NoteCh[NNoteCh];
static	short	NextID = -1;
static	WindowPtr	NextWin = NULL;
short	playID = -1, play1 = -1, play2 = 0;
WindowPtr	PlayWindow = NULL;
enum { PlayStopped, PlayOne, PlayAndQueued } Playing = PlayStopped;
Boolean	PlayAll = false, PlayLoop = true;
short	defaultInstrument[] = {
	74,	/* Soprano1	= Flute */
	57,	/* Soprano2	= Trumpet */
	65,	/* Soprano3	= Soprano Sax */
	57,	/* Alto1	= Trumpet */
	66,	/* Alto2	= Alto Sax */
	72,	/* Alto3	= Clarinet */
	58,	/* Tenor1	= Trombone */
	67,	/* Tenor2	= Tenor Sax */
	72,	/* Tenor3	= Clarinet */
	59,	/* Bariton1	= Tuba */
	68,	/* Bariton2	= Baritone Sax */
	33,	/* Bass1	= Wood Bass */
	34,	/* Bass2	= Electric Bass Fingered */
	1,	/* Piano1	= Accoustic Grand Piano */
	28,	/* Piano2	= Electric Clean Guitar */
	kFirstDrumkit+1	/* Drums & Percussion	= Standard Kit */
};
/*unsigned char	*timbreNames[] = {
#ifdef	JAPANESE_VERSION
	"\pアコースティックバスドラム", "\pバスドラム１", "\pサイドスティック",
	"\pアコースティックスネア", "\p手拍子", "\pエレクトリックスネア",
	"\pローフロアタム", "\pクローズドハイハット",
	"\pハイフロアタム", "\pペダルハイハット",
	"\pロータム", "\pオープンハイハット", "\pローミッドタム", "\pハイミッドタム",
	"\pクラッシュシンバル1", "\pハイタム", "\pライドシンバル1", "\pチャイニーズシンバル",
	"\pライドベル", "\pタンバリン", "\pスプラッシュシンバル", "\pカウベル",
	"\pクラッシュシンバル2", "\pビブラスラップ", "\pライドシンバル2",
	"\pハイボンゴ", "\pローボンゴ", "\pミュートハイコンガ", "\pハイコンガ", "\pローコンガ",
	"\pハイティンベール", "\pローティンベール", "\pハイアゴゴ", "\pローアゴゴ", "\pカバサ",
	"\pマラカス", "\pショートホイッスル", "\pロングホイッスル",
	"\pショートギロ", "\pロングギロ", "\pクラベス",
	"\pハイウッドブロック", "\pローウッドブロック",
	"\pオープンクィーカ", "\pミュートクィーカ",
	"\pオープントライアングル", "\pミュートトライアングル"
#else
	"\pAcoustic Bass Drum", "\pBass Drum 1", "\pSide Stick", "\pAcoustic Snare",
	"\pHand Clap", "\pElectric Snare", "\pLow Floor Tom", "\pClosed Hi-Hat",
	"\pHigh Floor Tom", "\pPedal Hi-Hat", "\pLow Tom", "\pOpen Hi-Hat",
	"\pLow-Mid Tom", "\pHigh-Mid Tom", "\pCrash Cymbal 1", "\pHigh Tom",
	"\pRide Cymbal 1", "\pChinese Cymbal", "\pRide Bell", "\pTambourine",
	"\pSplash Cymbal", "\pCowbell", "\pCrash Cymbal 2", "\pVibraslap",
	"\pRide Cymbal 2", "\pHigh Bongo", "\pLow Bongo", "\pMute High Conga",
	"\pHigh Conga", "\pLow Conga", "\pHigh Timbale", "\pLow Timbale",
	"\pHigh Agogo", "\pLow Agogo", "\pCabasa", "\pMaracas",
	"\pShort Whistle", "\pLong Whistle", "\pShort Guillo", "\pLong Guillo",
	"\pClaves", "\pHigh Wood Block", "\pLow Wood Block",
	"\pOpen Cuica", "\pMute Cuica", "\pOpen Triangle", "\pMute Triangle"
#endif
};*/
enum {
	tbFingerSnap = FirstDsPitch,
	tbHighQ, tbSlap, tbScratchPush, tbScratchPull, tbSticks,
	tbSquareClick, tbMetronomeClick, tbMetronomeBell,
	tbAcousticBassDrum, tbBassDrum, tbSideStick, tbAcousticSnare,
	tbHandClap, tbElectricSnare, tbLoFloorTom, tbClosedHiHat,
	tbHiFloorTom, tbPedalHiHat, tbLoTomTom, tbOpenHiHat,
	tbLoMidTom, tbHiMidTom, tbCrashCymbal1, tbHiTomTom,
	tbRideCymbal1, tbChineseCymbal, tbRideBell, tbTambourine,
	tbSplashCymbal, tbCowbell, tbCrashCymbal2, tbVibraslap,
	tbRideCymbal2, tbHiBongo, tbLoBongo, tbMuteHiConga,
	tbHiConga, tbLoConga, tbHiTimbale, tbLoTimbale,
	tbHiAgogo, tbLoAgogo, tbCabasa, tbMaracas,
	tbShortWhistle, tbLongWhistle, tbShortGuillo, tbLongGuillo,
	tbClaves, tbHiWoodBlock, tbLoWoodBlock,
	tbMutedCuica, tbOpenCuica, tbMutedTriangle, tbOpenTriangle,
	tbShaker, tbJingleBell, tbBellTree, tbCastanets,
	tbMuteSurdo, tbOpenSurdo
};
unsigned char	CymbalFlags[] = {
	0x00, 0x04, 0xa9, 0x7f, 0x40, 0x78, 0x03, 0x00
};
unsigned char
	tlBassDrums[] =		{ 2, tbAcousticBassDrum, tbBassDrum },
	tlHiHatPedal[] =	{ 1, tbPedalHiHat },
	tlDsStick[] =		{ 18, tbLoFloorTom, tbHiFloorTom,
		tbLoTomTom, tbLoMidTom, tbHiMidTom, tbHiTomTom,
		tbSideStick, tbAcousticSnare, tbElectricSnare,
		tbClosedHiHat, tbOpenHiHat, tbRideBell,
		tbRideCymbal1, tbRideCymbal2, tbCrashCymbal1, tbCrashCymbal2,
		tbSplashCymbal, tbChineseCymbal },
	tlTomTom[] =		{ 6, tbLoFloorTom, tbHiFloorTom,
		tbLoTomTom, tbLoMidTom, tbHiMidTom, tbHiTomTom },
	tlSnare[] =			{ 3, tbSideStick, tbAcousticSnare, tbElectricSnare },
	tlHiHat[] =			{ 3, tbPedalHiHat, tbClosedHiHat, tbOpenHiHat },
	tlRideCymbal[] =	{ 3, tbRideBell, tbRideCymbal1, tbRideCymbal2 },
	tlCrashCymbal[] =	{ 4, tbCrashCymbal1, tbCrashCymbal2,
		tbSplashCymbal, tbChineseCymbal },
	tlConga[] =		{ 3, tbLoConga, tbHiConga, tbMuteHiConga },
	tlBongo[] =		{ 2, tbLoBongo, tbHiBongo },
	tlTimbale[] =	{ 2, tbLoTimbale, tbHiTimbale },
	tlSurdo[] =		{ 2, tbOpenSurdo, tbMuteSurdo },
	tlAgogo[] =		{ 2, tbLoAgogo, tbHiAgogo },
	tlWhistle[] =	{ 2, tbLongWhistle, tbShortWhistle },
	tlGuillo[] =	{ 2, tbLongGuillo, tbShortGuillo },
	tlWoodBlock[] =	{ 2, tbLoWoodBlock, tbHiWoodBlock },
	tlCuica[] =		{ 2, tbOpenCuica, tbMutedCuica },
	tlScratch[] =	{ 2, tbScratchPull, tbScratchPush },
	tlTriangle[] =	{ 2, tbOpenTriangle, tbMutedTriangle },
	tlMetronome[] =	{ 2, tbMetronomeClick, tbMetronomeBell },
	tlSticks[] =	{ 1, tbSticks },
	tlHandClap[] =	{ 1, tbHandClap },
//	tlFingerSnap[] =	{ 1, tbFingerSnap },
	tlCastanets[] =		{ 1, tbCastanets },
	tlTambourine[] =	{ 1, tbTambourine },
	tlJingleBell[] =	{ 1, tbJingleBell },
	tlBellTree[] =	{ 1, tbBellTree },
	tlCowbell[] =	{ 1, tbCowbell },
	tlVibraslap[] =	{ 1, tbVibraslap },
	tlCabasa[] =	{ 1, tbCabasa },
	tlShaker[] = 	{ 1, tbShaker },
	tlMaracas[] =	{ 1, tbMaracas },
	tlClaves[] =	{ 1, tbClaves },
	tlHighQ[] =		{ 1, tbHighQ },
	tlSlap[] =		{ 1, tbSlap },
	tlSqureClick[] =	{ 1, tbSquareClick };
unsigned char	*timbreList[] = {
	tlBassDrums, tlHiHatPedal, tlDsStick, tlTomTom, tlSnare, tlHiHat,
	tlRideCymbal, tlCrashCymbal, tlConga, tlBongo, tlTimbale, tlSurdo, tlAgogo,
	tlWhistle, tlGuillo, tlWoodBlock, tlCuica, tlScratch, tlTriangle,
	tlMetronome, tlSticks, tlHandClap, tlCastanets, tlTambourine,
	tlJingleBell, tlBellTree, tlCowbell, tlVibraslap, tlCabasa, tlShaker,
	tlMaracas, tlSlap, tlClaves, tlHighQ, tlSqureClick
};
unsigned char	*DsInstNames[] = {
#ifdef	JAPANESE_VERSION
	"\pバスドラム", "\pハイハットペダル", "\pドラムスティック", "\pタムタム",
	"\pスネア", "\pハイハット", "\pライドシンバル", "\pクラッシュシンバル",
	"\pコンガ", "\pボンゴ", "\pティンベール", "\pサード", "\pアゴゴ",
	"\pホイッスル", "\pギロ", "\pウッドブロック", "\pクィーカ", "\pスクラッチ",
	"\pトライアングル", "\pメトロノーム", "\pスティック", "\p手拍子",
	"\pカスタネット", "\pタンバリン", "\pジングルベル", "\pベルツリー", "\pカウベル",
	"\pビブラスラップ", "\pカバサ", "\pシェイカー", "\pマラカス", "\pスラップ",
	"\pクラベス", "\pハイＱ", "\p矩形クリック"
#else
	"\pBass Drums", "\pHi Hat Pedal", "\pDrums Stick", "\pTom Tom",
	"\pSnare", "\pHi Hat", "\pRide Cymbal", "\pCrash Cymbal",
	"\pConga", "\pBongo", "\pTimbale", "\pSurdo", "\pAgogo",
	"\pWhistle", "\pGuillo", "\pWood Block", "\pCuica", "\pScratch",
	"\pTriangle", "\pMetronome", "\pSticks", "\pHand Clap",
	"\pCastanets", "\pTambourine", "\pJingle Bell", "\pBell Tree", "\pCowbell",
	"\pVibraslap", "\pCabasa", "\pShaker", "\pMaracas", "\pSlap",
	"\pClaves", "\pHigh Q", "\pSquare Click"
#endif
};
short	defaultDrumsInst[] = {
	diBassDrums, diHiHatPedal, diDsStick, diSnare,
	diConga, diBongo, diTimbale, diAgogo
};
Boolean cymbalP(short part, short note, BarInfo *pi) {
	unsigned char	*ls = timbreList[pi->drumsInst[part - DsPercPart] & 0xff];
	note = ls[((note & 0x7f) % ls[0]) + 1] - FirstDsPitch;
	return ((CymbalFlags[note / 8] & (0x80 >> (note % 8))) != 0);
} 
short cntlID[] = {
/*	kControllerModulationWheel, */
/*	kControllerBreath, */
/*	kControllerFoot, */
/*	kControllerPortamentoTime, */
/*	kControllerBalance, */
	kControllerPan,
	kControllerVolume,
/*	kControllerExpression, */
/*	kControllerLever1, */
/*	kControllerLever2, */
/*	kControllerLever3, */
/*	kControllerLever4, */
/*	kControllerLever5, */
/*	kControllerLever6, */
/*	kControllerLever7, */
/*	kControllerLever8, */
/*	kControllerPitchBend, */
/*	kControllerAfterTouch, */
/*	kControllerSustain, */
/*	kControllerPortamento, */
/*	kControllerSostenuto, */
/*	kControllerSoftPedal, */
	kControllerReverb,
/*	kControllerTremolo, */
	kControllerChorus,
	kControllerCeleste,
/*	kControllerPhaser */
/*	,kControllerEditPart */
/*	,kControllerMasterTune */
};
/* BasePitch = 9 ... A1 */
#define	BasePitch	9
#define	NNotes		7
short my_note(short part, short x, BarInfo *pi) {
	static short scale[KeyMax+1][NNotes] = {	/* Minor scale */
		{-1, 1, 2, 4, 6, 7, 9},		/* Cb/Abm */
		{-1, 1, 2, 4, 6, 8, 9}, 	/* Gb/Ebm */
		{-1, 1, 3, 4, 6, 8, 9}, 	/* Db/Bbm */
		{-1, 1, 3, 4, 6, 8, 10},	/* Ab/Fm */
		{-1, 1, 3, 5, 6, 8, 10},	/* Eb/Cm */
		{0, 1, 3, 5, 6, 8, 10},		/* Bb/Gm */
		{0, 1, 3, 5, 7, 8, 10},		/* F/Dm */
		{0, 2, 3, 5, 7, 8, 10},		/* C/Am */
		{0, 2, 3, 5, 7, 9, 10},		/* G/Em */
		{0, 2, 4, 5, 7, 9, 10},		/* D/Bm */
		{0, 2, 4, 5, 7, 9, 11},		/* A/F#m */
		{0, 2, 4, 6, 7, 9, 11},		/* E/C#m */
		{1, 2, 4, 6, 7, 9, 11},		/* B/G#m */
		{1, 2, 4, 6, 8, 9, 11},		/* F#/D#m */
		{1, 3, 4, 6, 8, 9, 11}		/* C#/A#m */
	};
	short	nnotes, octave, note, pitch;
	nnotes = (pi->keyScale <= KeyMax)? NNotes : 11;
	octave = x / nnotes; note = x % nnotes;
	if (x < 0) { octave --; note += nnotes; }
	pitch = (pi->keyScale <= KeyMax)? scale[pi->keyScale][note] : note;
	return (pi->octave[part] + octave) * 12
		+ pitch + BasePitch;
}
static pascal void TunePlayCB(unsigned long *event, long seed, long) {
	short	i, n;
	if (seed < 0) return;
	if (qtma_EventType(event[0]) != kGeneralEventType) return;
	n = qtma_GeneralLength(event[0], 0);
	if (n != qtma_GeneralLength(event[n-1], 0)) return;
	for (i = 1; i < n-1; i ++) {
		short	part = event[i] >> 16;
		short	gm = event[i] & 0xffff;
		NASetInstrumentNumberInterruptSafe(NoteAlct, NoteCh[part - 1], gm);
	}
}
static OSStatus alloc_channel(short part) {
	static	NoteRequest nr = {
		{ 0, 0, 2, 0x00010000 },
		{ kAnyComponentType, "\pAny Synthesizer", "\p", 1, 1 }
	};
//	nr.info.midiChannelAssignment = kNoteRequestSpecifyMIDIChannel |
//		((part < 9)? part + 1 : ((part == DsPercPart)? 10 : part + 2));
	nr.info.polyphony = (part < ChordPart)? 1 : ((part < DsPercPart)? 3 : 8);
//	nr.info.typicalPolyphony = 0x00010000;
	nr.tone.instrumentNumber = nr.tone.gmNumber = defaultInstrument[part];
	return NANewNoteChannel(NoteAlct, &nr, &NoteCh[part]);
}
Boolean init_tp(void) {
	static TunePlayCallBackUPP	myTunePlayCB = NULL;
	ComponentResult thisError;
	short	i, k;
	extern	Boolean	playDefault[];
	if (tp) return true;
	NoteAlct = OpenDefaultComponent(kNoteAllocatorComponentType, 0);
	if (!NoteAlct)
		{ res_msg(1, NULL, 0, 0); return false; };
	if ((thisError = alloc_channel(DsPercPart)))
		error_msg("\pNote channel allocation failed on drums & percussion part.", thisError);
	for (i = k = 0; i < DsPercPart; i ++) if (playDefault[i])
		if (alloc_channel(i) == noErr) k ++;
	for (i = 0; i < DsPercPart; i ++) if (!playDefault[i])
		if (alloc_channel(i) == noErr) k ++;
	if (k < DsPercPart)
		error_msg("\pThere is no enough number of channels allocated.", k);
	tp = OpenDefaultComponent(kTunePlayerComponentType, 0);
	if (!tp) { error_msg("\pCould not open TunePlayerComponent.", 0); goto error; }
	if (!myTunePlayCB)
		myTunePlayCB = NewTunePlayCallBackUPP(&TunePlayCB);
	thisError = TuneSetNoteChannels(tp, NNoteCh, NoteCh, myTunePlayCB, 0);
	if (thisError) { error_msg("\pTuneSetNoteChannels", thisError); goto error; }
	thisError = TuneSetTimeScale(tp, 600);
	if (thisError) { error_msg("\pTuneSetTimeScale", thisError); goto error; }
	thisError = TunePreroll(tp);
	if (thisError) { error_msg("\pTunePreroll", thisError); goto error; }
	thisError = TuneSetVolume(tp, 0x10000);
	if (thisError) { error_msg("\pTuneSetVolume", thisError); goto error; }
	return true;
error:
	if (NoteAlct) { CloseComponent(NoteAlct); NoteAlct = 0; }
	if (tp) { CloseComponent(tp); tp = 0; }
	return false;
}
static	short	tuneIndex = 0;
typedef	struct {
	unsigned long	tune[TuneSize];
	short	disp[TuneSize];
}	tuneData;
static	tuneData	Tune1, Tune2, *Tune = &Tune1;

static void stuff_note_event(short part, short pitch, short vel, long duration) {
	if (32 <= pitch && pitch <= 95 && duration < 0x800) {
		qtma_StuffNoteEvent(Tune->tune[tuneIndex], part+1, pitch, vel, duration);
		tuneIndex ++;
	} else { 
		qtma_StuffXNoteEvent(Tune->tune[tuneIndex], Tune->tune[tuneIndex+1],
			part+1, pitch, vel, duration);
		tuneIndex += 2;
	}
}
#ifdef	EFFECT
static short check_effect(Score *sc, short part) {
	short	delta;
	switch (sc[part].effect) {
		case EfctDown: delta = 1; break;
		case EfctUp: delta = -1; break;
		default: delta = 0;
	}
	if (delta) {
		qtma_StuffControlEvent(Tune->tune[tuneIndex], part+1, kControllerPitchBend,
			PitchBendWidth * delta);
		tuneIndex ++;
	}
	return delta;
}
#endif
static long qtma_cntl_value(short cntl, short v) {
	return (cntlID[cntl] == kControllerPan)?
			(v & 0x7f) * 2 + 256 : (v & 0x7f) << 8;
}
static void stuff_play_info(BarInfo *pi) {
	short	i, j, n;
	Boolean	playOn[NNoteCh];
	BlockMove(pi->playOn, playOn, sizeof(playOn));
	if (!playOn[DsPercPart]) for (i = DsPercPart+1; i < NTotalParts; i ++)
		if (pi->playOn[i]) { playOn[DsPercPart] = true; break; }
	for (i = n = 0; i < NNoteCh; i ++) if (playOn[i]) n ++;
	if (n == 0) return;
	qtma_StuffGeneralEvent(Tune->tune[tuneIndex], Tune->tune[tuneIndex+n+1],
		63, kGeneralEventNoOp, n+2);
	for (i = j = 0; i < NNoteCh && j < n; i ++) if (playOn[i]) {
		Tune->tune[tuneIndex+1+j] = ((i + 1) << 16) | pi->instrument[i];
		j ++;
	}
	tuneIndex += n + 2;
	for (i = 0; i < NNoteCh; i ++) if (playOn[i]) {
		for (j = 0; j < NControls; j ++) {
			qtma_StuffControlEvent(Tune->tune[tuneIndex], i+1, (long)cntlID[j],
				qtma_cntl_value(j, pi->control[i][j]));
			tuneIndex ++;
		}
	}
}
#define	MyNote(part,delta)	my_note(part,sc[part].note+(delta), pi)
short get_timbre_id(short part, short note, BarInfo *pi) {
	long	Inst = pi->drumsInst[part - DsPercPart], mask;
	unsigned char	*kit = timbreList[Inst & 0xff];
	short	i, id = note % kit[0];
	for (i = 0, mask = 0x100 << id; i < kit[0]; id ++, mask <<= 1) {
		if (id >= kit[0]) { id = 0; mask = 0x100; }
		if (!(Inst & mask)) break;
	}
	return (i < kit[0])? id : 0;
}
short get_timbre(short part, short note, BarInfo *pi) {
	return timbreList[pi->drumsInst[part - DsPercPart] & 0xff]
		[get_timbre_id(part, note, pi) + 1];
}
static void set_tune(Score *sc, BarInfo *pi, short *len, short min) {
	long	duration;
	short	i;
	for (i = 0; i < ChordPart; i ++) if (sc[i].note >= 0)
		stuff_note_event(i, MyNote(i, 0),
			sc[i].velocity + 64, len[i] * (9000 / pi->tempo));
	for (; i < DsPercPart; i ++) if (sc[i].note >= 0) {
		short	vel = sc[i].velocity * 1.5 + 32;
		duration = len[i] * (9000 / pi->tempo);
		stuff_note_event(i, MyNote(i, 2), vel, duration);
		stuff_note_event(i, MyNote(i, 0), vel, duration);
		stuff_note_event(i, MyNote(i, -2), vel, duration);
	}
	for (; i < NTotalParts; i ++) if (sc[i].note >= 0)
		stuff_note_event(DsPercPart, get_timbre(i, sc[i].note, pi),
			sc[i].velocity + 64, len[i] * (9000 / pi->tempo));
	if ((duration = min * (9000 / pi->tempo)) > 0) {
		qtma_StuffRestEvent(Tune->tune[tuneIndex], duration);
		tuneIndex++;
	}
}
static void set_next_id(WindowPtr win, short id) {
	switch (GetWRefCon(win)) {
		case WIDField: {
			SBField	*sf = FindField(win);
			if (!sf) return;
			if (sf->PlayAll && (PlayLoop || id < PopulationSize - 1)) {
				NextWin = win; NextID = (id + 1) % PopulationSize;
			} else { NextWin = NULL; NextID = -1; }
		} break;
		case WIDIntegrator: {
			short	k, nsecs;
			SBIntegrator	*si = FindIntegrator(win);
			if (!si) return;
			nsecs = si->nSectionsH * si->nSectionsW;
			for (k = 0; k < nsecs; k ++) {
				if ((++ id) >= nsecs)
					{ if (PlayLoop) id = 0; else break; }
				if (ItgOnP(*si->scoreHandle,id))
					{ NextWin = win; NextID = id; return; }
			}
			NextWin = NULL; NextID = -1;
		} break;
		case WIDgedit: NextWin = NULL; NextID = -1;
	}
}
#define	THNWords	3
static void stuff_tune_data(WindowPtr win, short id) {
	short	i, j, len, n, tempo;
	long	rest;
	BarInfo	*pi;
	tuneIndex = THNWords;
	switch (GetWRefCon(win)) {
		case WIDField: {
			SBField *sf = FindField(win); if (!sf) return;
			tempo = sf->b.tempo;
			stuff_play_info(pi = &sf->b);
			proc_score(sf->pop[id].score, pi, set_tune);
		} break;
		case WIDIntegrator: {
			short	nsecs;
			SBIntegrator *si = FindIntegrator(win); if (!si) return;
			nsecs = si->nSectionsH * si->nSectionsW;
			if (id >= nsecs) id = 0;
			HLock((Handle)si->scoreHandle);
			tempo = (*si->scoreHandle)[id].b.tempo;
			stuff_play_info(pi = &(*si->scoreHandle)[id].b);
			proc_score((*si->scoreHandle)[id].i.score, pi, set_tune);
			HUnlock((Handle)si->scoreHandle);
		} break;
		case WIDgedit: {
			SBgedit	*sg = FindGedit(win); if (!sg) return;
			tempo = sg->b.tempo;
			stuff_play_info(pi = &sg->b);
			proc_score(sg->ind.score, pi, set_tune);
		} break;
	}
	qtma_StuffMarkerEvent(Tune->tune[tuneIndex], kMarkerEventEnd, 0); tuneIndex ++;
	rest = 0;
	for (n = 0, i = THNWords; i < tuneIndex; i ++) {
		switch (qtma_EventType(Tune->tune[i])) {
			case kRestEventType:
			rest += qtma_RestDuration(Tune->tune[i]);
			if (rest >= (9000 / tempo)) {
				n += rest / (9000 / tempo);
				rest = 0;
			}
			break;
			case kXNoteEventType:
			Tune->disp[i-THNWords] = n; i ++;
			break;
			case kGeneralEventType:
			len = qtma_GeneralLength(Tune->tune[i],0) - 1;
			for (j = 0; j < len; j ++, i++) Tune->disp[i-THNWords] = n;
		}
		Tune->disp[i-THNWords] = n;
	}
	Tune->disp[tuneIndex-THNWords] = 0;
}
static short get_tempo_from_win(WindowPtr win, short id) {
	SBField	*sf; SBIntegrator *si; SBgedit *sg;
	integScorePtr	scp;
	switch (GetWRefCon(win)) {
		case WIDField: sf = FindField(win);
		return sf? sf->b.tempo : 100;
		case WIDgedit: sg = FindGedit(win);
		return sg? sg->b.tempo : 100;
		case WIDIntegrator: si = FindIntegrator(win); if (!si) return 100;
		scp = *si->scoreHandle; if (!ItgOnP(scp,id)) return 100;
		return scp[id].b.tempo;
		default: return 100;
	}
}
static void enque_tune(WindowPtr win, short id) {
	short	i, j;
	ComponentResult thisError;
	if (!init_tp()) return;
	/* error_msg("\pTest", id); */
	if (Playing == PlayAndQueued) return;
	Tune->tune[0] = (get_tempo_from_win(win, id) << 8) | id;
	Tune->tune[1] = (unsigned long)(Tune->disp);
	Tune->tune[2] = (unsigned long)win;
	stuff_tune_data(win, id);
	if (Playing == PlayStopped) {
		extern	BarInfo	*CurrentBI;
		Playing = PlayOne; PlayWindow = win;
		if (CurrentBI) switch (GetWRefCon(win)) {
			case WIDField: case WIDgedit:
			for (i = 0; i < NNoteCh; i ++) for (j = 0; j < NControls; j ++)
				set_qtma_control(j, i, CurrentBI->control[i][j]);
		}
	} else Playing = PlayAndQueued;
	set_next_id(win, id);
	thisError = TuneQueue(tp, Tune->tune+THNWords, 0x00010000, 0, 0x7fffffff,
		(PlayLoop? kTuneLoopUntil : 0), NULL, 0);
	Tune = (Tune == &Tune1)? &Tune2 : &Tune1;
}
void play_sequence(WindowPtr win, short id) {
	tuneData	*org;
	if (GetWRefCon(win) == WIDIntegrator) {
		short	i, nsecs;
		SBIntegrator	*si = FindIntegrator(win); if (!si) return;
		nsecs = si->nSectionsH * si->nSectionsW;
		if (id >= nsecs) id = 0;
		for (i = 0; i < nsecs; i ++, id = (id + 1) % nsecs)
			if (ItgOnP(*si->scoreHandle,id)) break;
		if (i >= nsecs) return;
	}
	switch (Playing) {
		case PlayStopped: enque_tune(win, id); break;
		case PlayOne: NextWin = win; NextID = id; break;
		case PlayAndQueued: org = Tune;
		Tune = (Tune == &Tune1)? &Tune2 : &Tune1;
		Tune->tune[0] = (get_tempo_from_win(win, id) << 8) | id;
		Tune->tune[2] = (unsigned long)win;
		stuff_tune_data(win, id);
		Tune = org;
		set_next_id(win, id);
	}
}
void play_selected_score(WindowPtr win) {
	if (win == PlayWindow) return;
	switch (GetWRefCon(win)) {
		case WIDField: {
			SBField *sf = FindField(win);
			if (!sf) return;
			if (sf->PlayID >= 0) play_sequence(win, sf->PlayID);
		} break;
		case WIDIntegrator: {
			SBIntegrator	*si = FindIntegrator(win);
			if (!si) return;
			if (si->PlayID >= 0) play_sequence(win, si->PlayID);
		} break;
		case WIDgedit: {
			SBgedit	*sg = FindGedit(win);
			if (sg &&sg->playing) play_sequence(win, 0);
		}
	}
}
void stop_tune(void) {
	if (!tp || Playing == PlayStopped) return;
	TuneStop(tp, 0);
	if (playID >= 0) inverse_playing_mark();
	Playing = PlayStopped;
	playID = NextID = -1;
	play2 = 0;
	PlayWindow = NULL;
}
static void stuff_stop_tune(unsigned long *tune) {
	memset(tune, 0, sizeof(unsigned long) * THNWords);
	qtma_StuffRestEvent(tune[THNWords], 50);
	qtma_StuffMarkerEvent(tune[THNWords+1], kMarkerEventEnd, 0);
}
void change_play_loop(void) {
	enum { SingleBar, MiddleBar, FinalBar } s;
	static	unsigned long	dummy_tune[THNWords+4];
	if (Playing == PlayStopped || !PlayWindow) return;
	switch (GetWRefCon(PlayWindow)) {
		case WIDField: if (!PlayAll) s = SingleBar;
		else if (playID == PopulationSize - 1) s = FinalBar;
		else s = MiddleBar;
		break;
		case WIDIntegrator: if (NextID < 0) s = FinalBar;
		else s = MiddleBar;
		break;
		case WIDgedit: s = SingleBar; break;
		default: return;
	}
	if (PlayLoop) switch (s) {
		case SingleBar: enque_tune(PlayWindow, playID); break;
		case FinalBar: set_next_id(PlayWindow, playID);
	} else switch (s) {
		case SingleBar:
		stuff_stop_tune(dummy_tune);
		TuneQueue(tp, dummy_tune + THNWords, 0x00010000, 0, 0x7fffffff, 0, NULL, 0);
		break;
		case FinalBar: if (Playing == PlayAndQueued)
			stuff_stop_tune((Tune == &Tune1)? Tune2.tune : Tune1.tune);
	}
}
void play_restart(void) {
	if (PlayWindow && playID >= 0) {
		if (Playing == PlayAndQueued) {
			tuneData	*org = Tune;
			Tune = (Tune == &Tune1)? &Tune2 : &Tune1;
			stuff_tune_data((WindowPtr)(Tune->tune[2]), Tune->tune[0] & 0xff);
			Tune = org;
		} else if (!PlayAll && NextID < 0) enque_tune(PlayWindow, playID);
	}
}
void revise_tune_queue(WindowPtr win, short id) {
	if (PlayTypeFieldP(win) && !PlayAll) {
		tuneData	*tn = (Tune == &Tune1)? &Tune2 : &Tune1;
		if ((tn->tune[0] & 0xff) == id && tn->tune[2] == (unsigned long)win)
			play_restart();
	} else if (Playing == PlayAndQueued) {
		tuneData	*org, *tn = (Tune == &Tune1)? &Tune2 : &Tune1;
		if ((tn->tune[0] & 0xff) == id && tn->tune[2] == (unsigned long)win) {
			org = Tune; Tune = tn;
			stuff_tune_data(win, id);
			Tune = org;
		}
	}
}
void change_instrument(short part, BarInfo *pi) {
	if (NoteAlct) NASetInstrumentNumberInterruptSafe(
		NoteAlct, NoteCh[part], pi->instrument[part]);
}
void set_gm_to_inst(long gm, ControlHandle button) {
	ToneDescription	td;
	if (!NoteAlct) if (!init_tp()) ExitToShell();
	NAStuffToneDescription(NoteAlct, gm, &td);
	SetControlTitle(button, td.instrumentName);
//	DrawOneControl(button);
}
enum {	// Dialog item IDs of Drums & Percussion Timbre Picker
	DPTPOkButton = 1,
	DPTPCancelButton,
	DPTPInstTextGroupBox,
	DPTPInstRadioGroupBox,
	DPTPInstRadioButton
};
static pascal Boolean my_NApickInst_filter(
	DialogRef dlg, EventRecord *e, short *itemHit) {
	if (StdFilterProc(dlg, e, itemHit)) return true;
	switch (e->what) {
		case nullEvent: check_playing(); check_integ_sel();
		*itemHit = 0; break;
		case updateEvt:
		if ((WindowPtr)e->message != GetDialogWindow(dlg)) DoUpdate(e);
		*itemHit = 0; break;
		case activateEvt: if (e->modifiers & activeFlag) {
			SetDialogDefaultItem(dlg, DPTPOkButton);
			SetDialogCancelItem(dlg, DPTPCancelButton);
		}
		*itemHit = 0;
		return true;
	}
#ifdef	DEBUG
	event_monitor("ModalFilter",e);
#endif
	return false;
}
#ifdef	JAPANESE_VERSION
#define	pickInstPrompt	"「%s」に割り当てる音色を選択して下さい。"
#else
#define	pickInstPrompt	"Select a new timbre for %s."
#endif
static ModalFilterUPP	MyModalFilter = NULL;
long new_instrument(char *partID, long oldGM, ControlRef button) {
	ToneDescription	td;
	OSErr	err;
	unsigned char	prompt[64];
	if (!NoteAlct) if (!init_tp()) return -1;
	memset(&td, 0, sizeof(td));
	td.instrumentNumber = td.gmNumber = oldGM;
	if (!MyModalFilter) MyModalFilter = NewModalFilterUPP(&my_NApickInst_filter);
	sprintf((char *)prompt + 1, pickInstPrompt, partID);
	prompt[0] = strlen((char *)prompt + 1);
	err = NAPickInstrument(NoteAlct, MyModalFilter, prompt,
		&td, (kPickDontMix|kPickSameSynth), 0, 0, 0);
	if (err != noErr) return -1;
	SetControlTitle(button, td.instrumentName);
	return td.instrumentNumber;
}
static void append_timbre_selector(DialogRef dlg, long Inst,
	short itemCount, Point dlgSize) {
	short	inst = Inst & 0xff;
	short	cnitem = CountDITL(dlg);
	if (cnitem > itemCount) {
		ShortenDITL(dlg, cnitem - itemCount);
		SizeWindow(GetDialogWindow(dlg), dlgSize.h, dlgSize.v, false);
	}
	if (timbreList[inst][0] <= 1) {
		AppendDialogItemList(dlg, 201, overlayDITL);
	} else {
		short	i;
		long	mask;
		ControlRef	cbox;
		AppendDialogItemList(dlg, 200 + inst, appendDITLBottom);
		cnitem = timbreList[inst][0] + itemCount;
		for (i = itemCount + 1, mask = 0x100; i <= cnitem; i ++, mask <<= 1) {
			GetDialogItemAsControl(dlg, i, &cbox);
			SetControlValue(cbox, (Inst & mask)?
				kControlCheckBoxUncheckedValue : kControlCheckBoxCheckedValue);
		}
	}
}
static void play_drums_timbre(short pitch) {
	unsigned long	t;
	NAPlayNote(NoteAlct, NoteCh[DsPercPart], pitch, 100);
	Delay(6, &t); // delay 0.1 second
	NAPlayNote(NoteAlct, NoteCh[DsPercPart], pitch, 0);
}
void set_ds_inst_name(ControlRef button, long inst) {
	unsigned char	*np, title[64];
	np = DsInstNames[inst & 0xff];
	BlockMove(np, title, np[0] + 1);
	if (inst & 0xffffff00) {
		BlockMove(" -", &title[np[0] + 1], 2);
		title[0] += 2;
	}
	SetControlTitle(button, title);
}
long new_drums_instrument(char *partID, long oldInst, ControlRef button) {
	long	newInst = oldInst, k;
	short	i, itemCount, itemHit;
	OSErr	err;
	Rect	bound;
	Point	dlgSize;
	ControlRef	cbox, radioBtnGroup;
	WindowRef	win;
	GrafPtr	oldPort;
	unsigned char	prompt[64];
	DialogRef	dlg;
	if (!NoteAlct) if (!init_tp()) return -1;
	sprintf((char *)prompt + 1, pickInstPrompt, partID);
	prompt[0] = strlen((char *)prompt + 1);
	dlg = GetNewDialog(129, NULL, kMoveToFront);
	if (!dlg) {
		error_msg("\pGetNewDialog in new_drums_instrument", 129); return -1;
	}
	for (i = 0; DsInstNames[i]; i ++) {
		if (GetDialogItemAsControl(dlg, i + DPTPInstRadioButton, &cbox)) break;
		SetControlTitle(cbox, DsInstNames[i]);
	}
	itemCount = CountDITL(dlg);
	GetWindowPortBounds((win = GetDialogWindow(dlg)), &bound);
	dlgSize.h = bound.right;
	dlgSize.v = bound.bottom;
	SetWTitle(win, prompt);
	err = GetDialogItemAsControl(dlg, DPTPInstRadioGroupBox, &radioBtnGroup);
	if (err != noErr) return -1;
	SetControlValue(radioBtnGroup, (oldInst & 0xff) + 1);
	append_timbre_selector(dlg, oldInst, itemCount, dlgSize);
	if (!MyModalFilter) MyModalFilter = NewModalFilterUPP(&my_NApickInst_filter);
	GetPort(&oldPort); //SetPort(GetWindowPort(win));
	while (1) {
		ModalDialog(MyModalFilter, &itemHit);
		if (itemHit == DPTPOkButton) break;
		if (itemHit == DPTPCancelButton) { newInst = -1; break; }
		if (itemHit == DPTPInstRadioGroupBox) {
			k = GetControlValue(radioBtnGroup) - 1;
			if (k != (newInst & 0xff)) {
				if (timbreList[newInst & 0xff][0] > 1 || timbreList[k][0] > 1)
					append_timbre_selector(dlg, k, itemCount, dlgSize);
				newInst = k;
			}
		} else if (itemHit > itemCount) {
			short	itemID = itemHit - itemCount;
			unsigned char	*tlist = timbreList[newInst & 0xff];
			if (tlist[0] <= 1) {
				if (itemID == 1) play_drums_timbre(tlist[1]);
			} else if (itemID <= tlist[0]) {
				long	mask = 0x100L << (itemID - 1);
				short	value;
				GetDialogItemAsControl(dlg, itemHit, &cbox);
				value = GetControlValue(cbox);
				SetControlValue(cbox, (value == kControlCheckBoxUncheckedValue)?
					kControlCheckBoxCheckedValue : kControlCheckBoxUncheckedValue);
				newInst = (value == kControlCheckBoxUncheckedValue)?
					(newInst & ~mask) : (newInst | mask);
			} else if (itemID <= tlist[0] * 2)
				play_drums_timbre(tlist[itemID - tlist[0]]);
		}
	};
	SetPort(oldPort);
	DisposeDialog(dlg);
	if (newInst >= 0 &&
		(((oldInst ^ newInst) & 0xff) ||
		 ((oldInst & 0xffffff00) != 0) != ((newInst & 0xffffff00) != 0)))
		set_ds_inst_name(button, newInst);
	return newInst;
}
static short default_cntl_value(short cntl, short part) {
	static unsigned char	soloPan[] =
		{ 32, 56, 96, 40, 64, 104, 48, 72, 112, 80, 120, 24, 88, 8, 16 };
	switch (cntlID[cntl]) {
		case kControllerVolume: case kControllerExpression:
		return (ChordPart <= part && part < DsPercPart)? 80 : 100;
		case kControllerPan:
		return (part < DsPercPart)? soloPan[part] : 64;
		case kControllerReverb: return 40;
		default: return 0;
	}
}
void init_part_control(BarInfo *pi) {
	short	i, p;
	for (p = 0; p < NNoteCh; p ++)
	for (i = 0; i < NControls; i ++) {
		pi->control[p][i] = default_cntl_value(i, p);
		set_qtma_control(i, p, pi->control[p][i]);
	}
}
void set_qtma_control(short cntl, short part, short v) {
	if (!NoteAlct || 0 > cntl || cntl >= NControls) return;
	NASetController(NoteAlct, NoteCh[part],
		(long)cntlID[cntl], qtma_cntl_value(cntl, v));
}
void inject_controls(BarInfo *p) {
	short	i, j;
	if (!NoteAlct) return;
	for (i = 0; i < NNoteCh; i ++) {
		NASetInstrumentNumberInterruptSafe(NoteAlct, NoteCh[i], p->instrument[i]);
		for (j = 0; j < NControls; j ++)
			NASetController(NoteAlct, NoteCh[i],
				(long)cntlID[j], qtma_cntl_value(j, p->control[i][j]));
	}
}
void play_one_channel(short part, Fixed volume) {
	short	i;
	if (!NoteAlct) return;
	if (part >= NNoteCh) part = NNoteCh - 1;
	for (i = 0; i < NNoteCh; i ++) if (i != part)
		NASetNoteChannelVolume(NoteAlct, NoteCh[i], volume);
}
//#define	MONITOR
#ifdef	MONITOR
static void monitorx(TuneStatus *st) {
	static WindowPtr	win = NULL;
	unsigned char	line[256];
	GrafPtr	oldport;
	GetPort(&oldport);
	if (!win) {
		short	fid;
		win = GetNewCWindow(512, NULL, (WindowPtr)-1);
		if (!win) return;
		GetFNum("\pCourier", &fid);
		SetPort(win); TextFont(fid);
	} else SetPort(win);
	EraseRect(&win->portRect);
	sprintf((char *)line+1, "      tune: 0x%08X", st->tune);
	line[0] = strlen((char *)line+1); MoveTo(10,20); DrawString(line);
	sprintf((char *)line+1, "   tunePtr: 0x%08X", st->tunePtr);
	line[0] = strlen((char *)line+1); MoveTo(10,40); DrawString(line);
	sprintf((char *)line+1, "      time: %d", st->time);
	line[0] = strlen((char *)line+1); MoveTo(10,60); DrawString(line);
	sprintf((char *)line+1, "queueCount: %d", st->queueCount);
	line[0] = strlen((char *)line+1); MoveTo(10,80); DrawString(line);
	sprintf((char *)line+1, "queueSpots: %d", st->queueSpots);
	line[0] = strlen((char *)line+1); MoveTo(10,100); DrawString(line);
	sprintf((char *)line+1, " queueTime: %d", st->queueTime);
	line[0] = strlen((char *)line+1); MoveTo(10,120); DrawString(line);
	sprintf((char *)line+1, "Playing Mode = %d", Playing);
	line[0] = strlen((char *)line+1); MoveTo(10,140); DrawString(line);
	sprintf((char *)line+1, "     Tune = 0x%08X", Tune->tune+THNWords);
	line[0] = strlen((char *)line+1); MoveTo(10,160); DrawString(line);
	sprintf((char *)line+1, "NextWin=0x%X, NextID=%d", NextWin, NextID);
	line[0] = strlen((char *)line+1); MoveTo(10,180); DrawString(line);
	
	SetPort(oldport);
}
#endif
void check_playing(void) {
	short	id, n, tempo;
	TuneStatus	status;
	WindowPtr	win;
	if (NoteAlct) NATask(NoteAlct);
	if (!tp || Playing == PlayStopped) {
		PlayWindow = NULL; playID = -1;
#ifdef	MONITOR
		if (tp) { TuneGetStatus(tp, &status); monitorx(&status); }
#endif
		return;
	}
	TuneGetStatus(tp, &status);
#ifdef	MONITOR
	monitorx(&status);
#endif
	if (status.queueCount == 0 || !status.tune[-(THNWords-2)]) {
		if (PlayWindow && playID >= 0) {
			SBField	*sf; SBIntegrator *si; SBgedit *sg;
			inverse_playing_mark();
			switch (GetWRefCon(PlayWindow)) {
				case WIDField: sf = FindField(PlayWindow);
				if (sf) { clear_speaker_icon(sf); sf->PlayID = -1; }
				break;
				case WIDIntegrator: si = FindIntegrator(PlayWindow);
				if (si) {
					DeactivateControl(si->button[CntlStop-1]);
					DeactivateControl(si->button[CntlPause-1]);
					ActivateControl(si->button[CntlPlay-1]);
					si->PlayID = -1;
				} break;
				case WIDgedit: sg = FindGedit(PlayWindow);
				if (sg) { switch_GEplay_button(true); sg->playing = false; }
			}
		}
		Playing = PlayStopped;
		PlayWindow = NULL; playID = -1;
		if (status.queueCount > 0) TuneStop(tp, 0);
		return;
	}
	id = status.tune[-THNWords] & 0xff;
	tempo = status.tune[-THNWords] >> 8;
	n = ((short *)(status.tune[-(THNWords-1)]))[status.tunePtr - status.tune];
	win = (WindowPtr)status.tune[-(THNWords-2)];
	if (PlayWindow != win || id != playID || n != play2) {
		if (playID >= 0) inverse_playing_mark();
		play1 = (play2 >= n)? 0 : play2;
		play2 = n;
		PlayWindow = win;
		playID = id;
		inverse_playing_mark();
		switch (GetWRefCon(win)) {
			case WIDField: {
				SBField	*sf = FindField(win);
				if (sf) sf->PlayID = id;
			} break;
			case WIDIntegrator: {
				SBIntegrator *si = FindIntegrator(win);
				if (si) { si->PlayID = id; check_integ_playing(si); }
			}
		}
	}
	if (Playing == PlayOne) {
		if (NextWin && NextID >= 0)
			enque_tune(NextWin, NextID);
	} else if (status.tune != Tune->tune+THNWords) Playing = PlayOne;
}
