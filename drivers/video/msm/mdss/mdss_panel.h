/* Copyright (c) 2008-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef MDSS_PANEL_H
#define MDSS_PANEL_H

#include <linux/platform_device.h>
#include <linux/types.h>
#ifdef CONFIG_MACH_LGE
#define QCT_UNDERRUN_PATCH
#endif

/*               */
struct panel_id {
	u16 id;
	u16 type;
};

/*                 */
#define NO_PANEL		0xffff	/*          */
#define MDDI_PANEL		1	/*      */
#define EBI2_PANEL		2	/*      */
#define LCDC_PANEL		3	/*                    */
#define EXT_MDDI_PANEL		4	/*          */
#define TV_PANEL		5	/*    */
#define HDMI_PANEL		6	/*         */
#define DTV_PANEL		7	/*     */
#define MIPI_VIDEO_PANEL	8	/*      */
#define MIPI_CMD_PANEL		9	/*      */
#define WRITEBACK_PANEL		10	/*              */
#define LVDS_PANEL		11	/*      */
#define EDP_PANEL		12	/*      */

/*             */
enum {
	DISPLAY_LCD = 0,	/*                 */
	DISPLAY_LCDC,		/*      */
	DISPLAY_TV,		/*        */
	DISPLAY_EXT_MDDI,	/*               */
	DISPLAY_WRITEBACK,
};

/*                       */
enum {
	DISPLAY_1 = 0,		/*                          */
	DISPLAY_2,		/*                           */
	DISPLAY_3,              /*                                    */
	MAX_PHYS_TARGET_NUM,
};

/* 
                                                                 
  
                                                                 
                                                                   
                    
                                                           
                                                                       
                                               
                                                                          
                                          
                                                        
                                                            
                                                          
                                                                            
                                                  
                                           
                                                      
                                                 
                                               
                                               
                                                                            
                                                     
                                               
                                                                             
                                                     
                                                 
                                  
                                                                             
                                                   
                               
                                                  
                      
                     
                                                                              
 */
enum mdss_intf_events {
	MDSS_EVENT_RESET = 1,
	MDSS_EVENT_UNBLANK,
	MDSS_EVENT_PANEL_ON,
	MDSS_EVENT_BLANK,
	MDSS_EVENT_PANEL_OFF,
	MDSS_EVENT_CLOSE,
	MDSS_EVENT_SUSPEND,
	MDSS_EVENT_RESUME,
	MDSS_EVENT_CHECK_PARAMS,
	MDSS_EVENT_CONT_SPLASH_BEGIN,
	MDSS_EVENT_CONT_SPLASH_FINISH,
#ifdef CONFIG_OLED_SUPPORT
	MDSS_EVENT_FIRST_FRAME_UPDATE,
#endif
	MDSS_EVENT_FB_REGISTERED,
	MDSS_EVENT_PANEL_CLK_CTRL,
	MDSS_EVENT_DSI_CMDLIST_KOFF,
};

struct lcd_panel_info {
	u32 h_back_porch;
	u32 h_front_porch;
	u32 h_pulse_width;
	u32 v_back_porch;
	u32 v_front_porch;
	u32 v_pulse_width;
	u32 border_clr;
	u32 underflow_clr;
	u32 hsync_skew;
	/*           */
	u32 xres_pad;
	/*            */
	u32 yres_pad;
#ifdef CONFIG_OLED_SUPPORT
	/*              */
	u32 xres_margin;
	/*               */
	u32 yres_margin;
#endif
};


/*                       */
struct mdss_dsi_phy_ctrl {
	uint32_t regulator[7];
	uint32_t timing[12];
	uint32_t ctrl[4];
	uint32_t strength[2];
	char bistCtrl[6];
	uint32_t pll[21];
	char laneCfg[45];
};

struct mipi_panel_info {
	char mode;		/*           */
	char interleave_mode;
	char crc_check;
	char ecc_check;
	char dst_format;	/*                             */
	char data_lane0;
	char data_lane1;
	char data_lane2;
	char data_lane3;
	char dlane_swap;	/*                */
	char rgb_swap;
	char b_sel;
	char g_sel;
	char r_sel;
	char rx_eot_ignore;
	char tx_eot_append;
	char t_clk_post; /*                              */
	char t_clk_pre;  /*                              */
	char vc;	/*                 */
	struct mdss_dsi_phy_ctrl *dsi_phy_db;
	/*            */
	char pulse_mode_hsa_he;
	char hfp_power_stop;
	char hbp_power_stop;
	char hsa_power_stop;
	char eof_bllp_power_stop;
	char bllp_power_stop;
	char traffic_mode;
	char frame_rate;
	/*              */
	char interleave_max;
	char insert_dcs_cmd;
	char wr_mem_continue;
	char wr_mem_start;
	char te_sel;
	char stream;	/*        */
	char mdp_trigger;
	char dma_trigger;
	u32 dsi_pclk_rate;
	/*                                        */
	char no_max_pkt_size;
	/*                                   */
	char force_clk_lane_hs;

	char vsync_enable;
	char hw_vsync_mode;
};

enum lvds_mode {
	LVDS_SINGLE_CHANNEL_MODE,
	LVDS_DUAL_CHANNEL_MODE,
};

struct lvds_panel_info {
	enum lvds_mode channel_mode;
	/*                           */
	char channel_swap;
};

struct fbc_panel_info {
	u32 enabled;
	u32 target_bpp;
	u32 comp_mode;
	u32 qerr_enable;
	u32 cd_bias;
	u32 pat_enable;
	u32 vlc_enable;
	u32 bflc_enable;

	u32 line_x_budget;
	u32 block_x_budget;
	u32 block_budget;

	u32 lossless_mode_thd;
	u32 lossy_mode_thd;
	u32 lossy_rgb_thd;
	u32 lossy_mode_idx;
};

struct mdss_panel_info {
	u32 xres;
	u32 yres;
	u32 bpp;
	u32 type;
	u32 wait_cycle;
	u32 pdest;
	u32 bl_max;
	u32 bl_min;
	u32 fb_num;
	u32 clk_rate;
	u32 clk_min;
	u32 clk_max;
	u32 frame_count;
	u32 is_3d_panel;
	u32 out_format;
	u32 vic; /*                           */
	int bklt_ctrl;	/*                */
	int pwm_pmic_gpio;
	int pwm_lpg_chan;
	int pwm_period;
#ifdef CONFIG_OLED_SUPPORT
       int blmap_size;
       char *blmap;
#endif

	u32 cont_splash_enabled;
	struct ion_handle *splash_ihdl;
	u32 panel_power_on;

	struct lcd_panel_info lcdc;
	struct fbc_panel_info fbc;
	struct mipi_panel_info mipi;
	struct lvds_panel_info lvds;
};

struct mdss_panel_data {
	struct mdss_panel_info panel_info;
	void (*set_backlight) (struct mdss_panel_data *pdata, u32 bl_level);
	unsigned char *mmss_cc_base;

	/* 
                                                          
                                                                   
                                               
                                                         
                                                               
   
                                                                     
                                                                    
                                                                        
                 
  */
	int (*event_handler) (struct mdss_panel_data *pdata, int e, void *arg);

	struct mdss_panel_data *next;
};

#ifdef QCT_UNDERRUN_PATCH
/* 
                                                       
                                                                 
  
                                                                       
                                                                         
 */
static inline int mdss_panel_get_vtotal(struct mdss_panel_info *pinfo)
{
	return pinfo->yres + pinfo->lcdc.v_back_porch +
			pinfo->lcdc.v_front_porch +
			pinfo->lcdc.v_pulse_width;
}
#endif
int mdss_register_panel(struct platform_device *pdev,
	struct mdss_panel_data *pdata);
#endif /*              */
