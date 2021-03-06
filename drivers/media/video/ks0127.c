/*
 * Video Capture Driver (Video for Linux 1/2)
 * for the Matrox Marvel G200,G400 and Rainbow Runner-G series
 *
 * This module is an interface to the KS0127 video decoder chip.
 *
 * Copyright (C) 1999  Ryan Drake <stiletto@mediaone.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *****************************************************************************
 *
 * Modified and extended by
 *	Mike Bernson <mike@mlb.org>
 *	Gerard v.d. Horst
 *	Leon van Stuivenberg <l.vanstuivenberg@chello.nl>
 *	Gernot Ziegler <gz@lysator.liu.se>
 *
 * Version History:
 * V1.0 Ryan Drake	   Initial version by Ryan Drake
 * V1.1 Gerard v.d. Horst  Added some debugoutput, reset the video-standard
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/videodev2.h>
#include <linux/slab.h>
#include <media/v4l2-device.h>
#include <media/v4l2-chip-ident.h>
#include "ks0127.h"

MODULE_DESCRIPTION("KS0127 video decoder driver");
MODULE_AUTHOR("Ryan Drake");
MODULE_LICENSE("GPL");

#define I2C_KS0127_ADDON   0xD8
#define I2C_KS0127_ONBOARD 0xDA


#define KS_STAT     0x00
#define KS_CMDA     0x01
#define KS_CMDB     0x02
#define KS_CMDC     0x03
#define KS_CMDD     0x04
#define KS_HAVB     0x05
#define KS_HAVE     0x06
#define KS_HS1B     0x07
#define KS_HS1E     0x08
#define KS_HS2B     0x09
#define KS_HS2E     0x0a
#define KS_AGC      0x0b
#define KS_HXTRA    0x0c
#define KS_CDEM     0x0d
#define KS_PORTAB   0x0e
#define KS_LUMA     0x0f
#define KS_CON      0x10
#define KS_BRT      0x11
#define KS_CHROMA   0x12
#define KS_CHROMB   0x13
#define KS_DEMOD    0x14
#define KS_SAT      0x15
#define KS_HUE      0x16
#define KS_VERTIA   0x17
#define KS_VERTIB   0x18
#define KS_VERTIC   0x19
#define KS_HSCLL    0x1a
#define KS_HSCLH    0x1b
#define KS_VSCLL    0x1c
#define KS_VSCLH    0x1d
#define KS_OFMTA    0x1e
#define KS_OFMTB    0x1f
#define KS_VBICTL   0x20
#define KS_CCDAT2   0x21
#define KS_CCDAT1   0x22
#define KS_VBIL30   0x23
#define KS_VBIL74   0x24
#define KS_VBIL118  0x25
#define KS_VBIL1512 0x26
#define KS_TTFRAM   0x27
#define KS_TESTA    0x28
#define KS_UVOFFH   0x29
#define KS_UVOFFL   0x2a
#define KS_UGAIN    0x2b
#define KS_VGAIN    0x2c
#define KS_VAVB     0x2d
#define KS_VAVE     0x2e
#define KS_CTRACK   0x2f
#define KS_POLCTL   0x30
#define KS_REFCOD   0x31
#define KS_INVALY   0x32
#define KS_INVALU   0x33
#define KS_INVALV   0x34
#define KS_UNUSEY   0x35
#define KS_UNUSEU   0x36
#define KS_UNUSEV   0x37
#define KS_USRSAV   0x38
#define KS_USREAV   0x39
#define KS_SHS1A    0x3a
#define KS_SHS1B    0x3b
#define KS_SHS1C    0x3c
#define KS_CMDE     0x3d
#define KS_VSDEL    0x3e
#define KS_CMDF     0x3f
#define KS_GAMMA0   0x40
#define KS_GAMMA1   0x41
#define KS_GAMMA2   0x42
#define KS_GAMMA3   0x43
#define KS_GAMMA4   0x44
#define KS_GAMMA5   0x45
#define KS_GAMMA6   0x46
#define KS_GAMMA7   0x47
#define KS_GAMMA8   0x48
#define KS_GAMMA9   0x49
#define KS_GAMMA10  0x4a
#define KS_GAMMA11  0x4b
#define KS_GAMMA12  0x4c
#define KS_GAMMA13  0x4d
#define KS_GAMMA14  0x4e
#define KS_GAMMA15  0x4f
#define KS_GAMMA16  0x50
#define KS_GAMMA17  0x51
#define KS_GAMMA18  0x52
#define KS_GAMMA19  0x53
#define KS_GAMMA20  0x54
#define KS_GAMMA21  0x55
#define KS_GAMMA22  0x56
#define KS_GAMMA23  0x57
#define KS_GAMMA24  0x58
#define KS_GAMMA25  0x59
#define KS_GAMMA26  0x5a
#define KS_GAMMA27  0x5b
#define KS_GAMMA28  0x5c
#define KS_GAMMA29  0x5d
#define KS_GAMMA30  0x5e
#define KS_GAMMA31  0x5f
#define KS_GAMMAD0  0x60
#define KS_GAMMAD1  0x61
#define KS_GAMMAD2  0x62
#define KS_GAMMAD3  0x63
#define KS_GAMMAD4  0x64
#define KS_GAMMAD5  0x65
#define KS_GAMMAD6  0x66
#define KS_GAMMAD7  0x67
#define KS_GAMMAD8  0x68
#define KS_GAMMAD9  0x69
#define KS_GAMMAD10 0x6a
#define KS_GAMMAD11 0x6b
#define KS_GAMMAD12 0x6c
#define KS_GAMMAD13 0x6d
#define KS_GAMMAD14 0x6e
#define KS_GAMMAD15 0x6f
#define KS_GAMMAD16 0x70
#define KS_GAMMAD17 0x71
#define KS_GAMMAD18 0x72
#define KS_GAMMAD19 0x73
#define KS_GAMMAD20 0x74
#define KS_GAMMAD21 0x75
#define KS_GAMMAD22 0x76
#define KS_GAMMAD23 0x77
#define KS_GAMMAD24 0x78
#define KS_GAMMAD25 0x79
#define KS_GAMMAD26 0x7a
#define KS_GAMMAD27 0x7b
#define KS_GAMMAD28 0x7c
#define KS_GAMMAD29 0x7d
#define KS_GAMMAD30 0x7e
#define KS_GAMMAD31 0x7f



struct adjust {
	int	contrast;
	int	bright;
	int	hue;
	int	ugain;
	int	vgain;
};

struct ks0127 {
	struct v4l2_subdev sd;
	v4l2_std_id	norm;
	int		ident;
	u8 		regs[256];
};

static inline struct ks0127 *to_ks0127(struct v4l2_subdev *sd)
{
	return container_of(sd, struct ks0127, sd);
}


static int debug; 

module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "Debug output");

static u8 reg_defaults[64];

static void init_reg_defaults(void)
{
	static int initialized;
	u8 *table = reg_defaults;

	if (initialized)
		return;
	initialized = 1;

	table[KS_CMDA]     = 0x2c;  
	table[KS_CMDB]     = 0x12;  
	table[KS_CMDC]     = 0x00;  
	
	table[KS_CMDD]     = 0x01;
	table[KS_HAVB]     = 0x00;  
	table[KS_HAVE]     = 0x00;  
	table[KS_HS1B]     = 0x10;  
	table[KS_HS1E]     = 0x00;  
	table[KS_HS2B]     = 0x00;  
	table[KS_HS2E]     = 0x00;  
	table[KS_AGC]      = 0x53;  
	table[KS_HXTRA]    = 0x00;  
	table[KS_CDEM]     = 0x00;  
	table[KS_PORTAB]   = 0x0f;  
	table[KS_LUMA]     = 0x01;  
	table[KS_CON]      = 0x00;  
	table[KS_BRT]      = 0x00;  
	table[KS_CHROMA]   = 0x2a;  
	table[KS_CHROMB]   = 0x90;  
	table[KS_DEMOD]    = 0x00;  
	table[KS_SAT]      = 0x00;  
	table[KS_HUE]      = 0x00;  
	table[KS_VERTIA]   = 0x00;  
	
	table[KS_VERTIB]   = 0x12;
	table[KS_VERTIC]   = 0x0b;  
	table[KS_HSCLL]    = 0x00;  
	table[KS_HSCLH]    = 0x00;  
	table[KS_VSCLL]    = 0x00;  
	table[KS_VSCLH]    = 0x00;  
	
	table[KS_OFMTA]    = 0x30;
	table[KS_OFMTB]    = 0x00;  
	
	table[KS_VBICTL]   = 0x5d;
	table[KS_CCDAT2]   = 0x00;  
	table[KS_CCDAT1]   = 0x00;  
	table[KS_VBIL30]   = 0xa8;  
	table[KS_VBIL74]   = 0xaa;  
	table[KS_VBIL118]  = 0x2a;  
	table[KS_VBIL1512] = 0x00;  
	table[KS_TTFRAM]   = 0x00;  
	table[KS_TESTA]    = 0x00;  /* test register, shouldn't be written */
	table[KS_UVOFFH]   = 0x00;  
	table[KS_UVOFFL]   = 0x00;  
	table[KS_UGAIN]    = 0x00;  
	table[KS_VGAIN]    = 0x00;  
	table[KS_VAVB]     = 0x07;  
	table[KS_VAVE]     = 0x00;  
	table[KS_CTRACK]   = 0x00;  
	table[KS_POLCTL]   = 0x41;  
	table[KS_REFCOD]   = 0x80;  
	table[KS_INVALY]   = 0x10;  
	table[KS_INVALU]   = 0x80;  
	table[KS_INVALV]   = 0x80;  
	table[KS_UNUSEY]   = 0x10;  
	table[KS_UNUSEU]   = 0x80;  
	table[KS_UNUSEV]   = 0x80;  
	table[KS_USRSAV]   = 0x00;  
	table[KS_USREAV]   = 0x00;  
	table[KS_SHS1A]    = 0x00;  
	
	table[KS_SHS1B]    = 0x80;
	table[KS_SHS1C]    = 0x00;  
	table[KS_CMDE]     = 0x00;  
	table[KS_VSDEL]    = 0x00;  
	
	
	table[KS_CMDF]     = 0x02;
}




static u8 ks0127_read(struct v4l2_subdev *sd, u8 reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	char val = 0;
	struct i2c_msg msgs[] = {
		{ client->addr, 0, sizeof(reg), &reg },
		{ client->addr, I2C_M_RD | I2C_M_NO_RD_ACK, sizeof(val), &val }
	};
	int ret;

	ret = i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs));
	if (ret != ARRAY_SIZE(msgs))
		v4l2_dbg(1, debug, sd, "read error\n");

	return val;
}


static void ks0127_write(struct v4l2_subdev *sd, u8 reg, u8 val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ks0127 *ks = to_ks0127(sd);
	char msg[] = { reg, val };

	if (i2c_master_send(client, msg, sizeof(msg)) != sizeof(msg))
		v4l2_dbg(1, debug, sd, "write error\n");

	ks->regs[reg] = val;
}


static void ks0127_and_or(struct v4l2_subdev *sd, u8 reg, u8 and_v, u8 or_v)
{
	struct ks0127 *ks = to_ks0127(sd);

	u8 val = ks->regs[reg];
	val = (val & and_v) | or_v;
	ks0127_write(sd, reg, val);
}



static void ks0127_init(struct v4l2_subdev *sd)
{
	struct ks0127 *ks = to_ks0127(sd);
	u8 *table = reg_defaults;
	int i;

	ks->ident = V4L2_IDENT_KS0127;

	v4l2_dbg(1, debug, sd, "reset\n");
	msleep(1);

	
	

	for (i = 1; i < 33; i++)
		ks0127_write(sd, i, table[i]);

	for (i = 35; i < 40; i++)
		ks0127_write(sd, i, table[i]);

	for (i = 41; i < 56; i++)
		ks0127_write(sd, i, table[i]);

	for (i = 58; i < 64; i++)
		ks0127_write(sd, i, table[i]);


	if ((ks0127_read(sd, KS_STAT) & 0x80) == 0) {
		ks->ident = V4L2_IDENT_KS0122S;
		v4l2_dbg(1, debug, sd, "ks0122s found\n");
		return;
	}

	switch (ks0127_read(sd, KS_CMDE) & 0x0f) {
	case 0:
		v4l2_dbg(1, debug, sd, "ks0127 found\n");
		break;

	case 9:
		ks->ident = V4L2_IDENT_KS0127B;
		v4l2_dbg(1, debug, sd, "ks0127B Revision A found\n");
		break;

	default:
		v4l2_dbg(1, debug, sd, "unknown revision\n");
		break;
	}
}

static int ks0127_s_routing(struct v4l2_subdev *sd,
			    u32 input, u32 output, u32 config)
{
	struct ks0127 *ks = to_ks0127(sd);

	switch (input) {
	case KS_INPUT_COMPOSITE_1:
	case KS_INPUT_COMPOSITE_2:
	case KS_INPUT_COMPOSITE_3:
	case KS_INPUT_COMPOSITE_4:
	case KS_INPUT_COMPOSITE_5:
	case KS_INPUT_COMPOSITE_6:
		v4l2_dbg(1, debug, sd,
			"s_routing %d: Composite\n", input);
		
		ks0127_and_or(sd, KS_CMDA,   0xfc, 0x00);
		
		ks0127_and_or(sd, KS_CMDA,   ~0x40, 0x00);
		
		ks0127_and_or(sd, KS_CMDB,   0xb0, input);
		
		ks0127_and_or(sd, KS_CMDC,   0x70, 0x0a);
		
		ks0127_and_or(sd, KS_CMDD,   0x03, 0x00);
		
		ks0127_and_or(sd, KS_CTRACK, 0xcf, 0x00);
		
		ks0127_and_or(sd, KS_LUMA,   0x00,
			       (reg_defaults[KS_LUMA])|0x0c);
		
		ks0127_and_or(sd, KS_VERTIA, 0x08, 0x81);
		
		ks0127_and_or(sd, KS_VERTIC, 0x0f, 0x90);

		
		ks0127_and_or(sd, KS_CHROMB, 0x0f, 0x90);

		ks0127_write(sd, KS_UGAIN, reg_defaults[KS_UGAIN]);
		ks0127_write(sd, KS_VGAIN, reg_defaults[KS_VGAIN]);
		ks0127_write(sd, KS_UVOFFH, reg_defaults[KS_UVOFFH]);
		ks0127_write(sd, KS_UVOFFL, reg_defaults[KS_UVOFFL]);
		break;

	case KS_INPUT_SVIDEO_1:
	case KS_INPUT_SVIDEO_2:
	case KS_INPUT_SVIDEO_3:
		v4l2_dbg(1, debug, sd,
			"s_routing %d: S-Video\n", input);
		
		ks0127_and_or(sd, KS_CMDA,   0xfc, 0x00);
		
		ks0127_and_or(sd, KS_CMDA,   ~0x40, 0x00);
		
		ks0127_and_or(sd, KS_CMDB,   0xb0, input);
		
		ks0127_and_or(sd, KS_CMDC,   0x70, 0x0a);
		
		ks0127_and_or(sd, KS_CMDD,   0x03, 0x00);
		
		ks0127_and_or(sd, KS_CTRACK, 0xcf, 0x00);
		ks0127_and_or(sd, KS_LUMA, 0x00,
			       reg_defaults[KS_LUMA]);
		
		ks0127_and_or(sd, KS_VERTIA, 0x08,
			       (reg_defaults[KS_VERTIA]&0xf0)|0x01);
		ks0127_and_or(sd, KS_VERTIC, 0x0f,
			       reg_defaults[KS_VERTIC]&0xf0);

		ks0127_and_or(sd, KS_CHROMB, 0x0f,
			       reg_defaults[KS_CHROMB]&0xf0);

		ks0127_write(sd, KS_UGAIN, reg_defaults[KS_UGAIN]);
		ks0127_write(sd, KS_VGAIN, reg_defaults[KS_VGAIN]);
		ks0127_write(sd, KS_UVOFFH, reg_defaults[KS_UVOFFH]);
		ks0127_write(sd, KS_UVOFFL, reg_defaults[KS_UVOFFL]);
		break;

	case KS_INPUT_YUV656:
		v4l2_dbg(1, debug, sd, "s_routing 15: YUV656\n");
		if (ks->norm & V4L2_STD_525_60)
			
			ks0127_and_or(sd, KS_CMDA,   0xfc, 0x03);
		else
			
			ks0127_and_or(sd, KS_CMDA,   0xfc, 0x02);

		ks0127_and_or(sd, KS_CMDA,   0xff, 0x40); 
		
		ks0127_and_or(sd, KS_CMDB,   0xb0, (input | 0x40));
		
		
		ks0127_and_or(sd, KS_CMDC,   0x70, 0x87);
		
		ks0127_and_or(sd, KS_CMDD,   0x03, 0x08);
		
		ks0127_and_or(sd, KS_CTRACK, 0xcf, 0x30);
		
		ks0127_and_or(sd, KS_LUMA,   0x00, 0x71);
		ks0127_and_or(sd, KS_VERTIC, 0x0f,
			       reg_defaults[KS_VERTIC]&0xf0);

		
		ks0127_and_or(sd, KS_VERTIA, 0x08, 0x81);

		ks0127_and_or(sd, KS_CHROMB, 0x0f,
			       reg_defaults[KS_CHROMB]&0xf0);

		ks0127_and_or(sd, KS_CON, 0x00, 0x00);
		ks0127_and_or(sd, KS_BRT, 0x00, 32);	
			
		ks0127_and_or(sd, KS_SAT, 0x00, 0xe8);
		ks0127_and_or(sd, KS_HUE, 0x00, 0);

		ks0127_and_or(sd, KS_UGAIN, 0x00, 238);
		ks0127_and_or(sd, KS_VGAIN, 0x00, 0x00);

		
		ks0127_and_or(sd, KS_UVOFFH, 0x00, 0x4f);
		ks0127_and_or(sd, KS_UVOFFL, 0x00, 0x00);
		break;

	default:
		v4l2_dbg(1, debug, sd,
			"s_routing: Unknown input %d\n", input);
		break;
	}

	
	
	ks0127_write(sd, KS_DEMOD, reg_defaults[KS_DEMOD]);
	return 0;
}

static int ks0127_s_std(struct v4l2_subdev *sd, v4l2_std_id std)
{
	struct ks0127 *ks = to_ks0127(sd);

	
	ks0127_and_or(sd, KS_DEMOD, 0xf0, 0x00);

	ks->norm = std;
	if (std & V4L2_STD_NTSC) {
		v4l2_dbg(1, debug, sd,
			"s_std: NTSC_M\n");
		ks0127_and_or(sd, KS_CHROMA, 0x9f, 0x20);
	} else if (std & V4L2_STD_PAL_N) {
		v4l2_dbg(1, debug, sd,
			"s_std: NTSC_N (fixme)\n");
		ks0127_and_or(sd, KS_CHROMA, 0x9f, 0x40);
	} else if (std & V4L2_STD_PAL) {
		v4l2_dbg(1, debug, sd,
			"s_std: PAL_N\n");
		ks0127_and_or(sd, KS_CHROMA, 0x9f, 0x20);
	} else if (std & V4L2_STD_PAL_M) {
		v4l2_dbg(1, debug, sd,
			"s_std: PAL_M (fixme)\n");
		ks0127_and_or(sd, KS_CHROMA, 0x9f, 0x40);
	} else if (std & V4L2_STD_SECAM) {
		v4l2_dbg(1, debug, sd,
			"s_std: SECAM\n");

		
		ks0127_and_or(sd, KS_CHROMA, 0xdf, 0x20);
		ks0127_and_or(sd, KS_DEMOD, 0xf0, 0x00);
		schedule_timeout_interruptible(HZ/10+1);

		
		if (!(ks0127_read(sd, KS_DEMOD) & 0x40))
			
			ks0127_and_or(sd, KS_DEMOD, 0xf0, 0x0f);
	} else {
		v4l2_dbg(1, debug, sd, "s_std: Unknown norm %llx\n",
			       (unsigned long long)std);
	}
	return 0;
}

static int ks0127_s_stream(struct v4l2_subdev *sd, int enable)
{
	v4l2_dbg(1, debug, sd, "s_stream(%d)\n", enable);
	if (enable) {
		
		ks0127_and_or(sd, KS_OFMTA, 0xcf, 0x30);
		
		ks0127_and_or(sd, KS_CDEM, 0x7f, 0x00);
	} else {
		
		ks0127_and_or(sd, KS_OFMTA, 0xcf, 0x00);
		
		ks0127_and_or(sd, KS_CDEM, 0x7f, 0x80);
	}
	return 0;
}

static int ks0127_status(struct v4l2_subdev *sd, u32 *pstatus, v4l2_std_id *pstd)
{
	int stat = V4L2_IN_ST_NO_SIGNAL;
	u8 status;
	v4l2_std_id std = V4L2_STD_ALL;

	status = ks0127_read(sd, KS_STAT);
	if (!(status & 0x20))		 
		stat = 0;
	if (!(status & 0x01))		      
		stat |= V4L2_IN_ST_NO_COLOR;
	if ((status & 0x08))		   
		std = V4L2_STD_PAL;
	else
		std = V4L2_STD_NTSC;
	if (pstd)
		*pstd = std;
	if (pstatus)
		*pstatus = stat;
	return 0;
}

static int ks0127_querystd(struct v4l2_subdev *sd, v4l2_std_id *std)
{
	v4l2_dbg(1, debug, sd, "querystd\n");
	return ks0127_status(sd, NULL, std);
}

static int ks0127_g_input_status(struct v4l2_subdev *sd, u32 *status)
{
	v4l2_dbg(1, debug, sd, "g_input_status\n");
	return ks0127_status(sd, status, NULL);
}

static int ks0127_g_chip_ident(struct v4l2_subdev *sd, struct v4l2_dbg_chip_ident *chip)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct ks0127 *ks = to_ks0127(sd);

	return v4l2_chip_ident_i2c_client(client, chip, ks->ident, 0);
}


static const struct v4l2_subdev_core_ops ks0127_core_ops = {
	.g_chip_ident = ks0127_g_chip_ident,
	.s_std = ks0127_s_std,
};

static const struct v4l2_subdev_video_ops ks0127_video_ops = {
	.s_routing = ks0127_s_routing,
	.s_stream = ks0127_s_stream,
	.querystd = ks0127_querystd,
	.g_input_status = ks0127_g_input_status,
};

static const struct v4l2_subdev_ops ks0127_ops = {
	.core = &ks0127_core_ops,
	.video = &ks0127_video_ops,
};



static int ks0127_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct ks0127 *ks;
	struct v4l2_subdev *sd;

	v4l_info(client, "%s chip found @ 0x%x (%s)\n",
		client->addr == (I2C_KS0127_ADDON >> 1) ? "addon" : "on-board",
		client->addr << 1, client->adapter->name);

	ks = kzalloc(sizeof(*ks), GFP_KERNEL);
	if (ks == NULL)
		return -ENOMEM;
	sd = &ks->sd;
	v4l2_i2c_subdev_init(sd, client, &ks0127_ops);

	
	init_reg_defaults();
	ks0127_write(sd, KS_CMDA, 0x2c);
	mdelay(10);

	
	ks0127_init(sd);
	return 0;
}

static int ks0127_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);

	v4l2_device_unregister_subdev(sd);
	ks0127_write(sd, KS_OFMTA, 0x20); 
	ks0127_write(sd, KS_CMDA, 0x2c | 0x80); 
	kfree(to_ks0127(sd));
	return 0;
}

static const struct i2c_device_id ks0127_id[] = {
	{ "ks0127", 0 },
	{ "ks0127b", 0 },
	{ "ks0122s", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ks0127_id);

static struct i2c_driver ks0127_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "ks0127",
	},
	.probe		= ks0127_probe,
	.remove		= ks0127_remove,
	.id_table	= ks0127_id,
};

module_i2c_driver(ks0127_driver);
