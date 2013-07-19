#include  "decl.h"
#include	"pcolor.h"
extern	WindowPtr	TopWindow, PlayWindow;
extern	short	dispScaleM[], dispScaleD[];
RGBColor	partColor[] = {
		{Part1R,Part1G,Part1B}, {Part2R,Part2G,Part2B}, {Part3R,Part3G,Part3B},
		{Part4R,Part4G,Part4B}, {Part5R,Part5G,Part5B}, {Part6R,Part6G,Part6B},
		{Part7R,Part7G,Part7B}, {Part8R,Part8G,Part8B}, {Part9R,Part9G,Part9B},
		{PartAR,PartAG,PartAB}, {PartBR,PartBG,PartBB}, {PartCR,PartCG,PartCB},
		{PartDR,PartDG,PartDB}, {PartER,PartEG,PartEB}, {PartFR,PartFG,PartFB},
		{PartP1R,PartP1G,PartP1B}, {PartP2R,PartP2G,PartP2B}, {PartP3R,PartP3G,PartP3B},
		{PartP4R,PartP4G,PartP4B}, {PartP5R,PartP5G,PartP5B}, {PartP6R,PartP6G,PartP6B},
		{PartP7R,PartP7G,PartP7B}, {PartP8R,PartP8G,PartP8B},
		{0,0,0}};			/* black */
static	ViewInfo	*VInfo;
static	BarInfo	*PInfo, *PInfo2;
enum {
	bmGClef, bmFClef, bmBlackHead, bmWhiteHead,
	bmUp8Flag, bmUp16Flag, bmDown8Flag, bmDown16Flag,
	bmRest2, bmRest4, bmRest8, bmRest16,
	bmDrumHead, bmCymbalHead, bmOctava
};
static char g_bits[] = {
 0x01,0x00,0x03,0x80,0x03,0x80,0x06,0x40,0x06,0x40,0x04,0x40,0x04,0x40,0x04,
 0xc0,0x04,0xc0,0x05,0x80,0x07,0x80,0x07,0x00,0x0e,0x00,0x1e,0x00,0x3a,0x00,
 0x72,0x00,0x62,0xc0,0xe7,0xe0,0xc7,0xe0,0x8f,0x30,0x89,0x30,0x89,0x10,0xc9,
 0x10,0x49,0x10,0x46,0xa0,0x30,0xc0,0x0f,0x80,0x03,0x80,0x00,0x40,0x00,0x40,
 0x00,0x40,0x0e,0x40,0x1e,0x40,0x1c,0x80,0x0f,0x00},
	f_bits[] = {
 0x3f,0x00,0x63,0x80,0x81,0x98,0xf1,0xd8,0xf1,0xd8,0x61,0xc0,0x01,0xd8,0x01,
 0xd8,0x03,0x98,0x03,0x80,0x07,0x00,0x0e,0x00,0x18,0x00,0x30,0x00,0x40,0x00},
	nb_bits[] = {0x38,0x7c,0xfc,0xf8,0x70},
	nw_bits[] = {0x38,0x6c,0xcc,0xd8,0x70},
	u8_bits[] = {0x80,0x80,0xc0,0x40,0x20,0x30,0x10,0x10,0x10,0x10,0x10,0x20,0x20},
	uh_bits[] = {0x80,0xc0,0x60,0xa0,0x90,0xd0,0x70,0x10,0x18,0x08,0x08,0x08,0x08,0x10},
	d8_bits[] = {0x20,0x20,0x10,0x10,0x10,0x10,0x10,0x30,0x20,0x40,0xc0,0x80,0x80},
	dh_bits[] = {0x10,0x08,0x08,0x08,0x08,0x18,0x10,0x70,0xd0,0x90,0xa0,0x60,0xc0,0x80},
	r2_bits[] = {0xfc,0xfc,0xfc},
	r4_bits[] = {0x40,0x20,0x30,0x38,0x38,0x70,0x60,0x60,
		0x20,0x01,0xf0,0xe8,0xc0,0x40,0x20},
	r8_bits[] = {0xe0,0xe0,0xe8,0x18,0x10,0x10,0x10,0x30,0x20,0x20},
	rh_bits[] = {0x72,0x74,0x3c,0x04,0xe8,0xe8,0xf8,0x08,0x10,0x10,0x10,0x30,0x20,0x20},
	ds_bits[] = {0x1c,0x38,0x70,0xe0,0xc0},
	cym_bits[] = {0x88,0x50,0x20,0x50,0x88},
	va8_bits[] = {0x60,0x00,0x90,0x00,0x65,0x30,0x95,0x50,0x62,0x70};
static BitMap bMap[] = {
	{ g_bits, 2, {0,0,35,12}}, { f_bits, 2, {0,0,15,13}},
	{ nb_bits, 1, {0,0,5,6}}, { nw_bits, 1, {0,0,5,6}},
	{ u8_bits, 1, {0,0,13,4}}, { uh_bits, 1, {0,0,14,5}},
	{ d8_bits, 1, {0,0,13,4}}, { dh_bits, 1, {0,0,14,5}},
	{ r2_bits, 1, {0,0,3,6}}, { r4_bits, 1, {0,0,15,5}},
	{ r8_bits, 1, {0,0,10,5}}, { rh_bits, 1, {0,0,14,7}},
	{ ds_bits, 1, {0,0,5,6}}, { cym_bits, 1, {0,0,5,5}},
	{ va8_bits, 2, {0,0,5,12}},
};
static Point bMapOffset[] = {
	{ 27, 0 }, { 16, -1 }, { 2, 0 }, { 2, 0 },
	{ -2, -1 }, { -2, -1 }, { 15, -1 }, { 16, -1 },
	{ 2, 0 }, { 9, 0 }, { 3, 0 }, { 7, 0 },
	{ 2, 0 }, { 3, 0 },
	{ 0, 0 }
};
#define	TailLen	17
#define	DrumTailLen	14
static void draw_bm(short code) {
	CGrafPtr	dport;
	GDHandle	gdh;
	Rect	dr;
	Point	pt;
	GetGWorld(&dport, &gdh);
	GetPen(&pt);
	dr = bMap[code].bounds;
	OffsetRect(&dr, pt.h - bMapOffset[code].h, pt.v - bMapOffset[code].v); 
	CopyMask(&bMap[code], &bMap[code],
		GetPortBitMapForCopyBits(dport),
		&bMap[code].bounds, &bMap[code].bounds, &dr);
}
void draw_color_frame(short id, short fcol) {
	Rect	r;
	r.top = SubWinYUnit * (id / FieldNColumns) + WinTopSpace + IndControlH;
	r.left = SubWinXUnit * (id % FieldNColumns) + WinLeftSpace;
	r.bottom = r.top + SubWinPHeight + SubWinBorder*2;
	r.right = r.left + SubWinWidth + SubWinBorder*2;
	PenSize(SubWinBorder, SubWinBorder);
	ForeColor(fcol);
	FrameRect(&r);
}
void draw_frame(SBField *sf, short id) {
	short	fcol;
	switch (sf->sel[id] & (SelectedFlag|ProtectedFlag)) {
		case SelectedFlag: fcol = redColor; break;
		case ProtectedFlag: fcol = blueColor; break;
		case SelectedFlag|ProtectedFlag: fcol = magentaColor; break;
		default: fcol = blackColor;
	}
	draw_color_frame(id, fcol);
}
Point	noteP;
static void draw_dash(short h, short x) {
	ForeColor(blackColor);
	MoveTo(x - 2, noteP.v + ScoreLinePad * h);
	Line(NoteWUnit - 2, 0);
}
#define	noteWidth	5
#define	noteHeight	6
static void draw_tie(short x1, short x2, short y, Boolean up) {
	Rect	r;
	if (up) {
		SetRect(&r,x1+noteWidth,y,x2,y+noteHeight);
		FrameArc(&r, 90, 180);
	} else {
		SetRect(&r,x1+noteWidth,y-noteHeight,x2,y);
		FrameArc(&r, 270, 180);
	}
}
static void plot_drum_note(BarInfo *pi, short part, short nt, short len) {
	short	x1 = noteP.h + 2, y, l, nn = nt;
	RGBForeColor(&partColor[part]);
	if (nn >= 9) nn -= 9;
	y = noteP.v + (14 - nn) * ScoreLinePad / 2;
	MoveTo(x1, y);
	draw_bm(cymbalP(part, nt, pi)? bmCymbalHead : bmDrumHead);
	if (len == 3) {
		Move(8, ((nn & 1)? 0 : -2));
		Line(1,0); Line(0,-1); Line(-1,0);
	}
	l = cymbalP(part, nt, pi)? - ScoreLinePad / 2: 0;
	MoveTo(x1+5, y+l-1); Line(0,-DrumTailLen-l);
	if (len < 4) draw_bm((len < 2)? bmUp16Flag : bmUp8Flag);
}
static void plot_note(short part, short nt[], short k, short len) {
	short	i, j, jf, jt, d, a, m, x0, x1, x, y, l, pre_nt;
	Point	pt = noteP;
	x0 = nt[0] - 7; x1 = 7 - nt[k-1];
	if (x0 > x1) d = 1;
	else if (x0 < x1) d = -1;
	else d = (x0 > 0)? 1 : -1;
	if (d > 0) { jf = 0; jt = k; }
	else { jf = k - 1; jt = -1; }
	pt.h += 2;
	x0 = pt.h;
	for (a = 4, m = 16; a >= 0; a --, m >>= 1) if (len & m) {
		x1 = pt.h;
		pt.h += NoteWUnit * m;
		m >>= 1;
		if (nt[k-1] < 3)
			for (i = (2-nt[k-1])/2; i >= 0; i --) draw_dash(i+8, x1);
		if (nt[0] > 13)
			for (i = (nt[0]-14)/2; i >= 0; i --) draw_dash(2-i, x1);
		RGBForeColor(&partColor[part]);
		pre_nt = -999;
		for (j = jf; j != jt; j += d) {
			x = pre_nt - nt[j];
			if (-1 <= x && x <= 1) { x = ScoreLinePad; pre_nt = -999; }
			else { x = 0; pre_nt = nt[j]; }
			y = pt.v + (18 - nt[j]) * ScoreLinePad / 2;
			MoveTo(x1 + x, y);
			draw_bm((a < 3)? bmBlackHead : bmWhiteHead);
			if (len & m) {
				Move(8, ((nt[j] & 1)? 0 : -2));
				Line(1,0); Line(0,-1); Line(-1,0);
			}
		}
		if (len & m) pt.h += NoteWUnit * m;
		if (a < 4) {
			l = (nt[0] - nt[k-1]) * ScoreLinePad / 2;
			if (d < 0) { MoveTo(x1+5, y+l); Line(0,-TailLen-l); }
			else { MoveTo(x1, y-l); Line(0,TailLen+l); }
			if (a < 2) draw_bm((d < 0)?
				(a? bmUp8Flag : bmUp16Flag) : (a? bmDown8Flag : bmDown16Flag));
		}
		a --;
	}
	if (x0 != x1) draw_tie(x0, x1,
		pt.v + (18 - nt[jf]) * ScoreLinePad / 2, (nt[jf] < 8));
}
static void draw_rest(short part, short len) {
	short	a, m, m2, w;
	Point	pt = noteP;
	RGBForeColor(&partColor[part]);
	pt.h += 2; pt.v += ScoreLinePad * 5;
	for (a = 4, m = 16; a >= 0; a --, m >>= 1) if (len & m) {
		w = NoteWUnit * m;
		m2 = m >> 1;
		if (len & m2) w += NoteWUnit * m2;
		MoveTo(pt.h + w/2 - NoteWUnit/2, pt.v);
		if (a == 4) { Move(0, -2); draw_bm(bmRest2); }
		else draw_bm(bmRest16 - a);
		if (len & m2) {
			Move(8, -2);
			Line(1,0); Line(0,-1); Line(-1,0);
			a --; m = m2;
		}
		pt.h += w;
	}
}
static void draw_notes(Score *sc, BarInfo *pi, short *len, short min) {
	short	i, j, p, k, v_old = noteP.v, notes[3];
	for (i = 0; i < NDispParts; i ++, noteP.v += PartHeight) {
		p = VInfo->viewPartID[i];
		if (sc[p].note == NoteRest) draw_rest(p, len[p]);
		else if (sc[p].note != NoteRemain) {
			if (p >= DsPercPart) {
				j = get_timbre_id(p, sc[p].note, pi);
				plot_drum_note(pi, p, j, len[p]);
			} else {
				k = sc[p].note;
				if (pi->octave[p] <= 3) k -= 2;
				if (PInfo2) k += 8 * (pi->octave[p] - PInfo2->octave[p]);
				if (p < ChordPart) plot_note(p, &k, 1, len[p]);
				else {
					for (j = 0; j < 3; j ++) notes[j] = k + 2 - j * 2;
					plot_note(p, notes, 3, len[p]);
				}
			}
#ifdef	EFFECT
			if (sc[p].effect > 0 && sc[p].effect < 3) {
				unsigned char c = sc[p].effect + '0';
				MoveTo(noteP.h, noteP.v+6); DrawChar(c);
			}
#endif
		}
	}
	noteP.v = v_old;
	noteP.h += NoteWUnit * min;
}
static void draw_five_lines(Rect *rs) {
	short	y, p, k, w;
	y = rs->top + ScoreLinePad * 3;
	w = rs->right - rs->left - 1;
	for (p = 0; p < NDispParts; p ++, y += PartHeight) {
		MoveTo(rs->left, y);
		for (k = 0; k < 5; k ++) { Line(w, 0); Move(-w, ScoreLinePad); }
	}
}
static void draw_8va(short x, short y, short octv) {
	MoveTo(x + 15, y);
	draw_bm(bmOctava);
	if (octv > 5) { Move(0, -2); Line(12, 0); }
	else if (octv < 2)  { Move(0, 6); Line(12, 0); }
}
static void draw_score_head(Rect *rs) {
	short	i, p, x, y = rs->top + ScoreLinePad * 3;
	ForeColor(blackColor);
	PenSize(1,1);
	for (i = 0; i < NDispParts; i ++, y += PartHeight) {
		MoveTo(rs->left+3, y+ScoreLinePad*4);
		p = VInfo->viewPartID[i];
		if (p >= DsPercPart) draw_bm(bmFClef);
		else if (!PInfo) draw_bm(bmGClef);
		else {
			short	octv = PInfo->octave[p];
			draw_bm((octv > 3)? bmGClef : bmFClef);
			if (octv > 4) draw_8va(rs->left, y - 7, octv);
			else if (octv < 3) draw_8va(rs->left, y + ScoreLinePad*4 + 2, octv);
		}
		for (x = rs->left+WinLeftSpace+SubWinWidth-1; x < rs->right; x += SubWinWidth)
			{ MoveTo(x, y); Line(0, ScoreLinePad*4); }
	}
	draw_five_lines(rs);
}
extern	short	playID, play1, play2;
void inverse_playing_mark(void) {
	Rect	r;
	GrafPtr	oldg;
	SBIntegrator	*si;
	short	scale = 1;
	if (!PlayWindow) return;
	switch (GetWRefCon(PlayWindow)) {
		case WIDField:
		r.top = (playID / FieldNColumns) * SubWinYUnit + SubWinBorder
			+ WinTopSpace + IndControlH;
		r.left = (playID % FieldNColumns) * SubWinXUnit + SubWinBorder;
		r.bottom = r.top + SubWinPHeight;
		break;
		case WIDIntegrator:
		si = FindIntegrator(PlayWindow);
		if (!set_integ_pm_rect(si, &r)) return;
		scale = si->displayScale;
		break;
		case WIDgedit:
		SetRect(&r, 0, 0, 0, SubWinPHeight);
	}
	r.left += WinLeftSpace + play1 * NoteWUnit / scale;
	r.right = r.left
	+ (((play2 == 0)? NShortBeats : play2) - play1) * NoteWUnit / scale;
	GetPort(&oldg);
	SetPort(myGetWPort(PlayWindow));
	ForeColor(blackColor);
	PenMode(patXor);
	PaintRect(&r);
	PenMode(patCopy);
	SetPort(oldg);
}
static void draw_bar_id(Rect *rs, short id) {
//	static	short	fontNum = -1;
	static	FontInfo	finfo = { 0, 0 };
	static	RGBColor	col = { 0xbbbb, 0xbbbb, 0xbbbb };
	unsigned char	txt[8];
//	if (fontNum < 0) GetFNum("\pPalatino", &fontNum);
//	TextFont(fontNum);
	TextSize(64);
	if (finfo.ascent <= 0) GetFontInfo(&finfo);
	NumToString(id, txt);
	MoveTo((rs->right - rs->left - StringWidth(txt)) / 2 + rs->left,
		rs->bottom - (rs->bottom - rs->top - finfo.ascent) / 2);
	RGBForeColor(&col);
	DrawString(txt);
}
static void draw_individual_score(Score sc[NTotalParts][NShortBeats],
	Rect *rs, RGBColor *col, short id) {
	static	RgnHandle	oldclip = NULL, clip = NULL, newclip = NULL;
	if (!oldclip) {
		oldclip = NewRgn(); clip = NewRgn(); newclip = NewRgn();
	}
	GetClip(oldclip);
	RectRgn(clip, rs);
	SectRgn(clip, oldclip, newclip);
	SetClip(newclip);
	EraseRect(rs);
	if (col) {
		Rect	rct = *rs;
		rct.bottom -= IntegYPad;
		InsetRect(&rct, 2, 2);
		RGBForeColor(col);
		PaintRect(&rct);
	}
	if (id > 0) draw_bar_id(rs, id);
	ForeColor(blackColor);
	PenSize(1,1);
	draw_five_lines(rs);
	if (sc) {
		noteP.h = rs->left + 1; noteP.v = rs->top;
		proc_score(sc, PInfo, draw_notes);
	}
	SetClip(oldclip);
}
static void update_field_window(SBField *sf, Boolean renewal) {
	unsigned short	i, j, id;
	Rect	rs;
	GrafPtr	oldg;
	GetPort(&oldg);
	SetPort(myGetWPort(sf->win));
	VInfo = &sf->v;
	PInfo = &sf->b; PInfo2 = NULL;
	for (i = id = 0; i < FieldNRows; i ++) {
		rs.top = IndControlH + SubWinYUnit * i + SubWinBorder + WinTopSpace;
		rs.bottom = rs.top + SubWinPHeight;
		rs.left = 0; rs.right = WinLeftSpace - SubWinPad;
		if (renewal) EraseRect(&rs);
		draw_score_head(&rs);
		for (j = 0; j < FieldNColumns; j ++, id ++) {
			rs.left = SubWinXUnit * j + SubWinBorder + WinLeftSpace;
			rs.right = rs.left + SubWinWidth;
			draw_individual_score(sf->pop[id].score, &rs, NULL, 0);
			draw_frame(sf, id);
		}
	}
	if (sf->win == PlayWindow && playID >= 0) inverse_playing_mark();
	SetPort(oldg);
}
void draw_window(SBField *sf) {
	update_field_window(sf, true);
}
void update_field(SBField *sf) {
	RgnHandle	rgn;
	if (!sf) return;
	rgn = NewRgn();
	update_field_window(sf, false);
	UpdateControls(sf->win, GetPortVisibleRegion(myGetWPort(sf->win), rgn));
	DisposeRgn(rgn);
}
void draw_field_ind(SBField *sf, short id) {
	Rect	rs;
	rs.left = SubWinXUnit * (id % FieldNColumns) + SubWinBorder + WinLeftSpace;
	rs.right = rs.left + SubWinWidth;
	rs.top = IndControlH + SubWinYUnit * (id / FieldNColumns) + SubWinBorder + WinTopSpace;
	rs.bottom = rs.top + SubWinPHeight;
	VInfo = &sf->v;
	PInfo = &sf->b; PInfo2 = NULL;
	draw_individual_score(sf->pop[id].score, &rs, NULL, 0);
	if (sf->win == PlayWindow && id == playID) inverse_playing_mark();
}
static	RgnHandle	integHeadRgn = NULL, integOldClip = NULL;
void begin_integ_update(void) {
	GrafPtr	gp;
	Rect	bounds;
	RgnHandle	newClip = NewRgn(), bottomBar = NewRgn();
	if (!integHeadRgn) {
		integHeadRgn = NewRgn(); integOldClip = NewRgn();
		SetRectRgn(integHeadRgn, 0, 0, IntWindowWidth+ScrollBarWidth-1, intWinTopSpace);
	}
	GetPort(&gp);
	GetPortBounds(gp, &bounds);
	SetRectRgn(bottomBar, 0, bounds.bottom - GrowIconSize,
		bounds.right, bounds.bottom);
	GetClip(integOldClip);
	DiffRgn(integOldClip, integHeadRgn, newClip);
	DiffRgn(newClip, bottomBar, newClip);
	SetClip(newClip);
	DisposeRgn(newClip); DisposeRgn(bottomBar);
}
void end_integ_update(void) {
	if (integOldClip) SetClip(integOldClip);
}
static void draw_integ_ind0(SBIntegrator *si, short ns, Rect *rs) {
	integScorePtr	scorep = *(si->scoreHandle);
	static RGBColor	selc = { 0xdddd, 0xdddd, 0xdddd },
		tmpsel1 = { 0x8888, 0xffff, 0xffff }, tmpsel2 = { 0xffff, 0x8888, 0xffff };
	RGBColor	*col;
	CGrafPtr	oldBarPort;
	GDHandle	oldBarGDH;
	Rect	srcRect;
	static	GWorldPtr	barGWorld = NULL;
	switch (scorep[ns].flag & (ItgSelectedFlag | ItgTmpSelectedFlag)) {
		case ItgSelectedFlag: col = &selc; break;
		case ItgTmpSelectedFlag: col = &tmpsel1; break;
		case (ItgSelectedFlag | ItgTmpSelectedFlag): col = &tmpsel2; break;
		default: col = NULL;
	}
	SetRect(&srcRect, 0, 0, SubWinWidth - 1, IntegYUnit);
	if (barGWorld == NULL) {
		QDErr	err = NewGWorld(&barGWorld, 0, &srcRect, NULL, NULL, 0);
		if (err != noErr) {
			error_msg("\pNo enough memory to allocate GWorld for bar", err);
			SetPort(myGetWPort(si->win)); return;
		}
	}
	GetGWorld(&oldBarPort, &oldBarGDH);
	SetGWorld(barGWorld, NULL);
	draw_individual_score((ItgOnP(scorep,ns)? scorep[ns].i.score : NULL),
		&srcRect, col, ns+1);
	SetGWorld(oldBarPort, oldBarGDH);
	CopyBits(GetPortBitMapForCopyBits(barGWorld),
		GetPortBitMapForCopyBits(GetWindowPort(si->win)), &srcRect, rs, srcCopy, NULL);
}
void draw_integ_ind(SBIntegrator *si, short ns, Boolean locked) {
	Rect	rs;
	short	i, k;
	integScorePtr	scorep;
	rs.top = si->barHeight * (ns / si->nSectionsW) - si->scroll;
	if (rs.top > si->winHeight) return;
	rs.bottom = rs.top + si->barHeight;
	if (rs.bottom < 0) return;
	OffsetRect(&rs, 0, intWinTopSpace);
	rs.left = si->barWidth * (ns % si->nSectionsW) + WinLeftSpace;
	rs.right = rs.left + si->barWidth - 1;
	begin_integ_update();
	if (!locked) HLock((Handle)si->scoreHandle);
	scorep = *(si->scoreHandle);
	VInfo = &si->vInfo;
	PInfo = &scorep[ns].b;
	k = (ns / si->nSectionsW) * si->nSectionsW; PInfo2 = NULL;
	for (i = 0; i < si->nSectionsW; i ++, k ++) if (ItgOnP(scorep, k))
		{ PInfo2 = &scorep[k].b; break; }
	if (k == ns) PInfo2 = NULL;
	draw_integ_ind0(si, ns, &rs);
	if (!locked) HUnlock((Handle)si->scoreHandle);
	end_integ_update();
	if (ns == si->sel) draw_sel_frame(si, true);
	if (si->win == PlayWindow && ns == playID) inverse_playing_mark();
}
static void draw_score_head_integ(SBIntegrator *si, Rect *rs) {
	static GWorldPtr gw = NULL;
	OSErr	err;
	CGrafPtr	old;
	GDHandle	gdh;
	Rect rct, rctdst;
	short x, y, i, scoreHeight;
	SetRect(&rct, 0, 0, WinLeftSpace * 4, IntegYUnit);
	if (!gw) {
		err = NewGWorld(&gw, 0, &rct, NULL, NULL, 0);
		if (!gw) return;
	}
	GetGWorld(&old, &gdh);
	SetGWorld(gw, NULL);
	EraseRect(&rct);
	draw_score_head(&rct);
	SetGWorld(old, gdh);
	rct.right = WinLeftSpace * SubWinWidth / si->barWidth;
	rctdst = *rs;
	rctdst.right = WinLeftSpace;
	CopyBits(GetPortBitMapForCopyBits(gw),
		GetPortBitMapForCopyBits(old), &rct, &rctdst, srcCopy, NULL);
	scoreHeight = ScoreLinePad * 4 / si->displayScale;
	for (x = WinLeftSpace + si->barWidth - 1; x < rs->right; x += si->barWidth)
		for (i = 0; i < NDispParts; i ++) {
			y = rs->top + ((PartHeight * i) + ScoreLinePad * 3) / si->displayScale;
			MoveTo(x, y); Line(0, scoreHeight);
		}
}
void draw_integ_scores(SBIntegrator *si) {
	Rect	rs;
	short	i, j, k, headNs;
	integScorePtr	scorep;
	BarInfo	*headPInfo;
	HLock((Handle)si->scoreHandle);
	scorep = *(si->scoreHandle);
	VInfo = &si->vInfo;
	SetRect(&rs, 0, intWinTopSpace, IntWindowWidth, si->winHeight + intWinTopSpace);
	EraseRect(&rs);
	for (j = si->scroll / si->barHeight;
		j * si->barHeight < si->scroll + si->winHeight; j ++) {
		rs.top = noteP.v = j * si->barHeight + intWinTopSpace - si->scroll;
		rs.bottom = rs.top + si->barHeight;
		rs.left = 0; rs.right = IntWindowWidth;
		headNs = j * si->nSectionsW; headPInfo = NULL;
		for (i = 0; i < si->nSectionsW; i ++, headNs ++) if (ItgOnP(scorep, headNs))
			{ headPInfo = &scorep[headNs].b; break; }
		PInfo = headPInfo; draw_score_head_integ(si, &rs);
		k = j * si->nSectionsW; rs.left = WinLeftSpace;
		for (i = 0; i < si->nSectionsW; i ++, k ++, rs.left += si->barWidth) {
			rs.right = rs.left + si->barWidth - 1;
			PInfo = &scorep[k].b;
			PInfo2 = (k == headNs)? NULL : headPInfo;
			draw_integ_ind0(si, k, &rs);
		}
	}
	HUnlock((Handle)si->scoreHandle);
}
/*
static BarInfo *get_pinfo_for_head(integScorePtr scorep, short row) {
	short	i, k;
	for (i = 0, k = row * NSectionsW; i < NSectionsW; i ++, k ++)
		if (ItgOnP(scorep, k)) return &scorep[k].b;
	return NULL;
}
*/
void check_integ_score_head(SBIntegrator *si, integScorePtr scorep, short row) {
	short	i, k;
	Rect	rs;
	GrafPtr	oldG;
	rs.top = row * si->barHeight + intWinTopSpace - si->scroll;
	if (rs.top > si->winHeight) return;
	rs.bottom = rs.top + si->barHeight;
	if (rs.bottom < intWinTopSpace) return;
	rs.left = 0; rs.right = WinLeftSpace;
	k = row * si->nSectionsW; PInfo = NULL;
	for (i = 0; i < si->nSectionsW; i ++, k ++) if (ItgOnP(scorep, k))
		{ PInfo = &scorep[k].b; break; }
	VInfo = &si->vInfo;
	GetPort(&oldG); SetPort(myGetWPort(si->win));
	begin_integ_update();
	EraseRect(&rs);
	draw_score_head_integ(si, &rs);
	end_integ_update();
	SetPort(oldG);
}
static void gray_part(short from, short to) {
	static	GWorldPtr	gw = NULL;
	static	RGBColor	ltGray = {0xcccc,0xcccc,0xcccc},
		gray = {0x8888,0x8888,0x8888};
	Rect	rs;
	CGrafPtr	old;
	GDHandle	gdh;
	GetGWorld(&old, &gdh);
	if (!gw) {
		OSErr	err;
		SetRect(&rs, 0, 0, WinLeftSpace + SubWinWidth, SubWinPHeight);
		err = NewGWorld(&gw, 0, &rs, NULL, NULL, 0);
		if (!gw) return;
		SetGWorld(gw, NULL);
		RGBForeColor(&ltGray);
		PaintRect(&rs);
		SetGWorld(old, gdh);
	}
	OpColor(&gray);
	SetRect(&rs, 0, from * PartHeight,
		WinLeftSpace + SubWinWidth, to * PartHeight);
	CopyBits(GetPortBitMapForCopyBits(gw),
		GetPortBitMapForCopyBits(old), &rs, &rs, blend, NULL);
}
void draw_gedit(SBgedit *sg) {
	Rect	rs;
	short	vp;
	SetRect(&rs, 0, 0, WinLeftSpace+SubWinWidth, SubWinPHeight);
	VInfo = &sg->v;
	PInfo = &sg->b; PInfo2 = NULL;
	EraseRect(&rs);
	draw_score_head(&rs);
	noteP.h = WinLeftSpace; noteP.v = 0;
	proc_score(sg->ind.score, PInfo, draw_notes);
	for (vp = 0; vp < NDispParts; vp ++)
		if (sg->v.viewPartID[vp] == sg->editPart) break;
	if (vp < NDispParts) {
		if (vp > 0) gray_part(0, vp);
		if (vp < NDispParts-1) gray_part(vp+1, NDispParts);
	}
	if (sg->win == PlayWindow) inverse_playing_mark();
}
