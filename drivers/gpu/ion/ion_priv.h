/*
 * drivers/gpu/ion/ion_priv.h
 *
 * Copyright (C) 2011 Google, Inc.
 * Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _ION_PRIV_H
#define _ION_PRIV_H

#include <linux/ion.h>
#include <linux/kref.h>
#include <linux/mm_types.h>
#include <linux/mutex.h>
#include <linux/rbtree.h>
#include <linux/seq_file.h>

#include "msm_ion_priv.h"
#include <linux/sched.h>
#include <linux/shrinker.h>
#include <linux/types.h>

struct ion_buffer *ion_handle_buffer(struct ion_handle *handle);

/* 
                                                       
                        
                                              
                                        
                                                        
                                 
                             
                                                           
             
                                                           
                                                   
                                          
                                                                 
                                                      
                                                           
                                                                  
                                                                
                                                       
               
                                            
                                                          
                                                                     
                               
                                                          
                               
*/
struct ion_buffer {
	struct kref ref;
	union {
		struct rb_node node;
		struct list_head list;
	};
	struct ion_device *dev;
	struct ion_heap *heap;
	unsigned long flags;
	size_t size;
	union {
		void *priv_virt;
		ion_phys_addr_t priv_phys;
	};
	struct mutex lock;
	int kmap_cnt;
	void *vaddr;
	int dmap_cnt;
	struct sg_table *sg_table;
	unsigned long *dirty;
	struct list_head vmas;
	/*                                */
	int handle_count;
	char task_comm[TASK_COMM_LEN];
	pid_t pid;
};

/* 
                                                       
                              
                      
                                                          
                                 
                                                    
                                       
                                        
                                           
                                     
                                         
 */
struct ion_heap_ops {
	int (*allocate) (struct ion_heap *heap,
			 struct ion_buffer *buffer, unsigned long len,
			 unsigned long align, unsigned long flags);
	void (*free) (struct ion_buffer *buffer);
	int (*phys) (struct ion_heap *heap, struct ion_buffer *buffer,
		     ion_phys_addr_t *addr, size_t *len);
	struct sg_table *(*map_dma) (struct ion_heap *heap,
					struct ion_buffer *buffer);
	void (*unmap_dma) (struct ion_heap *heap, struct ion_buffer *buffer);
	void * (*map_kernel) (struct ion_heap *heap, struct ion_buffer *buffer);
	void (*unmap_kernel) (struct ion_heap *heap, struct ion_buffer *buffer);
	int (*map_user) (struct ion_heap *mapper, struct ion_buffer *buffer,
			 struct vm_area_struct *vma);
	void (*unmap_user) (struct ion_heap *mapper, struct ion_buffer *buffer);
	int (*print_debug)(struct ion_heap *heap, struct seq_file *s,
			   const struct rb_root *mem_map);
	int (*secure_heap)(struct ion_heap *heap, int version, void *data);
	int (*unsecure_heap)(struct ion_heap *heap, int version, void *data);
	int (*secure_buffer)(struct ion_buffer *buffer, int version,
				void *data, int flags);
	int (*unsecure_buffer)(struct ion_buffer *buffer, int force_unsecure);
};

/* 
                                                         
 */
#define ION_HEAP_FLAG_DEFER_FREE (1 << 0)

/* 
                                                    
                                                                
                                        
                       
                             
                 
                                                               
                                                          
                   
                             
                            
                                                       
                                 
                                                          
                                              
                                                               
                                       
  
                                                                       
                                                                        
                                                                           
                                                     
 */
struct ion_heap {
	struct plist_node node;
	struct ion_device *dev;
	enum ion_heap_type type;
	struct ion_heap_ops *ops;
	unsigned long flags;
	unsigned int id;
	const char *name;
	void *priv;
	struct list_head free_list;
	struct rt_mutex lock;
	wait_queue_head_t waitqueue;
	struct task_struct *task;
	int (*debug_show)(struct ion_heap *heap, struct seq_file *, void *);
};

/* 
                                                
                   
  
                                              
 */
bool ion_buffer_cached(struct ion_buffer *buffer);

/* 
                                                                         
                   
  
                                                                      
                                                               
 */
bool ion_buffer_fault_user_mappings(struct ion_buffer *buffer);

/* 
                                                          
                                                            
  
                                     
 */
struct ion_device *ion_device_create(long (*custom_ioctl)
				     (struct ion_client *client,
				      unsigned int cmd,
				      unsigned long arg));

/* 
                                                         
                    
 */
void ion_device_destroy(struct ion_device *dev);

/* 
                                                      
                    
                          
 */
void ion_device_add_heap(struct ion_device *dev, struct ion_heap *heap);

/* 
                                                                   
                   
 */
void *ion_heap_map_kernel(struct ion_heap *, struct ion_buffer *);
void ion_heap_unmap_kernel(struct ion_heap *, struct ion_buffer *);
int ion_heap_map_user(struct ion_heap *, struct ion_buffer *,
			struct vm_area_struct *);
int ion_heap_buffer_zero(struct ion_buffer *buffer);


/* 
                                                                
                                                               
                        
 */

struct ion_heap *ion_heap_create(struct ion_platform_heap *);
void ion_heap_destroy(struct ion_heap *);
struct ion_heap *ion_system_heap_create(struct ion_platform_heap *);
void ion_system_heap_destroy(struct ion_heap *);

struct ion_heap *ion_system_contig_heap_create(struct ion_platform_heap *);
void ion_system_contig_heap_destroy(struct ion_heap *);

struct ion_heap *ion_carveout_heap_create(struct ion_platform_heap *);
void ion_carveout_heap_destroy(struct ion_heap *);

struct ion_heap *ion_chunk_heap_create(struct ion_platform_heap *);
void ion_chunk_heap_destroy(struct ion_heap *);
/* 
                                                                     
                                                    
 */
ion_phys_addr_t ion_carveout_allocate(struct ion_heap *heap, unsigned long size,
				      unsigned long align);
void ion_carveout_free(struct ion_heap *heap, ion_phys_addr_t addr,
		       unsigned long size);
/* 
                                                                       
                                                               
 */
#define ION_CARVEOUT_ALLOCATE_FAIL -1

/* 
                                                                  
                                                                         
                                                                          
                                                                           
                */

/* 
                                         
                                                    
                                                  
                                      
                                    
                                       
                                                                
              
                                                                 
             
                                                                 
                            
                                         
                                      
                                       
  
                                                                          
                                                                            
                                                                             
                  
 */
struct ion_page_pool {
	int high_count;
	int low_count;
	struct list_head high_items;
	struct list_head low_items;
	struct mutex mutex;
	void *(*alloc)(struct ion_page_pool *pool);
	void (*free)(struct ion_page_pool *pool, struct page *page);
	gfp_t gfp_mask;
	unsigned int order;
	struct plist_node list;
};

struct ion_page_pool *ion_page_pool_create(gfp_t gfp_mask, unsigned int order);
void ion_page_pool_destroy(struct ion_page_pool *);
void *ion_page_pool_alloc(struct ion_page_pool *);
void ion_page_pool_free(struct ion_page_pool *, struct page *);

#endif /*             */
