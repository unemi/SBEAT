#include  <Drag.h>
#include	"decl.h"
#define	flavorTypeInd	'SRCI'
typedef	struct {
	WindowPtr	win;
	short	id, part;
	Rect	rect;
	Individual	*i;
	BarInfo	*bi;
	ViewInfo	*vi;
}	DraggedIndividual;
typedef struct {
	DraggedIndividual	src;
	WindowPtr	dst_win;
	short		dst_id;
	Rect		dst_rect;
}	MyDragGlobalType;
static DragTrackingHandlerUPP	MyTrackingHandler = NULL;
static DragReceiveHandlerUPP	MyReceiveHandler = NULL;
static MyDragGlobalType	theDragGlobal;

static Boolean get_drag_item(DragReference theDrag, OSType type,
	Ptr flavor, Size size, Boolean (*proc)(Ptr)) {
	unsigned short items;
	short	index;
	ItemReference theItem;
	OSErr result;
	CountDragItems(theDrag, &items);
	for (index = 1; index <= items; index++) {
		GetDragItemReferenceNumber(theDrag, index, &theItem);
		result = GetFlavorData(theDrag, theItem, type, flavor, &size, 0L);
		if (result == noErr) {
			if (!proc) return true;
			else if ((*proc)(flavor)) return true;
		}
	}
	return false;
}
static Boolean check_my_file(Ptr p) {
	HFSFlavor	*flv = (HFSFlavor *)p;
	if (flv->fileCreator != CreatorCode) return false;
	switch (flv->fileType) {
		case FileTypeField: case FileTypeIntegrator: case FileTypeOptions:
		return true;
		default: return false;
	}
}
#define	GetDragItemFile	get_drag_item(theDrag,flavorTypeHFS,(Ptr)&flavor,sizeof(flavor),check_my_file)
static pascal OSErr drag_file_tracking(DragTrackingMessage theMessage,
	WindowPtr theWindow, void *, DragReference theDrag) {
	RgnHandle	hiliteRgn;
	Rect	portRect;
	HFSFlavor	flavor;
	switch(theMessage) {
		case kDragTrackingEnterWindow:
		if (!GetDragItemFile) break;
		RectRgn(hiliteRgn = NewRgn(),
			GetWindowPortBounds(theWindow, &portRect));
		ShowDragHilite(theDrag, hiliteRgn, true);
		DisposeRgn(hiliteRgn);
		break;
		case kDragTrackingLeaveWindow:
		HideDragHilite(theDrag);
	}
	return 0;
}
static pascal OSErr drag_file_receive(WindowPtr, void *, DragReference theDrag) {
	HFSFlavor	flavor;
	if (!GetDragItemFile) return 0;
	open_from_fss(&flavor.fileSpec, flavor.fileType);
	return 0;
}
void DropFileInit(void) {
	OSErr	err;
	err = InstallTrackingHandler(
		NewDragTrackingHandlerUPP(&drag_file_tracking), NULL, NULL);
	err = InstallReceiveHandler(
		NewDragReceiveHandlerUPP(&drag_file_receive), NULL, NULL);
}
static Boolean global_to_field_part(Point p, DraggedIndividual *di) {
	short	i, j;
	GlobalToLocal(&p);
	if ((p.h -= WinLeftSpace) < 0) return false;
	if ((i = p.h / SubWinXUnit) >= FieldNColumns) return false;
	if ((p.v -= WinTopSpace) < 0) return false;
	if ((j = p.v / SubWinYUnit) >= FieldNRows) return false;
	if ((p.v -= j * SubWinYUnit + SubWinBorder + IndControlH) < 0) return false;
	else if ((di->part = p.v / PartHeight) >= NDispParts)
		di->part = NDispParts - 1;
	di->id = j * FieldNColumns + i;
	return true;
}
void field_part_to_rect(short id, short part, Rect *r) {
	r->left = (id % FieldNColumns) * SubWinXUnit + WinLeftSpace + SubWinBorder;
	r->right = r->left + SubWinWidth;
	r->top = (id / FieldNColumns) * SubWinYUnit + SubWinBorder + WinTopSpace + IndControlH;
	if (0 <= part && part < NDispParts) {
		r->top += part * PartHeight;
		r->bottom = r->top + PartHeight;
	} else r->bottom = r->top + SubWinPHeight;
}
static short get_drop_field_id(DragReference theDrag) {
	Point	p;
	DraggedIndividual	dst;
	GetDragMouse(theDrag, &p, 0L);
	return global_to_field_part(p, &dst)? dst.id : -1;
}
static Boolean global_to_gedit_part(Point p, DraggedIndividual *di) {
	GlobalToLocal(&p);
	if ((p.h -= WinLeftSpace) < 0) return false;
	di->part = p.v / PartHeight;
	if (di->part >= NDispParts) di->part = NDispParts - 1;
	di->id = 0;
	return true;
}
void gedit_part_to_rect(short part, Rect *r) {
	r->left = WinLeftSpace;
	r->right = r->left + SubWinWidth;
	r->top = 0;
	if (0 <= part && part < NDispParts) {
		r->top += part * PartHeight;
		r->bottom = r->top + PartHeight;
	} else r->bottom = r->top + SubWinPHeight;
}
typedef enum {
	IntegUnknownArea	= -99,
	IntegTopSpace		= -10,
	IntegBottomSpace	= -9,
	IntegLeftSpace		= -8,
	IntegScrollBar		= -7,
	IntegScoreArea		= 0
}	IntegAreaType;
static IntegAreaType global_to_integ_part(Point p,
	SBIntegrator *si, DraggedIndividual *di) {
	short	col, row, part;
	Rect	prect;
	GetWindowPortBounds(si->win, &prect);
	GlobalToLocal(&p);
	if (p.v < 0 || p.v >= prect.bottom || p.h < 0 || p.h >= prect.right)
		return IntegUnknownArea;
	if ((p.v -= intWinTopSpace) < 0)
		{ di->id = p.v; return IntegTopSpace; }
	if (p.v > si->winHeight)
		{ di->id = p.v - si->winHeight; return IntegBottomSpace; }
	if ((p.h -= WinLeftSpace) < 0) return IntegLeftSpace;
	if ((col = p.h / si->barWidth) >= si->nSectionsW) return IntegScrollBar;
	row = (p.v += si->scroll) / si->barHeight;
	if (row < 0 || si->nSectionsH <= row) return IntegUnknownArea;
	part = (p.v - row * si->barHeight) * si->displayScale / PartHeight;
	if (part >= NDispParts) return IntegUnknownArea;
	di->part = part;
	di->id = row * si->nSectionsW + col;
	return IntegScoreArea;
}
void integ_part_to_rect(short id, short part, SBIntegrator *si, Rect *r) {
	r->left = (id % si->nSectionsW) * si->barWidth + WinLeftSpace;
	r->right = r->left + si->barWidth;
	r->top = (id / si->nSectionsW) * si->barHeight + intWinTopSpace - si->scroll;
	if (0 <= part && part < NDispParts) {
		r->top += part * PartHeight / si->displayScale;
		r->bottom = r->top + PartHeight / si->displayScale;
	} else r->bottom = r->top + SubWinPHeight / si->displayScale;
}
static IntegAreaType get_drop_integ_id(SBIntegrator *si,
	DragReference theDrag, short *return_id) {
	Point	p;
	DraggedIndividual	dst;
	IntegAreaType	result;
	GetDragMouse(theDrag, &p, 0L);
	result = global_to_integ_part(p, si, &dst);
	if (return_id) *return_id = dst.id;
	return result;
}
#define	GetDragItem	get_drag_item(theDrag,flavorTypeInd,(Ptr)&g->src,sizeof(DraggedIndividual),NULL)
static pascal OSErr drag_tracking(DragTrackingMessage theMessage, WindowPtr theWindow,
	void *myGlobals, DragReference theDrag) {
	MyDragGlobalType	*g = (MyDragGlobalType *)myGlobals;
	short	dst_id;
	Rect	rct;
	RgnHandle	hiliteRgn;
	Boolean	FieldP;
	if (!GetDragItem) return 1;
	switch(theMessage) {
		case kDragTrackingEnterWindow:
		g->dst_win = theWindow;
		g->dst_id = -1;
		break;
		case kDragTrackingInWindow:
		if (!g->src.win) break;
		FieldP = PlayTypeFieldP(theWindow);
		if (FieldP) dst_id = get_drop_field_id(theDrag);
		else {
			SBIntegrator *si = FindIntegrator(theWindow);
			short	k;
			dst_id = -1;
			switch (get_drop_integ_id(si, theDrag, &k)) {
				case IntegTopSpace: drag_integ_scroll(si, -1, k); break;
				case IntegBottomSpace: drag_integ_scroll(si, 1, k); break;
				default: drag_integ_scroll(NULL, 0, 0); dst_id = k;
			}
		}
		if (dst_id == g->dst_id) break;
		if (g->src.win == theWindow && dst_id == g->src.id) dst_id = -1;
		if (g->dst_id >= 0 && dst_id < 0) HideDragHilite(theDrag);
		else if (dst_id >= 0) {
			Point	p = {0, 0};
			if (FieldP)
				field_part_to_rect(dst_id, g->src.part, &rct);
			else integ_part_to_rect(dst_id, g->src.part, FindIntegrator(theWindow), &rct);
			RectRgn(hiliteRgn = NewRgn(), &rct);
			if (!FieldP) begin_integ_update();
			ShowDragHilite(theDrag, hiliteRgn, true);
			if (!FieldP) end_integ_update();
			DisposeRgn(hiliteRgn);
			LocalToGlobal(&p);
			OffsetRect(&rct, p.h, p.v);
			g->dst_rect = rct;
		}
		g->dst_id = dst_id;
		break;
		case kDragTrackingLeaveWindow:
		g->dst_win = NULL;
		HideDragHilite(theDrag);
	}
	return 0;
}
static short drop_partial_gene(MyDragGlobalType *g, Individual *ind) {
	short	i, partS, partM, partR, partV;
	partS = g->src.vi->viewPartID[g->src.part];
	partM = g->src.bi->genePart[partS][0];
	partR = g->src.bi->genePart[partS][1];
	partV = g->src.bi->genePart[partS][2] + GnFraction;
	for (i = 0; i < NShortBeats; i ++) {
		ind->gene[partR][i] = (ind->gene[partR][i] & 0x1f)
			| (g->src.i->gene[partR][i] & 0xe0);
		ind->gene[partM][i] = (ind->gene[partM][i] & 0xe0)
			| (g->src.i->gene[partM][i] & 0x1f);
		ind->gene[partV][i] = g->src.i->gene[partV][i];
	}
	return partS;
}
#define	MovingRectAnime	ZoomRects(&g->src.rect, &g->dst_rect, 25, 2)
static OSErr drag_recive_by_field(SBField *sf,
	MyDragGlobalType *g, DragReference theDrag) {
	short	id;
	if (!sf) return userCanceledErr;
	if ((id = get_drop_field_id(theDrag)) < 0) return userCanceledErr;
	if (sf->win == g->src.win && id == g->src.id) return userCanceledErr;
	MovingRectAnime;
	enque_field_ind_history(sf, id);
	if (g->src.part >= 0) drop_partial_gene(g, &sf->pop[id]);
	else BlockMove(g->src.i->gene[0], sf->pop[id].gene[0], BytesParNote*NShortBeats);
	develop_score(&sf->pop[id], &sf->b);
	HideDragHilite(theDrag);
	field_ind_changed(sf, id);
	return 0;
}
static OSErr drag_recive_by_integ(SBIntegrator *si,
	MyDragGlobalType *g, DragReference theDrag) {
	extern	WindowPtr	TopWindow;
	short	ns, selp;
	integScorePtr	scorep;
	if (!si) return userCanceledErr;
	if (get_drop_integ_id(si, theDrag, &ns) != IntegScoreArea)
		return userCanceledErr;
	if (si->win == g->src.win && ns == g->src.id) return userCanceledErr;
	MovingRectAnime;
	enque_integ_history(HistIOneBar, si, ns, 0);
	HideDragHilite(theDrag);
	HLock((Handle)si->scoreHandle);
	scorep = *(si->scoreHandle);
	if (g->src.part < 0) {
		scorep[ns].i = *g->src.i;
		scorep[ns].b = *g->src.bi;
	} else {
		short	i, j, part;
		BarInfo	*pd = &scorep[ns].b, *ps = g->src.bi;
		part = drop_partial_gene(g, &scorep[ns].i);
		pd->playOn[part] = ps->playOn[part];
		j = (part < DsPercPart)? part : DsPercPart;
		pd->instrument[j] = ps->instrument[j];
		for (i = 0; i < NControls; i ++)
			pd->control[j][i] = ps->control[j][i];
		if (part >= DsPercPart) {
			j = part - DsPercPart;
			pd->drumsInst[j] = ps->drumsInst[j];
		}
		if (part < NNoteCh - 1) pd->octave[part] = ps->octave[part];
		pd->protect[part] = ps->protect[part];
		pd->unitBeat[part] = ps->unitBeat[part];
		pd->iteration[part] = ps->iteration[part];
		for (i = 0; i < 3; i ++)
			pd->genePart[part][i] = ps->genePart[part][i];
		if (!ItgOnP(scorep, ns)) for (i = 0; i < NTotalParts; i ++)
			if (i != part) pd->playOn[i] = false;
		develop_score(&scorep[ns].i, pd);
	}
	scorep[ns].flag |= ItgOnFlag;
	if (ns == si->sel) activate_integ_sel_ctrl(si, true, true);
	draw_integ_ind(si, ns, true);
	check_integ_score_head(si, scorep, ns / si->nSectionsW);
	selp = ItgSelectedP(scorep, ns);
	HUnlock((Handle)si->scoreHandle);
	integ_modified(si);
	if (selp) {
		activate_integ_sel2_ctrl(si);
		set_current_integ_pi(si);
	}
	revise_tune_queue(si->win, ns);
	return 0;
}
static pascal OSErr drag_receive(WindowPtr win, void *myGlobals, DragReference theDrag) {
	MyDragGlobalType *g = (MyDragGlobalType *)myGlobals;
	if (!GetDragItem) return userCanceledErr;
	switch (GetWRefCon(win)) {
		case WIDField:
		return drag_recive_by_field(FindField(win), g, theDrag);
		case WIDIntegrator:
		return drag_recive_by_integ(FindIntegrator(win), g, theDrag);
		default: return -1;
	}
}
void set_drag_callback(WindowPtr window) {
	OSErr	result;
	if (!MyTrackingHandler)
		MyTrackingHandler = NewDragTrackingHandlerUPP(&drag_tracking);
	result = InstallTrackingHandler(MyTrackingHandler, window, &theDragGlobal);
	if (result) error_msg("\pInstallTrackingHandler", result);
	if (!MyReceiveHandler)
		MyReceiveHandler = NewDragReceiveHandlerUPP(&drag_receive);
	result = InstallReceiveHandler(MyReceiveHandler, window, &theDragGlobal);
	if (result) error_msg("\pInstallReceiveHandler", result);
}
void remove_drag_callback(WindowPtr window) {
	if (MyTrackingHandler) RemoveTrackingHandler(MyTrackingHandler, window);
	if (MyReceiveHandler) RemoveReceiveHandler(MyReceiveHandler,window);
}
static void drag_ind(DraggedIndividual *s, ItemReference ir, EventRecord *e) {
	OSErr	result;
	unsigned char	*msg = "\pdrag_ind";
	Point	p;
	DragReference	theDrag = NULL;
	RgnHandle	dragRegion = NULL, tempRgn = NULL;
	p.h = p.v = 0; LocalToGlobal(&p); 
	OffsetRect(&s->rect, p.h, p.v);
	result = NewDrag(&theDrag);
	if (result) { msg = "\pNewDrag"; goto error; }
	result = AddDragItemFlavor(theDrag, ir, flavorTypeInd,
		s, sizeof(DraggedIndividual), flavorSenderOnly);
	if (result) { msg = "\pAddDragItemFlavor"; goto error; }
	dragRegion = NewRgn(); if (!dragRegion) goto error;
	RectRgn(dragRegion, &s->rect);
	tempRgn = NewRgn(); if (!tempRgn) goto error;
	CopyRgn(dragRegion, tempRgn);
	InsetRgn(tempRgn, 2, 2);
	DiffRgn(dragRegion, tempRgn, dragRegion);
	result = TrackDrag(theDrag, e, dragRegion);
	if (result == userCanceledErr) result = 0;
	msg = "\pTrackDrag";
error:
	if (dragRegion) DisposeRgn(dragRegion);
	if (theDrag) DisposeDrag(theDrag);
	if (result) error_msg(msg, result);
}
void drag_field_ind(SBField *sf, EventRecord *e) {
	static	DraggedIndividual	src;
	src.win = sf->win;
	if (!global_to_field_part(e->where, &src)) return;
	if (!(e->modifiers & optionKey)) src.part = -1;
	field_part_to_rect(src.id, src.part, &src.rect);
	src.i = &sf->pop[src.id];
	src.bi = &sf->b;
	src.vi = &sf->v;
	drag_ind(&src, (ItemReference)sf, e);
}
void drag_gedit_ind(SBgedit *sg, EventRecord *e) {
	static	DraggedIndividual	src;
	src.win = sg->win;
	if (!global_to_gedit_part(e->where, &src)) return;
	if (!(e->modifiers & optionKey)) src.part = -1;
	gedit_part_to_rect(src.part, &src.rect);
	src.i = &sg->ind;
	src.bi = &sg->b;
	src.vi = &sg->v;
	drag_ind(&src, (ItemReference)sg, e);
}
void drag_integ_ind(SBIntegrator *si, EventRecord *e) {
	static	DraggedIndividual	src;
	static	Individual	indBuf;
	static	BarInfo	bInfoBuf;
	SetPort(myGetWPort(src.win = si->win));
	if (global_to_integ_part(e->where, si, &src) != IntegScoreArea) return;
	if (!(e->modifiers & optionKey)) src.part = -1;
	integ_part_to_rect(src.id, src.part, si, &src.rect);
	HLock((Handle)si->scoreHandle);
	indBuf = (*si->scoreHandle)[src.id].i;
	bInfoBuf = (*si->scoreHandle)[src.id].b;
	src.i = &indBuf;
	src.bi = &bInfoBuf;
	src.vi = &si->vInfo;
	HUnlock((Handle)si->scoreHandle);
	drag_ind(&src, (ItemReference)si, e);
}
