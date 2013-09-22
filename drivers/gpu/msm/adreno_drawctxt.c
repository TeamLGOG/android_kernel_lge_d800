/* Copyright (c) 2002,2007-2013, The Linux Foundation. All rights reserved.
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

#include <linux/slab.h>
#include <linux/msm_kgsl.h>

#include "kgsl.h"
#include "kgsl_sharedmem.h"
#include "adreno.h"

#define KGSL_INIT_REFTIMESTAMP		0x7FFFFFFF

/*                                         */
#define QUAD_LEN 12
#define QUAD_RESTORE_LEN 14

static unsigned int gmem_copy_quad[QUAD_LEN] = {
	0x00000000, 0x00000000, 0x3f800000,
	0x00000000, 0x00000000, 0x3f800000,
	0x00000000, 0x00000000, 0x3f800000,
	0x00000000, 0x00000000, 0x3f800000
};

static unsigned int gmem_restore_quad[QUAD_RESTORE_LEN] = {
	0x00000000, 0x3f800000, 0x3f800000,
	0x00000000, 0x00000000, 0x00000000,
	0x3f800000, 0x00000000, 0x00000000,
	0x3f800000, 0x00000000, 0x00000000,
	0x3f800000, 0x3f800000,
};

#define TEXCOORD_LEN 8

static unsigned int gmem_copy_texcoord[TEXCOORD_LEN] = {
	0x00000000, 0x3f800000,
	0x3f800000, 0x3f800000,
	0x00000000, 0x00000000,
	0x3f800000, 0x00000000
};

/*
                   
                                                                           
 */

/* 
                                                                
                               
 */

unsigned int uint2float(unsigned int uintval)
{
	unsigned int exp, frac = 0;

	if (uintval == 0)
		return 0;

	exp = ilog2(uintval);

	/*                    */
	if (23 > exp)
		frac = (uintval & (~(1 << exp))) << (23 - exp);

	/*                                          */
	exp = (exp + 127) << 23;

	return exp | frac;
}

static void set_gmem_copy_quad(struct gmem_shadow_t *shadow)
{
	/*                          */
	gmem_copy_quad[1] = uint2float(shadow->height);
	gmem_copy_quad[3] = uint2float(shadow->width);
	gmem_copy_quad[4] = uint2float(shadow->height);
	gmem_copy_quad[9] = uint2float(shadow->width);

	gmem_restore_quad[5] = uint2float(shadow->height);
	gmem_restore_quad[7] = uint2float(shadow->width);

	memcpy(shadow->quad_vertices.hostptr, gmem_copy_quad, QUAD_LEN << 2);
	memcpy(shadow->quad_vertices_restore.hostptr, gmem_restore_quad,
		QUAD_RESTORE_LEN << 2);

	memcpy(shadow->quad_texcoords.hostptr, gmem_copy_texcoord,
		TEXCOORD_LEN << 2);
}

/* 
                                                               
                                                   
                                                  
                                                               
 */

/*                                */
void build_quad_vtxbuff(struct adreno_context *drawctxt,
		struct gmem_shadow_t *shadow, unsigned int **incmd)
{
	 unsigned int *cmd = *incmd;

	/*                                            */
	shadow->quad_vertices.hostptr = cmd;
	shadow->quad_vertices.gpuaddr = virt2gpu(cmd, &drawctxt->gpustate);

	cmd += QUAD_LEN;

	/*                                                           */
	shadow->quad_vertices_restore.hostptr = cmd;
	shadow->quad_vertices_restore.gpuaddr =
		virt2gpu(cmd, &drawctxt->gpustate);

	cmd += QUAD_RESTORE_LEN;

	/*                                          */
	shadow->quad_texcoords.hostptr = cmd;
	shadow->quad_texcoords.gpuaddr = virt2gpu(cmd, &drawctxt->gpustate);

	cmd += TEXCOORD_LEN;

	set_gmem_copy_quad(shadow);
	*incmd = cmd;
}

/* 
                                                            
                                      
                                                         
  
                                                        
 */
struct kgsl_context *
adreno_drawctxt_create(struct kgsl_device_private *dev_priv,
			uint32_t *flags)
{
	struct adreno_context *drawctxt;
	struct kgsl_device *device = dev_priv->device;
	struct adreno_device *adreno_dev = ADRENO_DEVICE(device);
	struct adreno_ringbuffer *rb = &adreno_dev->ringbuffer;
	int ret;

	drawctxt = kzalloc(sizeof(struct adreno_context), GFP_KERNEL);
	if (drawctxt == NULL)
		return ERR_PTR(-ENOMEM);

	ret = kgsl_context_init(dev_priv, &drawctxt->base);
	if (ret != 0) {
		kfree(drawctxt);
		return ERR_PTR(ret);
	}

	drawctxt->bin_base_offset = 0;
	rb->timestamp[drawctxt->base.id] = 0;

	*flags &= (KGSL_CONTEXT_PREAMBLE |
		KGSL_CONTEXT_NO_GMEM_ALLOC |
		KGSL_CONTEXT_PER_CONTEXT_TS |
		KGSL_CONTEXT_USER_GENERATED_TS |
		KGSL_CONTEXT_NO_FAULT_TOLERANCE |
		KGSL_CONTEXT_TYPE_MASK);

	if (*flags & KGSL_CONTEXT_PREAMBLE)
		drawctxt->flags |= CTXT_FLAGS_PREAMBLE;

	if (*flags & KGSL_CONTEXT_NO_GMEM_ALLOC)
		drawctxt->flags |= CTXT_FLAGS_NOGMEMALLOC;

	if (*flags & KGSL_CONTEXT_PER_CONTEXT_TS)
		drawctxt->flags |= CTXT_FLAGS_PER_CONTEXT_TS;

	if (*flags & KGSL_CONTEXT_USER_GENERATED_TS) {
		if (!(*flags & KGSL_CONTEXT_PER_CONTEXT_TS)) {
			ret = -EINVAL;
			goto err;
		}
		drawctxt->flags |= CTXT_FLAGS_USER_GENERATED_TS;
	}

	if (*flags & KGSL_CONTEXT_NO_FAULT_TOLERANCE)
		drawctxt->flags |= CTXT_FLAGS_NO_FAULT_TOLERANCE;

	drawctxt->type =
		(*flags & KGSL_CONTEXT_TYPE_MASK) >> KGSL_CONTEXT_TYPE_SHIFT;

	ret = adreno_dev->gpudev->ctxt_create(adreno_dev, drawctxt);
	if (ret)
		goto err;

	kgsl_sharedmem_writel(device, &device->memstore,
			KGSL_MEMSTORE_OFFSET(drawctxt->base.id, ref_wait_ts),
			KGSL_INIT_REFTIMESTAMP);
	kgsl_sharedmem_writel(device, &device->memstore,
			KGSL_MEMSTORE_OFFSET(drawctxt->base.id, ts_cmp_enable),
			0);
	kgsl_sharedmem_writel(device, &device->memstore,
			KGSL_MEMSTORE_OFFSET(drawctxt->base.id, soptimestamp),
			0);
	kgsl_sharedmem_writel(device, &device->memstore,
			KGSL_MEMSTORE_OFFSET(drawctxt->base.id, eoptimestamp),
			0);

	return &drawctxt->base;
err:
	kgsl_context_put(&drawctxt->base);
	return ERR_PTR(ret);
}

/* 
                                                          
                                                           
  
 */
void adreno_drawctxt_detach(struct kgsl_context *context)
{
	struct kgsl_device *device;
	struct adreno_device *adreno_dev;
	struct adreno_context *drawctxt;

	if (context == NULL)
		return;

	device = context->device;
	adreno_dev = ADRENO_DEVICE(device);
	drawctxt = ADRENO_CONTEXT(context);
	/*                    */
	if (adreno_dev->drawctxt_active == drawctxt) {
		/*                                               
                     
   */
		drawctxt->flags &= ~(CTXT_FLAGS_GMEM_SAVE |
				     CTXT_FLAGS_SHADER_SAVE |
				     CTXT_FLAGS_GMEM_SHADOW |
				     CTXT_FLAGS_STATE_SHADOW);

		drawctxt->flags |= CTXT_FLAGS_BEING_DESTROYED;

		adreno_drawctxt_switch(adreno_dev, NULL, 0);
	}

	if (device->state != KGSL_STATE_HUNG)
		adreno_idle(device);

	kgsl_sharedmem_free(&drawctxt->gpustate);
	kgsl_sharedmem_free(&drawctxt->context_gmem_shadow.gmemshadow);
}


void adreno_drawctxt_destroy(struct kgsl_context *context)
{
	struct adreno_context *drawctxt;
	if (context == NULL)
		return;

	drawctxt = ADRENO_CONTEXT(context);
	kfree(drawctxt);
}

/* 
                                                                            
                                              
                                                           
                          
  
                                                                         
 */

void adreno_drawctxt_set_bin_base_offset(struct kgsl_device *device,
				      struct kgsl_context *context,
				      unsigned int offset)
{
	struct adreno_context *drawctxt;

	if (context == NULL)
		return;
	drawctxt = ADRENO_CONTEXT(context);
	drawctxt->bin_base_offset = offset;
}

/* 
                                                           
                                                    
                                          
                                                           
  
                                  
 */

void adreno_drawctxt_switch(struct adreno_device *adreno_dev,
				struct adreno_context *drawctxt,
				unsigned int flags)
{
	struct kgsl_device *device = &adreno_dev->dev;

	if (drawctxt) {
		if (flags & KGSL_CONTEXT_SAVE_GMEM)
			/*                                                 
                                        */
			drawctxt->flags |= CTXT_FLAGS_GMEM_SAVE;
		else
			/*                                          */
			drawctxt->flags &= ~CTXT_FLAGS_GMEM_SAVE;
	}

	/*                  */
	if (adreno_dev->drawctxt_active == drawctxt) {
		if (adreno_dev->gpudev->ctxt_draw_workaround &&
			adreno_is_a225(adreno_dev))
				adreno_dev->gpudev->ctxt_draw_workaround(
					adreno_dev, drawctxt);
		return;
	}

	KGSL_CTXT_INFO(device, "from %d to %d flags %d\n",
		adreno_dev->drawctxt_active ?
		adreno_dev->drawctxt_active->base.id : 0,
		drawctxt ? drawctxt->base.id : 0, flags);

	/*                      */
	adreno_dev->gpudev->ctxt_save(adreno_dev, adreno_dev->drawctxt_active);

	/*                                             */
	if (adreno_dev->drawctxt_active) {
		kgsl_context_put(&adreno_dev->drawctxt_active->base);
		adreno_dev->drawctxt_active = NULL;
	}

	/*                                    */
	if (drawctxt)
		_kgsl_context_get(&drawctxt->base);

	/*                     */
	adreno_dev->gpudev->ctxt_restore(adreno_dev, drawctxt);
	adreno_dev->drawctxt_active = drawctxt;
}