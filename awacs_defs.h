
/*********************************************************/
/* This file was written by someone, somewhere, sometime */
/* And is released into the Public Domain                */
/*********************************************************/

#ifndef _AWACS_DEFS_H_
#define _AWACS_DEFS_H_

/**********************/
/* AWACs DMA Channels */
/**********************/

#define AUD_DMA_IN			((unsigned *) 0x09)
#define AUD_DMA_OUT			((unsigned *) 0x08)

/********************************/
/* AWACs DMA Register Addresses */
/********************************/

#define AUD_OUT_DMAREG		((unsigned *) 0xf3008800)
#define AUD_OUT_CHCONT		((unsigned *) 0xf3008800)
#define AUD_OUT_CHSTAT		((unsigned *) 0xf3008804)
#define AUD_OUT_CMDPTR		((unsigned *) 0xf300880c)
#define AUD_OUT_INTRSEL		((unsigned *) 0xf3008810)
#define AUD_OUT_BRSEL		((unsigned *) 0xf3008814)
#define AUD_OUT_WAITSEL		((unsigned *) 0xf3008818)

#define AUD_IN_DMAREG		((unsigned *) 0xf3008900)
#define AUD_IN_CHCONT		((unsigned *) 0xf3008900)
#define AUD_IN_CHSTAT		((unsigned *) 0xf3008904)
#define AUD_IN_CMDPTR		((unsigned *) 0xf300890c)
#define AUD_IN_INTRSEL		((unsigned *) 0xf3008910)
#define AUD_IN_BRSEL		((unsigned *) 0xf3008914)
#define AUD_IN_WAITSEL		((unsigned *) 0xf3008918)

/**********************************/
/* AWACs Audio Register Addresses */
/**********************************/

#define AUD_CONT			((unsigned *) 0xf3014000)
#define AUD_COD_CONT		((unsigned *) 0xf3014010)
#define AUD_COD_STAT		((unsigned *) 0xf3014020)
#define AUD_CLIP_CNT		((unsigned *) 0xf3014030)
#define AUD_BYTESWAP		((unsigned *) 0xf3014040)

/*******************/
/* Audio Bit Masks */
/*******************/

/* Audio Control Reg Bit Masks */
/* ----- ------- --- --- ----- */
#define MASK_ISFSEL			(0xf)			/* Input SubFrame Select */
#define MASK_OSFSEL			(0xf << 4)		/* Output SubFrame Select */
#define MASK_RATE			(0x7 << 8)		/* Sound Rate */
#define MASK_CNTLERR		(0x1 << 11)		/* Error */
#define MASK_PORTCHG		(0x1 << 12)		/* Port Change */
#define MASK_IEE			(0x1 << 13)		/* Enable Interrupt on Error */
#define MASK_IEPC			(0x1 << 14)		/* Enable Interrupt on Port Change */
#define MASK_SSFSEL			(0x3 << 15)		/* Status SubFrame Select */

/* Audio Codec Control Reg Bit Masks */
/* ----- ----- ------- --- --- ----- */
#define MASK_NEWECMD		(0x1 << 24)		/* Lock: don't write to reg when high */
#define MASK_EMODESEL		(0x3 << 22)		/* Send info out on which frame? */
#define MASK_EXMODEADDR		(0x3ff << 12)	/* Extended Mode Address -- 10 bits (12-21) */
#define MASK_EXMODEDATA		(0xfff)			/* Extended Mode Data -- 12 bits (0-11) */

/* Audio Codec Control Address Values / Masks */
/* ----- ----- ------- ------- ------ - ----- */
#define MASK_ADDR0			(0x0 << 12)		/* Expanded Data Mode Address 0 */
#define MASK_ADDR_MUX		MASK_ADDR0		/* Mux Control */
#define MASK_ADDR_GAIN		MASK_ADDR0
#define MASK_ADDR1			(0x1 << 12)		/* Expanded Data Mode Address 1 */
#define MASK_ADDR_MUTE		MASK_ADDR1
#define MASK_ADDR_RATE		MASK_ADDR1
#define MASK_ADDR2			(0x2 << 12)		/* Expanded Data Mode Address 2 */
#define MASK_ADDR_VOLA		MASK_ADDR2		/* Volume Control A -- Speaker */
#define MASK_ADDR_VOLSPK	MASK_ADDR2
#define MASK_ADDR4			(0x4 << 12)		/* Expanded Data Mode Address 4 */
#define MASK_ADDR_VOLC		MASK_ADDR4		/* Volume Control C -- Headphones */
#define MASK_ADDR_VOLHD		MASK_ADDR4

/* Address 0 Bit Masks & Macros */
/* ------- - --- ----- - ------ */
#define MASK_GAINRIGHT		(0xf)			/* Gain Right Mask */
#define MASK_GAINLEFT		(0xf << 4)		/* Gain Left Mask */
#define MASK_GAINLINE		(0x1 << 8)		/* Change Gain for Line??? */
#define MASK_GAINMIC		(0x0 << 8)		/* Change Gain for Mic??? */
#define MASK_MUX_CD			(0x1 << 9)		/* Select CD in MUX */
#define MASK_MUX_MIC		(0x1 << 10)		/* Select Mic in MUX */
#define MASK_MUX_AUDIN		(0x1 << 11)		/* Select Audio In in MUX */
#define MASK_MUX_LINE		MASK_MUX_AUDIN

#define GAINRIGHT(x)		((x) & MASK_GAINRIGHT)
#define GAINLEFT(x)			(((x) << 4) & MASK_GAINLEFT)

/* Address 1 Bit Masks */
/* ------- - --- ----- */
#define MASK_ADDR1RES1		(0x3)			/* Reserved */
#define MASK_RECALIBRATE	(0x1 << 2)		/* Recalibrate */
#define MASK_SAMPLERATE		(0x7 << 3)		/* Sample Rate: */
#define MASK_LOOPTHRU		(0x1 << 6)		/* Loopthrough Enable */
#define MASK_CMUTE			(0x1 << 7)		/* Output C (Headphone) Mute */
#define MASK_HDMUTE			MASK_CMUTE
#define MASK_ADDR1RES2		(0x1 << 8)		/* Reserved */
#define MASK_AMUTE			(0x1 << 9)		/* Output A (Speaker) Mute */
#define MASK_SPKMUTE		MASK_AMUTE
#define MASK_PAROUT			(0x3 << 10)		/* Parallel Out (???) */

#define SAMPLERATE_48000	(0x0 << 3)		/* 48 kHz */
#define SAMPLERATE_32000	(0x1 << 3)		/* 32 kHz */
#define SAMPLERATE_24000	(0x2 << 3)		/* 24 kHz */
#define SAMPLERATE_19200	(0x3 << 3)		/* 19.2 kHz */
#define SAMPLERATE_16000	(0x4 << 3)		/* 16 kHz */
#define SAMPLERATE_12000	(0x5 << 3)		/* 12 kHz */
#define SAMPLERATE_9600		(0x6 << 3)		/* 9.6 kHz */
#define SAMPLERATE_8000		(0x7 << 3)		/* 8 kHz */

/* Address 2 & 4 Bit Masks & Macros */
/* ------- - - - --- ----- - ------ */
#define MASK_OUTVOLRIGHT	(0xf)			/* Output Right Volume */
#define MASK_ADDR2RES1		(0x2 << 4)		/* Reserved */
#define MASK_ADDR4RES1		MASK_ADDR2RES1
#define MASK_OUTVOLLEFT		(0xf << 6)		/* Output Left Volume */
#define MASK_ADDR2RES2		(0x2 << 10)		/* Reserved */
#define MASK_ADDR4RES2		MASK_ADDR2RES2

#define VOLRIGHT(x)			(((~(x)) & MASK_OUTVOLRIGHT))
#define VOLLEFT(x)			(((~(x)) << 6) & MASK_OUTVOLLEFT)

/* Audio Codec Status Reg Bit Masks */
/* ----- ----- ------ --- --- ----- */
#define MASK_EXTEND			(0x1 << 23)		/* Extend */
#define MASK_VALID			(0x1 << 22)		/* Valid Data? */
#define MASK_OFLEFT			(0x1 << 21)		/* Overflow Left */
#define MASK_OFRIGHT		(0x1 << 20)		/* Overflow Right */
#define MASK_ERRCODE		(0xf << 16)		/* Error Code */
#define MASK_REVISION		(0xf << 12)		/* Revision Number */
#define MASK_MFGID			(0xf << 8)		/* Mfg. ID */
#define MASK_CODSTATRES		(0xf << 4)		/* bits 4 - 7 reserved */
#define MASK_INPPORT		(0xf)			/* Input Port */

/* Clipping Count Reg Bit Masks */
/* -------- ----- --- --- ----- */
#define MASK_CLIPLEFT		(0xff << 7)		/* Clipping Count, Left Channel */
#define MASK_CLIPRIGHT		(0xff)			/* Clipping Count, Right Channel */

/* ChannelStatus Bit Masks */
/* ------------- --- ----- */
#define MASK_CSERR			(0x1 << 7)		/* Error */
#define MASK_EOI			(0x1 << 6)		/* End of Input -- only for Input Channel */
#define MASK_CSUNUSED		(0x1f << 1)		/* bits 1-5 not used */
#define MASK_WAIT			(0x1)			/* Wait */

/* Various Rates */
/* ------- ----- */
#define RATE_48000			(0x0 << 8)		/* 48 kHz */
#define RATE_44100			(0x0 << 8)		/* 44.1 kHz */
#define RATE_32000			(0x1 << 8)		/* 32 kHz */
#define RATE_29400			(0x1 << 8)		/* 29.4 kHz */
#define RATE_24000			(0x2 << 8)		/* 24 kHz */
#define RATE_22050			(0x2 << 8)		/* 22.05 kHz */
#define RATE_19200			(0x3 << 8)		/* 19.2 kHz */
#define RATE_17640			(0x3 << 8)		/* 17.64 kHz */
#define RATE_16000			(0x4 << 8)		/* 16 kHz */
#define RATE_14700			(0x4 << 8)		/* 14.7 kHz */
#define RATE_12000			(0x5 << 8)		/* 12 kHz */
#define RATE_11025			(0x5 << 8)		/* 11.025 kHz */
#define RATE_9600			(0x6 << 8)		/* 9.6 kHz */
#define RATE_8820			(0x6 << 8)		/* 8.82 kHz */
#define RATE_8000			(0x7 << 8)		/* 8 kHz */
#define RATE_7350			(0x7 << 8)		/* 7.35 kHz */

#define RATE_LOW			1				/* HIGH = 48kHz, etc;  LOW = 44.1kHz, etc. */

#endif /* _AWACS_DEFS_H_ */
