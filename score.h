#define  RemainGene	0x80
#define	RemainP(x)	((x & 0x80) == 0x80)
#define	PauseP(x)	((x & 0xe0) == 0x60)
#define	EffectPart(x)	((x >> 5) & 0x07)
#define	InhibitRemainBy	8
#define	SoloShiftValue(g)	(((g) & 0x07) % 3)

#define	VelocityMax	64
