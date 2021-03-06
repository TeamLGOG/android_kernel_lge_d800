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

#ifndef _IPA_RM_I_H_
#define _IPA_RM_I_H_

#include <linux/workqueue.h>
#include <mach/ipa.h>

#define IPA_RM_RESOURCE_CONS_MAX \
	(IPA_RM_RESOURCE_MAX - IPA_RM_RESOURCE_PROD_MAX)
#define IPA_RM_RESORCE_IS_PROD(x) \
	(x >= IPA_RM_RESOURCE_PROD && x < IPA_RM_RESOURCE_PROD_MAX)
#define IPA_RM_RESORCE_IS_CONS(x) \
	(x >= IPA_RM_RESOURCE_PROD_MAX && x < IPA_RM_RESOURCE_MAX)
#define IPA_RM_INDEX_INVALID	(-1)

int ipa_rm_prod_index(enum ipa_rm_resource_name resource_name);
int ipa_rm_cons_index(enum ipa_rm_resource_name resource_name);

/* 
                                          
 */
enum ipa_rm_wq_cmd {
	IPA_RM_WQ_NOTIFY_PROD,
	IPA_RM_WQ_NOTIFY_CONS,
	IPA_RM_WQ_RESOURCE_CB
};

/* 
                                                        
               
                     
                                                                 
                                                          
                   
                                                              
                          
 */
struct ipa_rm_wq_work_type {
	struct work_struct		work;
	enum ipa_rm_wq_cmd		wq_cmd;
	enum ipa_rm_resource_name	resource_name;
	enum ipa_rm_event		event;
};

int ipa_rm_wq_send_cmd(enum ipa_rm_wq_cmd wq_cmd,
		enum ipa_rm_resource_name resource_name,
		enum ipa_rm_event event);

int ipa_rm_initialize(void);

void ipa_rm_exit(void);

#endif /*              */
