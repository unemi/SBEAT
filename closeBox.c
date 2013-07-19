/*
struct ControlDefSpec {
    ControlDefType defType; 
    union {
          ControlDefUPP defProc; 
      void *classRef;
    } u;
};
kControlDefObjectClass,
*/
#define  DEBUG
#include	<stdio.h>
OSStatus CreateCloseBox(WindowRef, Rect *, ControlRef *);

static	pascal	SInt32	def_proc(
	SInt16		varCode,
	ControlRef	theControl,
	ControlDefProcMessage	message,
	SInt32 param)
{
	Rect	bound;
	Point	pt;
#ifdef	DEBUG
	static	FILE	*fd = NULL;
	static	long	logN = 0;
	if (!fd) fd = fopen("DefProc.log", "w");
#endif
	switch (message) {
		case initCntl: return 0;
		case calcCntlRgn:
		GetControlBounds(theControl, &bound);
		RectRgn((RgnHandle)param, &bound);
		return 0;
		case kControlMsgTestNewMsgSupport: return 0;
		case drawCntl: if (IsControlVisible(theControl)) {
			ForeColor(GetControlValue(theControl)? greenColor : redColor);
			PaintOval(GetControlBounds(theControl, &bound));
		} break;
		case testCntl: if (!IsControlActive(theControl)) return kControlNoPart;
		pt.v = HiWord(param); pt.h = LoWord(param);
		return PtInRect(pt, GetControlBounds(theControl, &bound))?
			kControlButtonPart : kControlNoPart;
/*		case posCntl:*/
	}
#ifdef	DEBUG
	if (fd) {
		fprintf(fd, "%d %d %d %d %d\r", ++logN, varCode, message, param,
			GetControlReference(theControl));
		fflush(fd);
	}
#endif
	return 0;
}
OSStatus	CreateCloseBox(
	WindowRef	inWindow,
	Rect		*inBoundRect,
	ControlRef	*outControl)
{
	ControlDefSpec	cds;
	cds.defType = kControlDefProcPtr;
	cds.u.defProc = NewControlDefUPP(def_proc);
	return CreateCustomControl(
		inWindow,
		inBoundRect,
		&cds,
		NULL,
		outControl);
}
