/* Copyright (c) 2011-2012, The Linux Foundation. All rights reserved.
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

#ifndef _MSM_H
#define _MSM_H

#ifdef __KERNEL__

/*              */
#include <linux/i2c.h>
#include <linux/videodev2.h>
#include <linux/pm_qos.h>
#include <linux/wakelock.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-device.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-event.h>
#include <media/v4l2-mediabus.h>
#include <media/videobuf2-dma-contig.h>
#include <media/videobuf2-msm-mem.h>
#include <media/msm_isp.h>
#include <mach/camera.h>
#include <mach/iommu.h>
#include <media/msm_isp.h>
#include <linux/msm_ion.h>
#include <linux/iommu.h>
#include <media/msm_gestures.h>

#define MSM_V4L2_DIMENSION_SIZE 96
#define MAX_DEV_NAME_LEN 50

#define ERR_USER_COPY(to) pr_err("%s(%d): copy %s user\n", \
				__func__, __LINE__, ((to) ? "to" : "from"))
#define ERR_COPY_FROM_USER() ERR_USER_COPY(0)
#define ERR_COPY_TO_USER() ERR_USER_COPY(1)

#define COPY_FROM_USER(error, dest, src, size) \
	(error = (copy_from_user(dest, src, size) ? -EFAULT : 0))
#define COPY_TO_USER(error, dest, src, size) \
	(error = (copy_to_user(dest, src, size) ? -EFAULT : 0))

#define MSM_CSIPHY_DRV_NAME "msm_csiphy"
#define MSM_CSID_DRV_NAME "msm_csid"
#define MSM_CSIC_DRV_NAME "msm_csic"
#define MSM_ISPIF_DRV_NAME "msm_ispif"
#define MSM_VFE_DRV_NAME "msm_vfe"
#define MSM_VPE_DRV_NAME "msm_vpe"
#define MSM_GEMINI_DRV_NAME "msm_gemini"
#define MSM_MERCURY_DRV_NAME "msm_mercury"
#define MSM_JPEG_DRV_NAME "msm_jpeg"
#define MSM_I2C_MUX_DRV_NAME "msm_cam_i2c_mux"
#define MSM_IRQ_ROUTER_DRV_NAME "msm_cam_irq_router"
#define MSM_CPP_DRV_NAME "msm_cpp"
#define MSM_CCI_DRV_NAME "msm_cci"

#define MAX_NUM_SENSOR_DEV 3
#define MAX_NUM_CSIPHY_DEV 3
#define MAX_NUM_CSID_DEV 4
#define MAX_NUM_CSIC_DEV 3
#define MAX_NUM_ISPIF_DEV 1
#define MAX_NUM_VFE_DEV 2
#define MAX_NUM_AXI_DEV 2
#define MAX_NUM_VPE_DEV 1
#define MAX_NUM_JPEG_DEV 3
#define MAX_NUM_CPP_DEV 1
#define MAX_NUM_CCI_DEV 1
#define MAX_NUM_FLASH_DEV 4

/*                          */

#define msm_dequeue(queue, member) ({	   \
	unsigned long flags;		  \
	struct msm_device_queue *__q = (queue);	 \
	struct msm_queue_cmd *qcmd = 0;	   \
	spin_lock_irqsave(&__q->lock, flags);	 \
	if (!list_empty(&__q->list)) {		\
		__q->len--;		 \
		qcmd = list_first_entry(&__q->list,   \
		struct msm_queue_cmd, member);  \
		list_del_init(&qcmd->member);	 \
	}			 \
	spin_unlock_irqrestore(&__q->lock, flags);  \
	qcmd;			 \
})

#define msm_queue_drain(queue, member) do {	 \
	unsigned long flags;		  \
	struct msm_device_queue *__q = (queue);	 \
	struct msm_queue_cmd *qcmd;	   \
	spin_lock_irqsave(&__q->lock, flags);	 \
	while (!list_empty(&__q->list)) {	 \
		qcmd = list_first_entry(&__q->list,   \
			struct msm_queue_cmd, member);	\
			list_del_init(&qcmd->member);	 \
			kfree(qcmd->command);		\
			free_qcmd(qcmd);		\
	 };			  \
	spin_unlock_irqrestore(&__q->lock, flags);	\
} while (0)

static inline void free_qcmd(struct msm_queue_cmd *qcmd)
{
	if (!qcmd || !atomic_read(&qcmd->on_heap))
		return;
	if (!atomic_sub_return(1, &qcmd->on_heap))
		kfree(qcmd);
}

struct isp_msg_stats {
	uint32_t    id;
	uint32_t    buffer;
	uint32_t    frameCounter;
	int32_t     buf_idx;
	int32_t     fd;
};

struct msm_free_buf {
	uint8_t num_planes;
	uint32_t inst_handle;
	uint32_t ch_paddr[VIDEO_MAX_PLANES];
	uint32_t vb;
};

struct isp_msg_event {
	uint32_t msg_id;
	uint32_t sof_count;
};

struct isp_msg_output {
	uint8_t   output_id;
	struct msm_free_buf buf;
	uint32_t  frameCounter;
};

struct rdi_count_msg {
	uint32_t rdi_interface;
	uint32_t count;
};

/*                                  */
enum msm_camera_v4l2_subdev_notify {
	NOTIFY_ISP_MSG_EVT, /*                           */
	NOTIFY_VFE_MSG_OUT, /*                             */
	NOTIFY_VFE_MSG_STATS,  /*                            */
	NOTIFY_VFE_MSG_COMP_STATS, /*                            */
	NOTIFY_VFE_BUF_EVT, /*                           */
	NOTIFY_VFE_CAMIF_ERROR,
	NOTIFY_VFE_PIX_SOF_COUNT, /*         */
	NOTIFY_AXI_RDI_SOF_COUNT, /*                          */
	NOTIFY_PCLK_CHANGE, /*            */
	NOTIFY_VFE_IRQ,
	NOTIFY_AXI_IRQ,
	NOTIFY_GESTURE_EVT, /*                  */
	NOTIFY_GESTURE_CAM_EVT, /*           */
	NOTIFY_INVALID
};

enum isp_vfe_cmd_id {
	/*
                                              
               */
	ISP_VFE_CMD_ID_STREAM_ON,
	ISP_VFE_CMD_ID_STREAM_OFF,
	ISP_VFE_CMD_ID_FRAME_BUF_RELEASE
};

struct msm_cam_v4l2_device;
struct msm_cam_v4l2_dev_inst;
#define MSM_MAX_IMG_MODE                MSM_V4L2_EXT_CAPTURE_MODE_MAX

enum msm_buffer_state {
	MSM_BUFFER_STATE_UNUSED,
	MSM_BUFFER_STATE_INITIALIZED,
	MSM_BUFFER_STATE_PREPARED,
	MSM_BUFFER_STATE_QUEUED,
	MSM_BUFFER_STATE_RESERVED,
	MSM_BUFFER_STATE_DEQUEUED
};

/*                            */
struct msm_frame_buffer {
	/*                                          */
	struct vb2_buffer         vidbuf;
	struct list_head		  list;
	enum v4l2_mbus_pixelcode  pxlcode;
	enum msm_buffer_state state;
	int active;
};

struct msm_isp_color_fmt {
	char *name;
	int depth;
	int bitsperpxl;
	u32 fourcc;
	enum v4l2_mbus_pixelcode pxlcode;
	enum v4l2_colorspace colorspace;
};

struct msm_cam_return_frame_info {
	int dirty;
	int node_type;
	struct timeval timestamp;
	uint32_t frame_id;
};

struct msm_cam_timestamp {
	uint8_t present;
	struct timeval timestamp;
	uint32_t frame_id;
};

struct msm_cam_buf_map_info {
	int fd;
	uint32_t data_offset;
	unsigned long paddr;
	unsigned long len;
	struct file *file;
	struct ion_handle *handle;
};

struct msm_cam_meta_frame {
	struct msm_pp_frame frame;
	/*                               */
	struct msm_cam_buf_map_info map[VIDEO_MAX_PLANES];
};

struct msm_mctl_pp_frame_info {
	int user_cmd;
	struct msm_cam_meta_frame src_frame;
	struct msm_cam_meta_frame dest_frame;
	struct msm_mctl_pp_frame_cmd pp_frame_cmd;
	struct msm_cam_media_controller *p_mctl;
};

struct msm_mctl_pp_ctrl {
	int pp_msg_type;
	struct msm_mctl_pp_frame_info *pp_frame_info;

};
struct msm_mctl_pp_info {
	spinlock_t lock;
	uint32_t cnt;
	uint32_t pp_key;
	uint32_t cur_frame_id[MSM_MAX_IMG_MODE];
	struct msm_free_buf div_frame[MSM_MAX_IMG_MODE];
	struct msm_mctl_pp_ctrl pp_ctrl;

};
/*                                                         
                                                          
                                                          
                                                           
                                     
 */

struct msm_cam_media_controller {

	int (*mctl_open)(struct msm_cam_media_controller *p_mctl,
					 const char *const apps_id);
	int (*mctl_cb)(void);
	int (*mctl_cmd)(struct msm_cam_media_controller *p_mctl,
					unsigned int cmd, unsigned long arg);
	void (*mctl_release)(struct msm_cam_media_controller *p_mctl);
	int (*mctl_buf_init)(struct msm_cam_v4l2_dev_inst *pcam);
	int (*mctl_vbqueue_init)(struct msm_cam_v4l2_dev_inst *pcam,
				struct vb2_queue *q, enum v4l2_buf_type type);
	int (*mctl_ufmt_init)(struct msm_cam_media_controller *p_mctl);
	int (*isp_config)(struct msm_cam_media_controller *pmctl,
		 unsigned int cmd, unsigned long arg);
	int (*isp_notify)(struct msm_cam_media_controller *pmctl,
		struct v4l2_subdev *sd, unsigned int notification, void *arg);

	/*                                                  */
	struct v4l2_subdev *sensor_sdev; /*                   */
	struct v4l2_subdev *act_sdev; /*                     */
	struct v4l2_subdev *csiphy_sdev; /*                 */
	struct v4l2_subdev *csid_sdev; /*               */
	struct v4l2_subdev *csic_sdev; /*               */
	struct v4l2_subdev *ispif_sdev; /*                  */
	struct v4l2_subdev *gemini_sdev; /*                   */
	struct v4l2_subdev *vpe_sdev; /*                */
	struct v4l2_subdev *axi_sdev; /*                */
	struct v4l2_subdev *vfe_sdev; /*                */
	struct v4l2_subdev *eeprom_sdev; /*                   */
	struct v4l2_subdev *cpp_sdev;/*              */
	struct v4l2_subdev *flash_sdev;/*                */

	struct msm_cam_config_dev *config_device;

	/*                                */
	uint8_t opencnt; /*              */
	const char *apps_id; /*                                 */
	struct mutex lock;
	struct wake_lock wake_lock; /*                                */
	struct pm_qos_request pm_qos_req_list;
	struct msm_mctl_pp_info pp_info;
	struct msm_mctl_stats_t stats_info; /*               */
	uint32_t vfe_output_mode; /*                 */
	struct ion_client *client;
	struct kref refcount;

	/*        */
	struct msm_cam_v4l2_device *pcam_ptr;

	/*           */
	struct msm_camera_sensor_info *sdata;

	/*                           */
	uint32_t ping_imem_y;
	uint32_t ping_imem_cbcr;
	uint32_t pong_imem_y;
	uint32_t pong_imem_cbcr;

	/*                             */
	int domain_num;
	struct iommu_domain *domain;
};

struct msm_isp_buf_info {
	int type;
	unsigned long buffer;
	int fd;
};
struct msm_cam_buf_offset {
	uint32_t addr_offset;
	uint32_t data_offset;
};

#define MSM_DEV_INST_MAX                    24
struct msm_cam_v4l2_dev_inst {
	struct v4l2_fh  eventHandle;
	struct vb2_queue vid_bufq;
	spinlock_t vq_irqlock;
	struct list_head free_vq;
	struct v4l2_format vid_fmt;
	/*                  */
	enum v4l2_mbus_pixelcode sensor_pxlcode;
	struct msm_cam_v4l2_device *pcam;
	int my_index;
	uint32_t image_mode;
	int path;
	int buf_count;
	/*                        */
	struct msm_cam_buf_offset **buf_offset;
	struct v4l2_crop crop;
	int streamon;
	struct msm_mem_map_info mem_map;
	int is_mem_map_inst;
	struct img_plane_info plane_info;
	int vbqueue_initialized;
	struct mutex inst_lock;
	uint32_t inst_handle;
};

struct msm_cam_mctl_node {
	/*                  */
	struct v4l2_device v4l2_dev;
	struct video_device *pvdev;
	struct msm_cam_v4l2_dev_inst *dev_inst[MSM_DEV_INST_MAX];
	struct msm_cam_v4l2_dev_inst *dev_inst_map[MSM_MAX_IMG_MODE];
	struct mutex dev_lock;
	int active;
	int use_count;
};

/*                                                           */
struct msm_cam_v4l2_device {

	/*                         */
	int vnode_id;
	struct v4l2_device v4l2_dev; /*             */
	struct video_device *pvdev; /*                         */
	struct msm_cam_mctl_node mctl_node; /*                            */
	struct media_device media_dev; /*                            */

	/*                            */
	int use_count;
	struct mutex vid_lock;
	uint32_t server_queue_idx;
	uint32_t mctl_handle;
	struct msm_cam_v4l2_dev_inst *dev_inst[MSM_DEV_INST_MAX];
	struct msm_cam_v4l2_dev_inst *dev_inst_map[MSM_MAX_IMG_MODE];
	int op_mode;

	/*                     */
	struct msm_isp_color_fmt *usr_fmts;
	int num_fmts;

	struct v4l2_subdev *sensor_sdev; /*                   */
	struct v4l2_subdev *act_sdev; /*                     */
	struct v4l2_subdev *eeprom_sdev; /*                     */
	struct v4l2_subdev *flash_sdev; /*                  */
	struct msm_camera_sensor_info *sdata;

	struct msm_device_queue eventData_q; /*                              */
	struct mutex event_lock;
};

static inline struct msm_cam_v4l2_device *to_pcam(
	struct v4l2_device *v4l2_dev)
{
	return container_of(v4l2_dev, struct msm_cam_v4l2_device, v4l2_dev);
}

/*                                       
                             */
struct v4l2_queue_util {
	struct video_device *pvdev;
	struct v4l2_fh  eventHandle;
};

/*                                                          */
struct msm_cam_config_dev {
	struct cdev config_cdev;
	struct v4l2_queue_util config_stat_event_queue;
	int use_count;
	struct msm_cam_media_controller *p_mctl;
	struct msm_mem_map_info mem_map;
	int dev_num;
	int domain_num;
	struct iommu_domain *domain;
};

struct msm_cam_subdev_info {
	uint8_t sdev_type;
	/*                                            */
	uint8_t sd_index;
	/*                                                
                                */
	uint8_t irq_num;
};

/*                             */
#define MAX_NUM_ACTIVE_CAMERA 3

struct msm_cam_server_queue {
	uint32_t queue_active;
	struct msm_device_queue ctrl_q;
	struct msm_device_queue eventData_q;
	uint8_t *ctrl_data;
	uint32_t evt_id;
};

struct msm_cam_server_mctl_inst {
	struct msm_cam_media_controller mctl;
	uint32_t handle;
};

struct msm_cam_server_irqmap_entry {
	int irq_num;
	int irq_idx;
	uint8_t cam_hw_idx;
	uint8_t is_composite;
};

struct intr_table_entry {
	/*                              
                                                                    
                                                                   */
	int irq_num;
	/*                                                  */
	uint8_t cam_hw_idx;
	/*                                                 */
	uint32_t cam_hw_mask;
	/*                                                    
                                                                     
                                                                    
                                                                      */
	int irq_idx;
	/*                        */
	uint8_t is_composite;
	/*                                                       */
	uint32_t irq_trigger_type;
	/*                             
                                                 
                                           
                                             
                                                                    */
	int num_hwcore;
	/*                                           
                                                                   
                                        */
	struct v4l2_subdev *subdev_list[CAMERA_SS_IRQ_MAX];
	/*                            */
	const char *dev_name;
	/*                             */
	void *data;
};

struct irqmgr_intr_lkup_table {
	/*                                       
                                                            
                                                     
                                                             
                                                               
                                                             
                                                           
                                */
	struct intr_table_entry ind_intr_tbl[CAMERA_SS_IRQ_MAX];

	/*                                  
                                                                
                                                                  
                                                             
                                                            
                                                              
                                                                */
	struct intr_table_entry comp_intr_tbl[CAMERA_SS_IRQ_MAX];
};

struct interface_map {
	/*                                              
                             
  */
	int interface;
	/*                                                    */
	uint32_t mctl_handle;
	int vnode_id;
	int is_bayer_sensor;
};

/*                                                                 */
struct msm_cam_server_dev {

	/*                   */
	struct platform_device *server_pdev;
	/*                         */
	struct v4l2_device v4l2_dev;
	struct video_device *video_dev;
	struct media_device media_dev;

	/*                                    */
	struct msm_camera_info camera_info;
	/*                                     */
	struct msm_cam_config_dev_info config_info;
	/*                                                             */
	struct msm_cam_v4l2_device *pcam_active[MAX_NUM_ACTIVE_CAMERA];
	/*                                                                 */
	struct msm_cam_v4l2_device *opened_pcam[MAX_NUM_ACTIVE_CAMERA];
	/*                                */
	atomic_t number_pcam_active;
	struct v4l2_queue_util server_command_queue;

	/*                                                                   
                                                                
  */
	struct msm_cam_server_queue server_queue[MAX_NUM_ACTIVE_CAMERA];
	uint32_t server_evt_id;

	struct msm_cam_server_mctl_inst mctl[MAX_NUM_ACTIVE_CAMERA];
	uint32_t mctl_handle_cnt;

	struct interface_map interface_map_table[INTF_MAX];

	int use_count;
	/*                                 */
	struct msm_isp_ops *isp_subdev[MSM_MAX_CAMERA_CONFIGS];
	/*                                       */
	struct msm_mctl_node_info mctl_node_info;
	struct mutex server_lock;
	struct mutex server_queue_lock;
	/*            */
	struct v4l2_subdev *sensor_device[MAX_NUM_SENSOR_DEV];
	struct v4l2_subdev *csiphy_device[MAX_NUM_CSIPHY_DEV];
	struct v4l2_subdev *csid_device[MAX_NUM_CSID_DEV];
	struct v4l2_subdev *csic_device[MAX_NUM_CSIC_DEV];
	struct v4l2_subdev *ispif_device[MAX_NUM_ISPIF_DEV];
	struct v4l2_subdev *vfe_device[MAX_NUM_VFE_DEV];
	struct v4l2_subdev *axi_device[MAX_NUM_AXI_DEV];
	struct v4l2_subdev *vpe_device[MAX_NUM_VPE_DEV];
	struct v4l2_subdev *gesture_device;
	struct v4l2_subdev *cpp_device[MAX_NUM_CPP_DEV];
	struct v4l2_subdev *irqr_device;
	struct v4l2_subdev *cci_device;
	struct v4l2_subdev *flash_device[MAX_NUM_FLASH_DEV];

	spinlock_t  intr_table_lock;
	struct irqmgr_intr_lkup_table irq_lkup_table;
	/*                                                     
                                                        
                                                      
                                                         
                                                  */
	struct v4l2_subdev *subdev_table[MSM_CAM_HW_MAX];
	struct msm_cam_server_irqmap_entry hw_irqmap[CAMERA_SS_IRQ_MAX];

    /*                         */
	int domain_num;
	struct iommu_domain *domain;
};

enum msm_cam_buf_lookup_type {
	BUF_LOOKUP_INVALID,
	BUF_LOOKUP_BY_IMG_MODE,
	BUF_LOOKUP_BY_INST_HANDLE,
};

struct msm_cam_buf_handle {
	uint16_t buf_lookup_type;
	uint32_t image_mode;
	uint32_t inst_handle;
};

/*                       */
void msm_isp_vfe_dev_init(struct v4l2_subdev *vd);
int msm_isp_config(struct msm_cam_media_controller *pmctl,
			 unsigned int cmd, unsigned long arg);
int msm_isp_notify(struct msm_cam_media_controller *pmctl,
	struct v4l2_subdev *sd, unsigned int notification, void *arg);
/*
                                                       
*/
int msm_sensor_register(struct v4l2_subdev *);
int msm_isp_init_module(int g_num_config_nodes);

int msm_mctl_init(struct msm_cam_v4l2_device *pcam);
int msm_mctl_free(struct msm_cam_v4l2_device *pcam);
int msm_mctl_buf_init(struct msm_cam_v4l2_device *pcam);
int msm_mctl_init_user_formats(struct msm_cam_v4l2_device *pcam);
int msm_mctl_buf_done(struct msm_cam_media_controller *pmctl,
	struct msm_cam_buf_handle *buf_handle,
	struct msm_free_buf *buf,
	uint32_t frame_id);
int msm_mctl_buf_done_pp(struct msm_cam_media_controller *pmctl,
	struct msm_cam_buf_handle *buf_handle,
	struct msm_free_buf *frame,
	struct msm_cam_return_frame_info *ret_frame);
int msm_mctl_reserve_free_buf(struct msm_cam_media_controller *pmctl,
	struct msm_cam_v4l2_dev_inst *pcam_inst,
	struct msm_cam_buf_handle *buf_handle,
	struct msm_free_buf *free_buf);
int msm_mctl_release_free_buf(struct msm_cam_media_controller *pmctl,
	struct msm_cam_v4l2_dev_inst *pcam_inst,
	struct msm_free_buf *free_buf);
/*                      */
int msm_register_pmem(struct hlist_head *ptype, void __user *arg,
	struct ion_client *client, int domain_num);
int msm_pmem_table_del(struct hlist_head *ptype, void __user *arg,
	struct ion_client *client, int domain_num);
int msm_pmem_region_get_phy_addr(struct hlist_head *ptype,
	struct msm_mem_map_info *mem_map, int32_t *phyaddr);
uint8_t msm_pmem_region_lookup(struct hlist_head *ptype,
	int pmem_type, struct msm_pmem_region *reg, uint8_t maxcount);
uint8_t msm_pmem_region_lookup_2(struct hlist_head *ptype,
	int pmem_type, struct msm_pmem_region *reg,
	uint8_t maxcount);
unsigned long msm_pmem_stats_vtop_lookup(
	struct msm_cam_media_controller *mctl,
	unsigned long buffer, int fd);
unsigned long msm_pmem_stats_ptov_lookup(
	struct msm_cam_media_controller *mctl,
	unsigned long addr, int *fd);

int msm_vfe_subdev_init(struct v4l2_subdev *sd);
void msm_vfe_subdev_release(struct v4l2_subdev *sd);

int msm_isp_subdev_ioctl(struct v4l2_subdev *sd,
	struct msm_vfe_cfg_cmd *cfgcmd, void *data);
int msm_vpe_subdev_init(struct v4l2_subdev *sd);
int msm_gemini_subdev_init(struct v4l2_subdev *gemini_sd);
void msm_vpe_subdev_release(struct v4l2_subdev *sd);
void msm_gemini_subdev_release(struct v4l2_subdev *gemini_sd);
int msm_mctl_is_pp_msg_type(struct msm_cam_media_controller *p_mctl,
	int msg_type);
int msm_mctl_do_pp(struct msm_cam_media_controller *p_mctl,
	int msg_type, uint32_t y_phy, uint32_t frame_id);
int msm_mctl_pp_ioctl(struct msm_cam_media_controller *p_mctl,
	unsigned int cmd, unsigned long arg);
int msm_mctl_pp_notify(struct msm_cam_media_controller *pmctl,
	struct msm_mctl_pp_frame_info *pp_frame_info);
int msm_mctl_img_mode_to_inst_index(struct msm_cam_media_controller *pmctl,
	int out_type, int node_type);
struct msm_frame_buffer *msm_mctl_buf_find(
	struct msm_cam_media_controller *pmctl,
	struct msm_cam_v4l2_dev_inst *pcam_inst, int del_buf,
	struct msm_free_buf *fbuf);
void msm_mctl_gettimeofday(struct timeval *tv);
int msm_mctl_check_pp(struct msm_cam_media_controller *p_mctl,
	int msg_type, int *pp_divert_type, int *pp_type);
int msm_mctl_do_pp_divert(
	struct msm_cam_media_controller *p_mctl,
	struct msm_cam_buf_handle *buf_handle,
	struct msm_free_buf *fbuf,
	uint32_t frame_id, int pp_type);
int msm_mctl_pp_release_free_frame(
	struct msm_cam_media_controller *p_mctl,
	void __user *arg);
int msm_mctl_pp_reserve_free_frame(
	struct msm_cam_media_controller *p_mctl,
	void __user *arg);
int msm_mctl_set_pp_key(struct msm_cam_media_controller *p_mctl,
	void __user *arg);
int msm_mctl_pp_done(
	struct msm_cam_media_controller *p_mctl,
	void __user *arg);
int msm_mctl_pp_divert_done(
	struct msm_cam_media_controller *p_mctl,
	void __user *arg);
void msm_setup_v4l2_event_queue(struct v4l2_fh *eventHandle,
	struct video_device *pvdev);
void msm_destroy_v4l2_event_queue(struct v4l2_fh *eventHandle);
int msm_setup_mctl_node(struct msm_cam_v4l2_device *pcam);
struct msm_cam_v4l2_dev_inst *msm_mctl_get_pcam_inst(
	struct msm_cam_media_controller *pmctl,
	struct msm_cam_buf_handle *buf_handle);
int msm_mctl_buf_return_buf(struct msm_cam_media_controller *pmctl,
	int image_mode, struct msm_frame_buffer *buf);
int msm_mctl_map_user_frame(struct msm_cam_meta_frame *meta_frame,
	struct ion_client *client, int domain_num);
int msm_mctl_unmap_user_frame(struct msm_cam_meta_frame *meta_frame,
	struct ion_client *client, int domain_num);
int msm_mctl_pp_mctl_divert_done(struct msm_cam_media_controller *p_mctl,
	void __user *arg);
void msm_release_ion_client(struct kref *ref);
int msm_cam_register_subdev_node(struct v4l2_subdev *sd,
	struct msm_cam_subdev_info *sd_info);
int msm_mctl_find_sensor_subdevs(struct msm_cam_media_controller *p_mctl,
	uint8_t csiphy_core_index, uint8_t csid_core_index);
int msm_mctl_find_flash_subdev(struct msm_cam_media_controller *p_mctl,
	uint8_t index);
int msm_server_open_client(int *p_qidx);
int msm_server_send_ctrl(struct msm_ctrl_cmd *out, int ctrl_id);
int msm_server_close_client(int idx);
int msm_cam_server_open_mctl_session(struct msm_cam_v4l2_device *pcam,
	int *p_active);
int msm_cam_server_close_mctl_session(struct msm_cam_v4l2_device *pcam);
long msm_v4l2_evt_notify(struct msm_cam_media_controller *mctl,
	unsigned int cmd, unsigned long evt);
int msm_mctl_pp_get_vpe_buf_info(struct msm_mctl_pp_frame_info *zoom);
void msm_queue_init(struct msm_device_queue *queue, const char *name);
void msm_enqueue(struct msm_device_queue *queue, struct list_head *entry);
void msm_drain_eventq(struct msm_device_queue *queue);
#endif /*            */

#endif /*        */
