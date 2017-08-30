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


#define  RemainGene	0x80
#define	RemainP(x)	((x & 0x80) == 0x80)
#define	PauseP(x)	((x & 0xe0) == 0x60)
#define	EffectPart(x)	((x >> 5) & 0x07)
#define	InhibitRemainBy	8
#define	SoloShiftValue(g)	(((g) & 0x07) % 3)

#define	VelocityMax	64
