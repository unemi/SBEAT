/**
  SBEAT: A classic MacOS 9 app of breeding tool for musical composition
  developed by T. Unemi in 2001
  Copyright (C) 2013  unemi

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**/


#include  <stdio.h>
#include	<string.h>
#include	<QuickTimeMusic.h>
#include	"decl.h"
#include	"vers.h"
#define	MIDIheadBytes	14
#define	MIDITimeBase	96

static	unsigned char	*MidiBuffer = NULL;
static	long	MidiBufIdx = 0;
static	short	MaxVelocity, MaxVolume;
static	Boolean	AdjustVelocity;
static	unsigned char	prev_com;
#define PutByte(c)	MidiBuffer[MidiBufIdx ++] = (c)

static long put_com(unsigned char com) {
	if (com != prev_com) {
		if (MidiBuffer) PutByte(com);
		prev_com = com; return 1;
	} else return 0;
}
#define	beat2time(b)	((b) * MIDITimeBase / 4)
static long put_delay_time(long dtime) {
	long	s, n = 0;
	unsigned char	c;
	for (s = 28; s > 0; s -= 7) {
		c = (dtime >> s) & 0x7f;
		if (c != 0 || n > 0)
			{ if (MidiBuffer) PutByte(c | 0x80); n ++; }
	}
	if (MidiBuffer) PutByte(dtime & 0x7f);
	return n + 1;
}
static long put_note_on_off(
	short *delay, short part, short note, short vel, BarInfo *pi) {
	unsigned char	com = 0x90 | ((part >= DsPercPart)? 9 :
		((part < 9)? part : part + 1));
	long	n = put_delay_time(beat2time(*delay));
	*delay = 0;
	if (AdjustVelocity) {
		if (MidiBuffer) {
			vel = vel * 127 / MaxVelocity;
			if (vel > 127) vel = 127;
		} else if (MaxVelocity < vel) MaxVelocity = vel;
	}
	if (part < ChordPart) {
		n += put_com(com);
		if (MidiBuffer) {
			PutByte(my_note(part, note, pi)); PutByte(vel);
		} n += 2;
	} else if (part < DsPercPart) {
		n += put_com(com);
		if (MidiBuffer) {
			PutByte(my_note(part, note+2, pi)); PutByte(vel);
			PutByte(0); PutByte(my_note(part, note, pi)); PutByte(vel);
			PutByte(0); PutByte(my_note(part, note-2, pi)); PutByte(vel);
		} n += 8;
	} else {
		n += put_com(com);
		if (MidiBuffer) {
			PutByte(get_timbre(part, note, pi)); PutByte(vel);
		} n += 2;
	}
	return n;
}
#define note_off(delay, part, note, pi)	put_note_on_off(delay, part, note, 0, pi)
#define note_on(delay, part, note, vel, pi)	put_note_on_off(delay, part, note, vel, pi)
static long score2midi(Score sc[NTotalParts][NShortBeats], BarInfo *pi, short *delay) {
	short	i, p;
	long	length = 0;
	Boolean	on[NTotalParts];
	short	note[NTotalParts];
	for (p = 0; p < NTotalParts; p ++) on[p] = false;
	for (i = 0; i < NShortBeats; i ++) {
		for (p = 0; p < NTotalParts; p ++) if (pi->playOn[p]) {
			switch (sc[p][i].note) {
				case NoteRest: if (on[p]) {
					length += note_off(delay, p, note[p], pi);
					on[p] = false;
				} break;
				case NoteRemain: break;
				default:
				if (on[p]) length += note_off(delay, p, note[p], pi);
				length += note_on(delay, p, sc[p][i].note, sc[p][i].velocity+64, pi);
				note[p] = sc[p][i].note;
				on[p] = true;
			}
		}
		(*delay) ++;
	}
	for (p = 0; p < NTotalParts; p ++) if (on[p])
		length += note_off(delay, p, note[p], pi);
	return length;
}
extern short	cntlID[];
static short controls2midi(BarInfo *pi, BarInfo *old_pi) {
	short	p, c, d, n, k, pb, pp;
	unsigned char	com;
	n = 0;
	if (!old_pi || old_pi->playOn[0] || old_pi->tempo != pi->tempo) {
		if (MidiBuffer) {
			long	count = 60000000 / pi->tempo;
			PutByte('\0'); PutByte(0xff); PutByte(0x51);	/* Set tempo */
			PutByte(0x03); PutByte(count >> 16);
			PutByte((count >> 8) & 0xff); PutByte(count & 0xff);
		}
		if (old_pi) {
			old_pi->tempo = pi->tempo;
			old_pi->playOn[0] = false;
		}
		n += 7;
	}
	for (p = 0; p < NNoteCh; p ++) {
		if (p < DsPercPart) {
			if (!pi->playOn[p]) continue;
			pp = (p < 9)? p : p + 1;
		} else {
			for (k = p; k < NTotalParts; k ++) if (pi->playOn[k]) break;
			if (k >= NTotalParts) continue;
			pp = 9;
		}
		if (!old_pi || old_pi->instrument[p] != pi->instrument[p]) {
			unsigned short	gmNum = pi->instrument[p] - 1;
			unsigned short	oldGM = old_pi? old_pi->instrument[p] - 1 : 0;
			if (p >= DsPercPart) {
				gmNum -= kFirstDrumkit;
				if (old_pi) oldGM -= kFirstDrumkit;
			}
			if ((gmNum & 0xff80) != (oldGM & 0xff80)) {
				if (MidiBuffer) {
					PutByte('\0');
					PutByte(0xb0 | pp); PutByte('\0');	/* bank select MSB */
					PutByte((gmNum >> 7) & 0xff);
				}
				n += 4;
			}
			if (MidiBuffer) {
				PutByte('\0');
				PutByte(0xc0 | pp);	/* Program change */
				PutByte(gmNum & 0x7f);
			}
			n += 3;
			if (old_pi) old_pi->instrument[p] = pi->instrument[p];
			prev_com = 0xc0 | pp;
		}
		com = 0;
		for (c = 0; c < NControls; c ++) {
			if (old_pi) d = old_pi->control[p][c];
			else switch (cntlID[c]) {
				case kControllerPan: case kControllerPitchBend: d = 64; break;
//				case kControllerVolume: case kControllerExpression: d = 127; break;
				default: d = 0;
			}
			if (pi->control[p][c] != d) {
				if (MidiBuffer) PutByte('\0');
				n ++;
				switch (cntlID[c]) {
					case kControllerPitchBend: n += put_com(0xe0 | pp);
					if (MidiBuffer) {
						pb = ((pi->control[p][c] - 64) * 64) + 0x2000;
						PutByte(pb & 0x7f); PutByte((pb >> 7) & 0x7f);
					} n += 2; break;
					case kControllerAfterTouch: n += put_com(0xd0 | pp);
					if (MidiBuffer) PutByte(pi->control[p][c]);
					n ++; break;
					case kControllerVolume: if (AdjustVelocity) {
						n += put_com(0xb0 | pp);
						if (MidiBuffer) {
							short	vol = pi->control[p][c] * 127 / MaxVolume;
							if (vol > 127) vol = 127;
							PutByte(cntlID[c]);
							PutByte(vol);
						} else if (MaxVolume < pi->control[p][c])
							MaxVolume = pi->control[p][c];
						n += 2; break;
					}
					default: n += put_com(0xb0 | pp);
					if (MidiBuffer) {
						PutByte(cntlID[c]);
						PutByte(pi->control[p][c]);
					} n += 2;
				}
				if (old_pi) old_pi->control[p][c] = pi->control[p][c];
			}
		}
	}
	return n;
}
static void set_default_pi(BarInfo *pi) {
	short	i, j;
	for (i = 0; i < NNoteCh; i ++) {
		pi->instrument[i] = (i == DsPercPart)? kFirstDrumkit+1 : 1;
		for (j = 0; j < NControls; j ++) switch (cntlID[j]) {
			case kControllerPan: pi->control[i][j] = 64; break;
			case kControllerReverb: pi->control[i][j] = 40; break;
//			case kControllerVolume: case kControllerExpression:
//			pi->control[i][j] = 127; break;
			default: pi->control[i][j] = 0;
		}
	}
	pi->playOn[0] = true;	/* used as the first-time flag */
}
void write_smf(WindowPtr win, short ref, smfOptionalInfo *smfInfo) {
	static	unsigned char	seqName[32] = {0};
	static	char	timeMark[] = { 0, 0xff, 0x58, 4, 4, 2, 24, 8 };
	static	Handle	dateFormat = NULL;
	unsigned char	*copyRight = smfInfo->copyRight;
	long	count, length;
	BarInfo	*pi;
	OSErr	err;
	short	i, k, delay, keyScale, lastTempo;
	unsigned long	secs;
	unsigned char	dateStr[12];
	char hbuf[MIDIheadBytes+4];
	struct {
		unsigned long	typeHD;
		unsigned long	lengthHD;
		unsigned short	format;
		unsigned short	ntracks;
		unsigned short	timeBase;
	} header;
	extern	BarInfo	*CurrentBI;
	AdjustVelocity = smfInfo->adjust;
	if (CurrentBI) keyScale = CurrentBI->keyScale;
	else if (GetWRefCon(win) == WIDIntegrator) {
		SBIntegrator	*si = FindIntegrator(win);
		if (!si || !ItgOnP(*si->scoreHandle,0)) keyScale = KeyInitVal;
		else keyScale = (*si->scoreHandle)[0].b.keyScale;
	}
	if (!seqName[0]) {
		sprintf((char *)seqName+1, "sbeat %s ", ProductVersionShort);
		seqName[0] = strlen((char *)seqName+1);
	}
	if (!dateFormat) dateFormat = GetResource('itl0', 128);
	if (dateFormat) {
		GetDateTime(&secs);
		DateString(secs, shortDate, dateStr, dateFormat);
	} else dateStr[0] = 0;
	header.typeHD = 'MThd';
	header.lengthHD = 6;
	header.format = 0;
	header.ntracks = 1;
	header.timeBase = MIDITimeBase;
	BlockMove(&header, hbuf, MIDIheadBytes);
	BlockMove("MTrk", hbuf+MIDIheadBytes, 4);
	count = MIDIheadBytes + 4;
	err = FSWrite(ref, &count, hbuf);
	if (err) return;
	length = MidiBufIdx = 4		/* for length of track */
		+ (copyRight[0]? 4 + copyRight[0] : 0)		/* Copyright */
		+ 4 + seqName[0] + dateStr[0]				/* Sequence name */
		+ 8						/* Beat mark */
		+ 6;					/* Chord mark */
	prev_com = 0;
	MaxVelocity = MaxVolume = 0;
	switch (GetWRefCon(win)) {
		case WIDField: {
			SBField	*sf = FindField(win); if (!sf) return;
			pi = &sf->b;
			length += controls2midi(pi, NULL);
			for (i = delay = 0; i < PopulationSize; i ++)
				length += score2midi(sf->pop[i].score, pi, &delay);
			MidiBuffer = (unsigned char *)NewPtr(length + 4);
			if (!MidiBuffer) return;
			(void)controls2midi(pi, NULL);
			for (i = delay = 0; i < PopulationSize; i ++)
				(void)score2midi(sf->pop[i].score, pi, &delay);
			lastTempo = pi->tempo;
		} break;
		case WIDgedit: {
			SBgedit	*sg = FindGedit(win); if (!sg) return;
			pi = &sg->b;
			length += controls2midi(pi, NULL);
			delay = 0;
			length += score2midi(sg->ind.score, pi, &delay);
			MidiBuffer = (unsigned char *)NewPtr(length + 4);
			if (!MidiBuffer) return;
			(void)controls2midi(pi, NULL);
			delay = 0;
			(void)score2midi(sg->ind.score, pi, &delay);
			lastTempo = pi->tempo;
		} break;
		case WIDIntegrator: {
			integScorePtr	scorep;
			short	nsecs;
			BarInfo	piTrace;
			SBIntegrator *si = FindIntegrator(win);
			if (!si) return;
			nsecs = si->nSectionsH * si->nSectionsW;
			set_default_pi(&piTrace);
			HLock((Handle)si->scoreHandle);
			scorep = *(si->scoreHandle);
			for (i = delay = 0; i < nsecs; i ++) if (ItgOnP(scorep,i)) {
				length += controls2midi(&scorep[i].b, &piTrace);
				length += score2midi(scorep[i].i.score, &scorep[i].b, &delay);
			}
			MidiBuffer = (unsigned char *)NewPtr(length + 4);
			if (!MidiBuffer) return;
			set_default_pi(&piTrace);
			for (i = delay = 0; i < nsecs; i ++) if (ItgOnP(scorep,i)) {
				(void)controls2midi(&scorep[i].b, &piTrace);
				(void)score2midi(scorep[i].i.score, &scorep[i].b, &delay);
				lastTempo = scorep[i].b.tempo;
			}
			HUnlock((Handle)si->scoreHandle);
		}
	}
	(void)put_delay_time(beat2time(delay)
		+ lastTempo * smfInfo->pause * MIDITimeBase / 2400);
	PutByte(0xff); PutByte(0x2f); PutByte(0);	/* End of track */
	length = MidiBufIdx - 4;
	BlockMove(&length, MidiBuffer, 4);
	k = 4;
	if (copyRight[0]) {
		MidiBuffer[k ++] = '\0'; MidiBuffer[k ++] = 0xff;	/* Copyright */
		MidiBuffer[k ++] = 0x02;
		BlockMove(copyRight, MidiBuffer+k, copyRight[0]+1);
		k += copyRight[0] + 1;
	}
	MidiBuffer[k ++] = '\0'; MidiBuffer[k ++] = 0xff;	/* Sequence name */
	MidiBuffer[k ++] = 0x03;
	BlockMove(seqName, MidiBuffer+k, seqName[0]+1);
	if (dateStr[0]) {
		BlockMove(dateStr+1, MidiBuffer+k+seqName[0]+1, dateStr[0]);
		MidiBuffer[k] += dateStr[0];
	}
	k += MidiBuffer[k] + 1;
	BlockMove(timeMark, MidiBuffer+k, 8); k += 8;		/* Time signature */
	MidiBuffer[k ++] = '\0'; MidiBuffer[k ++] = 0xff;	/* Key signature */
	MidiBuffer[k ++] = 0x59; MidiBuffer[k ++] = 0x02;
	MidiBuffer[k ++] = keyScale - KeyInitVal;
	MidiBuffer[k ++] = '\0';
	err = FSWrite(ref, &MidiBufIdx, MidiBuffer);
	DisposePtr((Ptr)MidiBuffer);
	MidiBuffer = NULL;
}
