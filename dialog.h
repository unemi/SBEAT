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


#ifndef  RESFILE
/* Dialog item IDs */
/* Player Dialog */
#define	NSTTicLabels	5
enum {
	kTabID = 1,
	SFGroup, SFRepeat, SFPlayOnce,
	STPane, STEdText, STSlider, STApply, STRevert, STStText,
	STTicLabel = (STPane+7),
	SKPane = 17, SKEPane, SKSlider, SKPict, SKPopMenu, SKMenuTitle,
	SKECheckBox = (SKPane+7)
};
#define	KeyPictID	160
#define	ETScaleMask	0x40
#define	M8ScaleMask	0x3f

/* Protection dialog */
#define	ProtectCBoxID	(NTotalParts+4)

/* Gene edit dialog */
#define	nGEtitleText	3
enum {
	GEmenuID = 1,
	GEplayID, GEstopID, GEsaveID,
	GErhythmTitleText,
	GEmelodyTitleText,
	GEvelocityTitleText,
	GEmelodyID,
	GErhythmID = (GEmelodyID+NShortBeats),
	GEvelocityText = (GErhythmID+NShortBeats),
	GEvelocitySlider = (GEvelocityText+NShortBeats)
};
typedef struct {
	char	genePart[NTotalParts];
	char	unitBeat[NTotalParts];
	char	iteration[NTotalParts];
	char	playOn[NTotalParts];
	long	instrument[NNoteCh];
	long	drumsInst[NDsPercParts];
	char	octave[NNoteCh-1];
	char	control[NNoteCh];
	char	keyNote, tempo, keyScale, ETScale;
}	BarInfoFlags;
#define	bInfoUniq(x)	(BarInfoUniq.x != -1)
#define	bInfoVary(x)	(BarInfoUniq.x == -1)
#define	setBInfoVary(x)	BarInfoUniq.x = -1
#define	setBInfoNotSetYet(x)	BarInfoUniq.x = -2

enum {
	IcnSpeakerUK = 129,
	IcnSpeakerOff, IcnSpeakerOn,
	IcnProtectUK, IcnProtectOff, IcnProtectOn,
	IcnEyeHalf, IcnEyeOpen, IcnEyeClosed,
	IcnPartOpen, IcnPartClosed,
	IcnSmallSpeaker, IcnSmallEye, IcnSmallIndicator,
	IcnRhythmCont, IcnRhythmRest, IcnRhythmNote,
	IcnSixteenth = IcnRhythmNote, IcnEighth, IcnQuarter,
	IcnColumnOpen, IcnColumnClosed
};
#endif

/* Definition of Small font size */
#define	SmallFontSize9	9
#define	SmallFontSizeX	10
#define	ButtonFontSizeX	12
