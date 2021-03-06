/*
 * fixmap.h: compile-time virtual memory allocation
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1998 Ingo Molnar
 *
 * Copyright 2008 Freescale Semiconductor Inc.
 *   Port to powerpc added by Kumar Gala
 */

#ifndef _ASM_FIXMAP_H
#define _ASM_FIXMAP_H

#ifndef __ASSEMBLY__
#include <linux/kernel.h>
#include <asm/page.h>
#ifdef CONFIG_HIGHMEM
#include <linux/threads.h>
#include <asm/kmap_types.h>
#endif

#define FIXADDR_TOP	((unsigned long)(-PAGE_SIZE))

/*
                                                        
                                                        
                                                     
                                                           
                                                         
                                               
                                                 
                                        
  
                                                    
                                                            
                                                        
                                       
  
                                                         
                 
 */
enum fixed_addresses {
	FIX_HOLE,
	/*                                                   */
	FIX_EARLY_DEBUG_TOP = FIX_HOLE,
	FIX_EARLY_DEBUG_BASE = FIX_EARLY_DEBUG_TOP+((128*1024)/PAGE_SIZE)-1,
#ifdef CONFIG_HIGHMEM
	FIX_KMAP_BEGIN,	/*                                              */
	FIX_KMAP_END = FIX_KMAP_BEGIN+(KM_TYPE_NR*NR_CPUS)-1,
#endif
	/*                */
	__end_of_fixed_addresses
};

extern void __set_fixmap (enum fixed_addresses idx,
					phys_addr_t phys, pgprot_t flags);

#define set_fixmap(idx, phys) \
		__set_fixmap(idx, phys, PAGE_KERNEL)
/*
                                                        
 */
#define set_fixmap_nocache(idx, phys) \
		__set_fixmap(idx, phys, PAGE_KERNEL_NCG)

#define clear_fixmap(idx) \
		__set_fixmap(idx, 0, __pgprot(0))

#define __FIXADDR_SIZE	(__end_of_fixed_addresses << PAGE_SHIFT)
#define FIXADDR_START		(FIXADDR_TOP - __FIXADDR_SIZE)

#define __fix_to_virt(x)	(FIXADDR_TOP - ((x) << PAGE_SHIFT))
#define __virt_to_fix(x)	((FIXADDR_TOP - ((x)&PAGE_MASK)) >> PAGE_SHIFT)

extern void __this_fixmap_does_not_exist(void);

/*
                                                                 
                                                                      
                                                                  
 */
static __always_inline unsigned long fix_to_virt(const unsigned int idx)
{
	/*
                                                          
                                                          
                                                          
                          
   
                                                       
                                                  
  */
	if (idx >= __end_of_fixed_addresses)
		__this_fixmap_does_not_exist();

        return __fix_to_virt(idx);
}

static inline unsigned long virt_to_fix(const unsigned long vaddr)
{
	BUG_ON(vaddr >= FIXADDR_TOP || vaddr < FIXADDR_START);
	return __virt_to_fix(vaddr);
}

#endif /*               */
#endif
