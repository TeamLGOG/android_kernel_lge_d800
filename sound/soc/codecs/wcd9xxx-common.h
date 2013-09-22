/* Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef WCD9XXX_CODEC_COMMON

#define WCD9XXX_CODEC_COMMON

#include "wcd9xxx-resmgr.h"

#define WCD9XXX_CLSH_REQ_ENABLE true
#define WCD9XXX_CLSH_REQ_DISABLE false

#define WCD9XXX_CLSH_EVENT_PRE_DAC 0x01
#define WCD9XXX_CLSH_EVENT_POST_PA 0x02

/*                                        
                                                  
                  
                       
                        
                      
                         
 */
#define	WCD9XXX_CLSH_STATE_IDLE 0x00
#define	WCD9XXX_CLSH_STATE_EAR (0x01 << 0)
#define	WCD9XXX_CLSH_STATE_HPHL (0x01 << 1)
#define	WCD9XXX_CLSH_STATE_HPHR (0x01 << 2)
#define	WCD9XXX_CLSH_STATE_LO (0x01 << 3)
#define NUM_CLSH_STATES ((0x01 << 4) - 1)

/*                                                                */
#define WCD9XXX_CLSH_STATE_HPH_ST (WCD9XXX_CLSH_STATE_HPHL | \
						WCD9XXX_CLSH_STATE_HPHR)


struct wcd9xxx_reg_mask_val {
	u16	reg;
	u8	mask;
	u8	val;
};

/*                                                  */
struct wcd9xxx_clsh_cdc_data {
	u8 state;
	int buck_mv;
	struct wcd9xxx_resmgr *resmgr;
};

struct wcd9xxx_anc_header {
	u32 reserved[3];
	u32 num_anc_slots;
};

enum wcd9xxx_buck_volt {
	WCD9XXX_CDC_BUCK_UNSUPPORTED = 0,
	WCD9XXX_CDC_BUCK_MV_1P8 = 1800000,
	WCD9XXX_CDC_BUCK_MV_2P15 = 2150000,
};

extern void wcd9xxx_clsh_fsm(struct snd_soc_codec *codec,
		struct wcd9xxx_clsh_cdc_data *cdc_clsh_d,
		u8 req_state, bool req_type, u8 clsh_event);

extern void wcd9xxx_clsh_init(struct wcd9xxx_clsh_cdc_data *clsh,
			      struct wcd9xxx_resmgr *resmgr);

#endif
