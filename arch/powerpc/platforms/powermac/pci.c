/*
 * Support for PCI bridges found on Power Macintoshes.
 *
 * Copyright (C) 2003-2005 Benjamin Herrenschmuidt (benh@kernel.crashing.org)
 * Copyright (C) 1997 Paul Mackerras (paulus@samba.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/bootmem.h>
#include <linux/irq.h>
#include <linux/of_pci.h>

#include <asm/sections.h>
#include <asm/io.h>
#include <asm/prom.h>
#include <asm/pci-bridge.h>
#include <asm/machdep.h>
#include <asm/pmac_feature.h>
#include <asm/grackle.h>
#include <asm/ppc-pci.h>

#undef DEBUG

#ifdef DEBUG
#define DBG(x...) printk(x)
#else
#define DBG(x...)
#endif

/*                                                                   
                                                   */
static int has_uninorth;
#ifdef CONFIG_PPC64
static struct pci_controller *u3_agp;
#else
static int has_second_ohare;
#endif /*              */

extern int pcibios_assign_bus_offset;

struct device_node *k2_skiplist[2];

/*
                                                                         
 */
#define BANDIT_DEVID_2	8
#define BANDIT_REVID	3

#define BANDIT_DEVNUM	11
#define BANDIT_MAGIC	0x50
#define BANDIT_COHERENT	0x40

static int __init fixup_one_level_bus_range(struct device_node *node, int higher)
{
	for (; node != 0;node = node->sibling) {
		const int * bus_range;
		const unsigned int *class_code;
		int len;

		/*                                                      */
		class_code = of_get_property(node, "class-code", NULL);
		if (!class_code || ((*class_code >> 8) != PCI_CLASS_BRIDGE_PCI &&
			(*class_code >> 8) != PCI_CLASS_BRIDGE_CARDBUS))
			continue;
		bus_range = of_get_property(node, "bus-range", &len);
		if (bus_range != NULL && len > 2 * sizeof(int)) {
			if (bus_range[1] > higher)
				higher = bus_range[1];
		}
		higher = fixup_one_level_bus_range(node->child, higher);
	}
	return higher;
}

/*                                                                  
                                                                   
  
                                                                      
                             
 */
static void __init fixup_bus_range(struct device_node *bridge)
{
	int *bus_range, len;
	struct property *prop;

	/*                                              */
	prop = of_find_property(bridge, "bus-range", &len);
	if (prop == NULL || prop->length < 2 * sizeof(int))
		return;

	bus_range = prop->value;
	bus_range[1] = fixup_one_level_bus_range(bridge->child, bus_range[1]);
}

/*
                                                               
  
                                                              
                                                            
                                            
  
                                                             
                                                        
                                                                
                                       
  
                                                           
                                                             
                                         
  
                                                           
                                                               
                                                            
             
 */

#define MACRISC_CFA0(devfn, off)	\
	((1 << (unsigned int)PCI_SLOT(dev_fn)) \
	| (((unsigned int)PCI_FUNC(dev_fn)) << 8) \
	| (((unsigned int)(off)) & 0xFCUL))

#define MACRISC_CFA1(bus, devfn, off)	\
	((((unsigned int)(bus)) << 16) \
	|(((unsigned int)(devfn)) << 8) \
	|(((unsigned int)(off)) & 0xFCUL) \
	|1UL)

static volatile void __iomem *macrisc_cfg_access(struct pci_controller* hose,
					       u8 bus, u8 dev_fn, u8 offset)
{
	unsigned int caddr;

	if (bus == hose->first_busno) {
		if (dev_fn < (11 << 3))
			return NULL;
		caddr = MACRISC_CFA0(dev_fn, offset);
	} else
		caddr = MACRISC_CFA1(bus, dev_fn, offset);

	/*                                                                */
	do {
		out_le32(hose->cfg_addr, caddr);
	} while (in_le32(hose->cfg_addr) != caddr);

	offset &= has_uninorth ? 0x07 : 0x03;
	return hose->cfg_data + offset;
}

static int macrisc_read_config(struct pci_bus *bus, unsigned int devfn,
				      int offset, int len, u32 *val)
{
	struct pci_controller *hose;
	volatile void __iomem *addr;

	hose = pci_bus_to_host(bus);
	if (hose == NULL)
		return PCIBIOS_DEVICE_NOT_FOUND;
	if (offset >= 0x100)
		return  PCIBIOS_BAD_REGISTER_NUMBER;
	addr = macrisc_cfg_access(hose, bus->number, devfn, offset);
	if (!addr)
		return PCIBIOS_DEVICE_NOT_FOUND;
	/*
                                                       
                                               
  */
	switch (len) {
	case 1:
		*val = in_8(addr);
		break;
	case 2:
		*val = in_le16(addr);
		break;
	default:
		*val = in_le32(addr);
		break;
	}
	return PCIBIOS_SUCCESSFUL;
}

static int macrisc_write_config(struct pci_bus *bus, unsigned int devfn,
				       int offset, int len, u32 val)
{
	struct pci_controller *hose;
	volatile void __iomem *addr;

	hose = pci_bus_to_host(bus);
	if (hose == NULL)
		return PCIBIOS_DEVICE_NOT_FOUND;
	if (offset >= 0x100)
		return  PCIBIOS_BAD_REGISTER_NUMBER;
	addr = macrisc_cfg_access(hose, bus->number, devfn, offset);
	if (!addr)
		return PCIBIOS_DEVICE_NOT_FOUND;
	/*
                                                       
                                               
  */
	switch (len) {
	case 1:
		out_8(addr, val);
		break;
	case 2:
		out_le16(addr, val);
		break;
	default:
		out_le32(addr, val);
		break;
	}
	return PCIBIOS_SUCCESSFUL;
}

static struct pci_ops macrisc_pci_ops =
{
	.read = macrisc_read_config,
	.write = macrisc_write_config,
};

#ifdef CONFIG_PPC32
/*
                                                       
 */
static int chaos_validate_dev(struct pci_bus *bus, int devfn, int offset)
{
	struct device_node *np;
	const u32 *vendor, *device;

	if (offset >= 0x100)
		return  PCIBIOS_BAD_REGISTER_NUMBER;
	np = of_pci_find_child_device(bus->dev.of_node, devfn);
	if (np == NULL)
		return PCIBIOS_DEVICE_NOT_FOUND;

	vendor = of_get_property(np, "vendor-id", NULL);
	device = of_get_property(np, "device-id", NULL);
	if (vendor == NULL || device == NULL)
		return PCIBIOS_DEVICE_NOT_FOUND;

	if ((*vendor == 0x106b) && (*device == 3) && (offset >= 0x10)
	    && (offset != 0x14) && (offset != 0x18) && (offset <= 0x24))
		return PCIBIOS_BAD_REGISTER_NUMBER;

	return PCIBIOS_SUCCESSFUL;
}

static int
chaos_read_config(struct pci_bus *bus, unsigned int devfn, int offset,
		  int len, u32 *val)
{
	int result = chaos_validate_dev(bus, devfn, offset);
	if (result == PCIBIOS_BAD_REGISTER_NUMBER)
		*val = ~0U;
	if (result != PCIBIOS_SUCCESSFUL)
		return result;
	return macrisc_read_config(bus, devfn, offset, len, val);
}

static int
chaos_write_config(struct pci_bus *bus, unsigned int devfn, int offset,
		   int len, u32 val)
{
	int result = chaos_validate_dev(bus, devfn, offset);
	if (result != PCIBIOS_SUCCESSFUL)
		return result;
	return macrisc_write_config(bus, devfn, offset, len, val);
}

static struct pci_ops chaos_pci_ops =
{
	.read = chaos_read_config,
	.write = chaos_write_config,
};

static void __init setup_chaos(struct pci_controller *hose,
			       struct resource *addr)
{
	/*                         */
	hose->ops = &chaos_pci_ops;
	hose->cfg_addr = ioremap(addr->start + 0x800000, 0x1000);
	hose->cfg_data = ioremap(addr->start + 0xc00000, 0x1000);
}
#endif /*              */

#ifdef CONFIG_PPC64
/*
                                                                     
                                         
 */

/*
                                                         
  
                        
                                                                
                                                                
                                     
                                              
 */
static int u3_ht_skip_device(struct pci_controller *hose,
			     struct pci_bus *bus, unsigned int devfn)
{
	struct device_node *busdn, *dn;
	int i;

	/*                                                                  
                                                                    
                                                                      
                                                      
  */
	if (bus->self)
		busdn = pci_device_to_OF_node(bus->self);
	else if (devfn == 0)
		return 0;
	else
		busdn = hose->dn;
	for (dn = busdn->child; dn; dn = dn->sibling)
		if (PCI_DN(dn) && PCI_DN(dn)->devfn == devfn)
			break;
	if (dn == NULL)
		return -1;

	/*
                                                         
                                  
  */
	for (i=0; i<2; i++)
		if (k2_skiplist[i] == dn)
			return 1;

	return 0;
}

#define U3_HT_CFA0(devfn, off)		\
		((((unsigned int)devfn) << 8) | offset)
#define U3_HT_CFA1(bus, devfn, off)	\
		(U3_HT_CFA0(devfn, off) \
		+ (((unsigned int)bus) << 16) \
		+ 0x01000000UL)

static void __iomem *u3_ht_cfg_access(struct pci_controller *hose, u8 bus,
				      u8 devfn, u8 offset, int *swap)
{
	*swap = 1;
	if (bus == hose->first_busno) {
		if (devfn != 0)
			return hose->cfg_data + U3_HT_CFA0(devfn, offset);
		*swap = 0;
		return ((void __iomem *)hose->cfg_addr) + (offset << 2);
	} else
		return hose->cfg_data + U3_HT_CFA1(bus, devfn, offset);
}

static int u3_ht_read_config(struct pci_bus *bus, unsigned int devfn,
				    int offset, int len, u32 *val)
{
	struct pci_controller *hose;
	void __iomem *addr;
	int swap;

	hose = pci_bus_to_host(bus);
	if (hose == NULL)
		return PCIBIOS_DEVICE_NOT_FOUND;
	if (offset >= 0x100)
		return  PCIBIOS_BAD_REGISTER_NUMBER;
	addr = u3_ht_cfg_access(hose, bus->number, devfn, offset, &swap);
	if (!addr)
		return PCIBIOS_DEVICE_NOT_FOUND;

	switch (u3_ht_skip_device(hose, bus, devfn)) {
	case 0:
		break;
	case 1:
		switch (len) {
		case 1:
			*val = 0xff; break;
		case 2:
			*val = 0xffff; break;
		default:
			*val = 0xfffffffful; break;
		}
		return PCIBIOS_SUCCESSFUL;
	default:
		return PCIBIOS_DEVICE_NOT_FOUND;
	}

	/*
                                                       
                                               
  */
	switch (len) {
	case 1:
		*val = in_8(addr);
		break;
	case 2:
		*val = swap ? in_le16(addr) : in_be16(addr);
		break;
	default:
		*val = swap ? in_le32(addr) : in_be32(addr);
		break;
	}
	return PCIBIOS_SUCCESSFUL;
}

static int u3_ht_write_config(struct pci_bus *bus, unsigned int devfn,
				     int offset, int len, u32 val)
{
	struct pci_controller *hose;
	void __iomem *addr;
	int swap;

	hose = pci_bus_to_host(bus);
	if (hose == NULL)
		return PCIBIOS_DEVICE_NOT_FOUND;
	if (offset >= 0x100)
		return  PCIBIOS_BAD_REGISTER_NUMBER;
	addr = u3_ht_cfg_access(hose, bus->number, devfn, offset, &swap);
	if (!addr)
		return PCIBIOS_DEVICE_NOT_FOUND;

	switch (u3_ht_skip_device(hose, bus, devfn)) {
	case 0:
		break;
	case 1:
		return PCIBIOS_SUCCESSFUL;
	default:
		return PCIBIOS_DEVICE_NOT_FOUND;
	}

	/*
                                                       
                                               
  */
	switch (len) {
	case 1:
		out_8(addr, val);
		break;
	case 2:
		swap ? out_le16(addr, val) : out_be16(addr, val);
		break;
	default:
		swap ? out_le32(addr, val) : out_be32(addr, val);
		break;
	}
	return PCIBIOS_SUCCESSFUL;
}

static struct pci_ops u3_ht_pci_ops =
{
	.read = u3_ht_read_config,
	.write = u3_ht_write_config,
};

#define U4_PCIE_CFA0(devfn, off)	\
	((1 << ((unsigned int)PCI_SLOT(dev_fn)))	\
	 | (((unsigned int)PCI_FUNC(dev_fn)) << 8)	\
	 | ((((unsigned int)(off)) >> 8) << 28) \
	 | (((unsigned int)(off)) & 0xfcU))

#define U4_PCIE_CFA1(bus, devfn, off)	\
	((((unsigned int)(bus)) << 16) \
	 |(((unsigned int)(devfn)) << 8)	\
	 | ((((unsigned int)(off)) >> 8) << 28) \
	 |(((unsigned int)(off)) & 0xfcU)	\
	 |1UL)

static volatile void __iomem *u4_pcie_cfg_access(struct pci_controller* hose,
					u8 bus, u8 dev_fn, int offset)
{
	unsigned int caddr;

	if (bus == hose->first_busno) {
		caddr = U4_PCIE_CFA0(dev_fn, offset);
	} else
		caddr = U4_PCIE_CFA1(bus, dev_fn, offset);

	/*                                                                */
	do {
		out_le32(hose->cfg_addr, caddr);
	} while (in_le32(hose->cfg_addr) != caddr);

	offset &= 0x03;
	return hose->cfg_data + offset;
}

static int u4_pcie_read_config(struct pci_bus *bus, unsigned int devfn,
			       int offset, int len, u32 *val)
{
	struct pci_controller *hose;
	volatile void __iomem *addr;

	hose = pci_bus_to_host(bus);
	if (hose == NULL)
		return PCIBIOS_DEVICE_NOT_FOUND;
	if (offset >= 0x1000)
		return  PCIBIOS_BAD_REGISTER_NUMBER;
	addr = u4_pcie_cfg_access(hose, bus->number, devfn, offset);
	if (!addr)
		return PCIBIOS_DEVICE_NOT_FOUND;
	/*
                                                       
                                               
  */
	switch (len) {
	case 1:
		*val = in_8(addr);
		break;
	case 2:
		*val = in_le16(addr);
		break;
	default:
		*val = in_le32(addr);
		break;
	}
	return PCIBIOS_SUCCESSFUL;
}

static int u4_pcie_write_config(struct pci_bus *bus, unsigned int devfn,
				int offset, int len, u32 val)
{
	struct pci_controller *hose;
	volatile void __iomem *addr;

	hose = pci_bus_to_host(bus);
	if (hose == NULL)
		return PCIBIOS_DEVICE_NOT_FOUND;
	if (offset >= 0x1000)
		return  PCIBIOS_BAD_REGISTER_NUMBER;
	addr = u4_pcie_cfg_access(hose, bus->number, devfn, offset);
	if (!addr)
		return PCIBIOS_DEVICE_NOT_FOUND;
	/*
                                                       
                                               
  */
	switch (len) {
	case 1:
		out_8(addr, val);
		break;
	case 2:
		out_le16(addr, val);
		break;
	default:
		out_le32(addr, val);
		break;
	}
	return PCIBIOS_SUCCESSFUL;
}

static struct pci_ops u4_pcie_pci_ops =
{
	.read = u4_pcie_read_config,
	.write = u4_pcie_write_config,
};

static void __devinit pmac_pci_fixup_u4_of_node(struct pci_dev *dev)
{
	/*                                                                
                                                                  
                                                                      
                                                                  
                                                                 
                                                                     
  */
	if (dev->dev.of_node == NULL)
		dev->dev.of_node = pcibios_get_phb_of_node(dev->bus);
}
DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_APPLE, 0x5b, pmac_pci_fixup_u4_of_node);

#endif /*              */

#ifdef CONFIG_PPC32
/*
                                                             
                                                           
 */
static void __init init_bandit(struct pci_controller *bp)
{
	unsigned int vendev, magic;
	int rev;

	/*                                                         */
	out_le32(bp->cfg_addr, (1UL << BANDIT_DEVNUM) + PCI_VENDOR_ID);
	udelay(2);
	vendev = in_le32(bp->cfg_data);
	if (vendev == (PCI_DEVICE_ID_APPLE_BANDIT << 16) +
			PCI_VENDOR_ID_APPLE) {
		/*                      */
		out_le32(bp->cfg_addr,
			 (1UL << BANDIT_DEVNUM) + PCI_REVISION_ID);
		udelay(2);
		rev = in_8(bp->cfg_data);
		if (rev != BANDIT_REVID)
			printk(KERN_WARNING
			       "Unknown revision %d for bandit\n", rev);
	} else if (vendev != (BANDIT_DEVID_2 << 16) + PCI_VENDOR_ID_APPLE) {
		printk(KERN_WARNING "bandit isn't? (%x)\n", vendev);
		return;
	}

	/*                              */
	out_le32(bp->cfg_addr, (1UL << BANDIT_DEVNUM) + BANDIT_MAGIC);
	udelay(2);
	magic = in_le32(bp->cfg_data);
	if ((magic & BANDIT_COHERENT) != 0)
		return;
	magic |= BANDIT_COHERENT;
	udelay(2);
	out_le32(bp->cfg_data, magic);
	printk(KERN_INFO "Cache coherency enabled for bandit/PSX\n");
}

/*
                                                         
 */
static void __init init_p2pbridge(void)
{
	struct device_node *p2pbridge;
	struct pci_controller* hose;
	u8 bus, devfn;
	u16 val;

	/*                                                     
                                 */
	p2pbridge = of_find_node_by_name(NULL, "pci-bridge");
	if (p2pbridge == NULL
	    || p2pbridge->parent == NULL
	    || strcmp(p2pbridge->parent->name, "pci") != 0)
		goto done;
	if (pci_device_from_OF_node(p2pbridge, &bus, &devfn) < 0) {
		DBG("Can't find PCI infos for PCI<->PCI bridge\n");
		goto done;
	}
	/*                                                               
                                              
  */
	hose = pci_find_hose_for_OF_device(p2pbridge);
	if (!hose) {
		DBG("Can't find hose for PCI<->PCI bridge\n");
		goto done;
	}
	if (early_read_config_word(hose, bus, devfn,
				   PCI_BRIDGE_CONTROL, &val) < 0) {
		printk(KERN_ERR "init_p2pbridge: couldn't read bridge"
		       " control\n");
		goto done;
	}
	val &= ~PCI_BRIDGE_CTL_MASTER_ABORT;
	early_write_config_word(hose, bus, devfn, PCI_BRIDGE_CONTROL, val);
done:
	of_node_put(p2pbridge);
}

static void __init init_second_ohare(void)
{
	struct device_node *np = of_find_node_by_name(NULL, "pci106b,7");
	unsigned char bus, devfn;
	unsigned short cmd;

	if (np == NULL)
		return;

	/*                                                             
                                                  
  */
	if (pci_device_from_OF_node(np, &bus, &devfn) == 0) {
		struct pci_controller* hose =
			pci_find_hose_for_OF_device(np);
		if (!hose) {
			printk(KERN_ERR "Can't find PCI hose for OHare2 !\n");
			of_node_put(np);
			return;
		}
		early_read_config_word(hose, bus, devfn, PCI_COMMAND, &cmd);
		cmd |= PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER;
		cmd &= ~PCI_COMMAND_IO;
		early_write_config_word(hose, bus, devfn, PCI_COMMAND, cmd);
	}
	has_second_ohare = 1;
	of_node_put(np);
}

/*
                                                                   
                                                                
                                                                  
                        
 */
static void __init fixup_nec_usb2(void)
{
	struct device_node *nec;

	for (nec = NULL; (nec = of_find_node_by_name(nec, "usb")) != NULL;) {
		struct pci_controller *hose;
		u32 data;
		const u32 *prop;
		u8 bus, devfn;

		prop = of_get_property(nec, "vendor-id", NULL);
		if (prop == NULL)
			continue;
		if (0x1033 != *prop)
			continue;
		prop = of_get_property(nec, "device-id", NULL);
		if (prop == NULL)
			continue;
		if (0x0035 != *prop)
			continue;
		prop = of_get_property(nec, "reg", NULL);
		if (prop == NULL)
			continue;
		devfn = (prop[0] >> 8) & 0xff;
		bus = (prop[0] >> 16) & 0xff;
		if (PCI_FUNC(devfn) != 0)
			continue;
		hose = pci_find_hose_for_OF_device(nec);
		if (!hose)
			continue;
		early_read_config_dword(hose, bus, devfn, 0xe4, &data);
		if (data & 1UL) {
			printk("Found NEC PD720100A USB2 chip with disabled"
			       " EHCI, fixing up...\n");
			data &= ~1UL;
			early_write_config_dword(hose, bus, devfn, 0xe4, data);
		}
	}
}

static void __init setup_bandit(struct pci_controller *hose,
				struct resource *addr)
{
	hose->ops = &macrisc_pci_ops;
	hose->cfg_addr = ioremap(addr->start + 0x800000, 0x1000);
	hose->cfg_data = ioremap(addr->start + 0xc00000, 0x1000);
	init_bandit(hose);
}

static int __init setup_uninorth(struct pci_controller *hose,
				 struct resource *addr)
{
	pci_add_flags(PCI_REASSIGN_ALL_BUS);
	has_uninorth = 1;
	hose->ops = &macrisc_pci_ops;
	hose->cfg_addr = ioremap(addr->start + 0x800000, 0x1000);
	hose->cfg_data = ioremap(addr->start + 0xc00000, 0x1000);
	/*                                                          */
	return addr->start == 0xf2000000;
}
#endif /*              */

#ifdef CONFIG_PPC64
static void __init setup_u3_agp(struct pci_controller* hose)
{
	/*                                                          
                                                               
                                                           
                                                                
              
                                                             
                                                             
                                                          
  */
	hose->first_busno = 0xf0;
	hose->last_busno = 0xff;
	has_uninorth = 1;
	hose->ops = &macrisc_pci_ops;
	hose->cfg_addr = ioremap(0xf0000000 + 0x800000, 0x1000);
	hose->cfg_data = ioremap(0xf0000000 + 0xc00000, 0x1000);
	u3_agp = hose;
}

static void __init setup_u4_pcie(struct pci_controller* hose)
{
	/*                                                              
                       
  */
	hose->ops = &u4_pcie_pci_ops;
	hose->cfg_addr = ioremap(0xf0000000 + 0x800000, 0x1000);
	hose->cfg_data = ioremap(0xf0000000 + 0xc00000, 0x1000);

	/*                                                          
                                                           
                                                              
                                                              
                 
  */
	hose->first_busno = 0x00;
	hose->last_busno = 0xff;
}

static void __init parse_region_decode(struct pci_controller *hose,
				       u32 decode)
{
	unsigned long base, end, next = -1;
	int i, cur = -1;

	/*                                                                   
                                             
  */
	for (i = 0; i < 31; i++) {
		if ((decode & (0x80000000 >> i)) == 0)
			continue;
		if (i < 16) {
			base = 0xf0000000 | (((u32)i) << 24);
			end = base + 0x00ffffff;
		} else {
			base = ((u32)i-16) << 28;
			end = base + 0x0fffffff;
		}
		if (base != next) {
			if (++cur >= 3) {
				printk(KERN_WARNING "PCI: Too many ranges !\n");
				break;
			}
			hose->mem_resources[cur].flags = IORESOURCE_MEM;
			hose->mem_resources[cur].name = hose->dn->full_name;
			hose->mem_resources[cur].start = base;
			hose->mem_resources[cur].end = end;
			DBG("  %d: 0x%08lx-0x%08lx\n", cur, base, end);
		} else {
			DBG("   :           -0x%08lx\n", end);
			hose->mem_resources[cur].end = end;
		}
		next = end + 1;
	}
}

static void __init setup_u3_ht(struct pci_controller* hose)
{
	struct device_node *np = hose->dn;
	struct resource cfg_res, self_res;
	u32 decode;

	hose->ops = &u3_ht_pci_ops;

	/*                                
  */
	if (of_address_to_resource(np, 0, &cfg_res) ||
	    of_address_to_resource(np, 1, &self_res)) {
		printk(KERN_ERR "PCI: Failed to get U3/U4 HT resources !\n");
		return;
	}

	/*                                                               
                 
  */
	hose->cfg_data = ioremap(cfg_res.start, 0x02000000);
	hose->cfg_addr = ioremap(self_res.start, resource_size(&self_res));

	/*
                                                                     
                                                                     
                                                                
  */
	hose->io_base_phys = 0xf4000000;
	hose->pci_io_size = 0x00400000;
	hose->io_resource.name = np->full_name;
	hose->io_resource.start = 0;
	hose->io_resource.end = 0x003fffff;
	hose->io_resource.flags = IORESOURCE_IO;
	hose->pci_mem_offset = 0;
	hose->first_busno = 0;
	hose->last_busno = 0xef;

	/*                                                 */
	decode = in_be32(hose->cfg_addr + 0x80);

	DBG("PCI: Apple HT bridge decode register: 0x%08x\n", decode);

	/*                                                         
                                                                   
                                             
                                                            
   
                                                                        
                                                                       
                                     
   
                                                                   
                                                                   
                                              
   
                                                                  
                                                                  
                                                        
  */
	decode &= 0x003fffff;

	/*                                                  */
	parse_region_decode(hose, decode);
}
#endif /*              */

/*
                                                                     
                                                                     
                                                                          
 */
static int __init pmac_add_bridge(struct device_node *dev)
{
	int len;
	struct pci_controller *hose;
	struct resource rsrc;
	char *disp_name;
	const int *bus_range;
	int primary = 1, has_address = 0;

	DBG("Adding PCI host bridge %s\n", dev->full_name);

	/*                                     */
	has_address = (of_address_to_resource(dev, 0, &rsrc) == 0);

	/*                      */
	bus_range = of_get_property(dev, "bus-range", &len);
	if (bus_range == NULL || len < 2 * sizeof(int)) {
		printk(KERN_WARNING "Can't get bus-range for %s, assume"
		       " bus 0\n", dev->full_name);
	}

	hose = pcibios_alloc_controller(dev);
	if (!hose)
		return -ENOMEM;
	hose->first_busno = bus_range ? bus_range[0] : 0;
	hose->last_busno = bus_range ? bus_range[1] : 0xff;

	disp_name = NULL;

	/*                      */
#ifdef CONFIG_PPC64
	if (of_device_is_compatible(dev, "u3-agp")) {
		setup_u3_agp(hose);
		disp_name = "U3-AGP";
		primary = 0;
	} else if (of_device_is_compatible(dev, "u3-ht")) {
		setup_u3_ht(hose);
		disp_name = "U3-HT";
		primary = 1;
	} else if (of_device_is_compatible(dev, "u4-pcie")) {
		setup_u4_pcie(hose);
		disp_name = "U4-PCIE";
		primary = 0;
	}
	printk(KERN_INFO "Found %s PCI host bridge.  Firmware bus number:"
	       " %d->%d\n", disp_name, hose->first_busno, hose->last_busno);
#endif /*              */

	/*                      */
#ifdef CONFIG_PPC32
	if (of_device_is_compatible(dev, "uni-north")) {
		primary = setup_uninorth(hose, &rsrc);
		disp_name = "UniNorth";
	} else if (strcmp(dev->name, "pci") == 0) {
		/*                                       */
		setup_grackle(hose);
		disp_name = "Grackle (MPC106)";
	} else if (strcmp(dev->name, "bandit") == 0) {
		setup_bandit(hose, &rsrc);
		disp_name = "Bandit";
	} else if (strcmp(dev->name, "chaos") == 0) {
		setup_chaos(hose, &rsrc);
		disp_name = "Chaos";
		primary = 0;
	}
	printk(KERN_INFO "Found %s PCI host bridge at 0x%016llx. "
	       "Firmware bus number: %d->%d\n",
		disp_name, (unsigned long long)rsrc.start, hose->first_busno,
		hose->last_busno);
#endif /*              */

	DBG(" ->Hose at 0x%p, cfg_addr=0x%p,cfg_data=0x%p\n",
		hose, hose->cfg_addr, hose->cfg_data);

	/*                                 */
	/*                                                        */
	pci_process_bridge_OF_ranges(hose, dev, primary);

	/*                               */
	fixup_bus_range(dev);

	return 0;
}

void __devinit pmac_pci_irq_fixup(struct pci_dev *dev)
{
#ifdef CONFIG_PPC32
	/*                                                         
                                         
                                                            
                                                            
                                                             
                  
  */
	if (has_second_ohare &&
	    dev->vendor == PCI_VENDOR_ID_DEC &&
	    dev->device == PCI_DEVICE_ID_DEC_TULIP_PLUS) {
		dev->irq = irq_create_mapping(NULL, 60);
		irq_set_irq_type(dev->irq, IRQ_TYPE_LEVEL_LOW);
	}
#endif /*              */
}

void __init pmac_pci_init(void)
{
	struct device_node *np, *root;
	struct device_node *ht = NULL;

	pci_set_flags(PCI_CAN_SKIP_ISA_ALIGN);

	root = of_find_node_by_path("/");
	if (root == NULL) {
		printk(KERN_CRIT "pmac_pci_init: can't find root "
		       "of device tree\n");
		return;
	}
	for (np = NULL; (np = of_get_next_child(root, np)) != NULL;) {
		if (np->name == NULL)
			continue;
		if (strcmp(np->name, "bandit") == 0
		    || strcmp(np->name, "chaos") == 0
		    || strcmp(np->name, "pci") == 0) {
			if (pmac_add_bridge(np) == 0)
				of_node_get(np);
		}
		if (strcmp(np->name, "ht") == 0) {
			of_node_get(np);
			ht = np;
		}
	}
	of_node_put(root);

#ifdef CONFIG_PPC64
	/*                                                              
         
  */
	if (ht && pmac_add_bridge(ht) != 0)
		of_node_put(ht);

	/*                                             */
	pci_devs_phb_init();

	/*                                                                 
                                                                   
                                                                  
                 
  */
	if (u3_agp) {
		struct device_node *np = u3_agp->dn;
		PCI_DN(np)->busno = 0xf0;
		for (np = np->child; np; np = np->sibling)
			PCI_DN(np)->busno = 0xf0;
	}
	/*                       */

#else /*              */
	init_p2pbridge();
	init_second_ohare();
	fixup_nec_usb2();

	/*                                                             
                                                              
                                         
  */
	if (pci_has_flag(PCI_REASSIGN_ALL_BUS))
		pcibios_assign_bus_offset = 0x10;
#endif
}

#ifdef CONFIG_PPC32
int pmac_pci_enable_device_hook(struct pci_dev *dev)
{
	struct device_node* node;
	int updatecfg = 0;
	int uninorth_child;

	node = pci_device_to_OF_node(dev);

	/*                                                                
                             
  */
	if (dev->vendor == PCI_VENDOR_ID_APPLE
	    && dev->class == PCI_CLASS_SERIAL_USB_OHCI
	    && !node) {
		printk(KERN_INFO "Apple USB OHCI %s disabled by firmware\n",
		       pci_name(dev));
		return -EINVAL;
	}

	if (!node)
		return 0;

	uninorth_child = node->parent &&
		of_device_is_compatible(node->parent, "uni-north");

	/*                                                             
                                              
  */
	if (uninorth_child && !strcmp(node->name, "firewire") &&
	    (of_device_is_compatible(node, "pci106b,18") ||
	     of_device_is_compatible(node, "pci106b,30") ||
	     of_device_is_compatible(node, "pci11c1,5811"))) {
		pmac_call_feature(PMAC_FTR_1394_CABLE_POWER, node, 0, 1);
		pmac_call_feature(PMAC_FTR_1394_ENABLE, node, 0, 1);
		updatecfg = 1;
	}
	if (uninorth_child && !strcmp(node->name, "ethernet") &&
	    of_device_is_compatible(node, "gmac")) {
		pmac_call_feature(PMAC_FTR_GMAC_ENABLE, node, 0, 1);
		updatecfg = 1;
	}

	/*
                                                               
                                                              
                                                              
                            
  */
	if (updatecfg) {
		u16 cmd;

		pci_read_config_word(dev, PCI_COMMAND, &cmd);
		cmd |= PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER
			| PCI_COMMAND_INVALIDATE;
		pci_write_config_word(dev, PCI_COMMAND, cmd);
		pci_write_config_byte(dev, PCI_LATENCY_TIMER, 16);

		pci_write_config_byte(dev, PCI_CACHE_LINE_SIZE,
				      L1_CACHE_BYTES >> 2);
	}

	return 0;
}

void __devinit pmac_pci_fixup_ohci(struct pci_dev *dev)
{
	struct device_node *node = pci_device_to_OF_node(dev);

	/*                                                     
                                                     
  */
	if (dev->class == PCI_CLASS_SERIAL_USB_OHCI && !node)
		dev->resource[0].flags = 0;
}
DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_APPLE, PCI_ANY_ID, pmac_pci_fixup_ohci);

/*                                                                
                              
 */
void __init pmac_pcibios_after_init(void)
{
	struct device_node* nd;

	for_each_node_by_name(nd, "firewire") {
		if (nd->parent && (of_device_is_compatible(nd, "pci106b,18") ||
				   of_device_is_compatible(nd, "pci106b,30") ||
				   of_device_is_compatible(nd, "pci11c1,5811"))
		    && of_device_is_compatible(nd->parent, "uni-north")) {
			pmac_call_feature(PMAC_FTR_1394_ENABLE, nd, 0, 0);
			pmac_call_feature(PMAC_FTR_1394_CABLE_POWER, nd, 0, 0);
		}
	}
	for_each_node_by_name(nd, "ethernet") {
		if (nd->parent && of_device_is_compatible(nd, "gmac")
		    && of_device_is_compatible(nd->parent, "uni-north"))
			pmac_call_feature(PMAC_FTR_GMAC_ENABLE, nd, 0, 0);
	}
}

void pmac_pci_fixup_cardbus(struct pci_dev* dev)
{
	if (!machine_is(powermac))
		return;
	/*
                                                            
                      
  */
	if (dev->vendor != PCI_VENDOR_ID_TI)
		return;
	if (dev->device == PCI_DEVICE_ID_TI_1130 ||
	    dev->device == PCI_DEVICE_ID_TI_1131) {
		u8 val;
		/*                      */
		if (pci_read_config_byte(dev, 0x91, &val) == 0)
			pci_write_config_byte(dev, 0x91, val | 0x30);
		/*                            */
		if (pci_read_config_byte(dev, 0x92, &val) == 0)
			pci_write_config_byte(dev, 0x92, val & ~0x06);
	}
	if (dev->device == PCI_DEVICE_ID_TI_1210 ||
	    dev->device == PCI_DEVICE_ID_TI_1211 ||
	    dev->device == PCI_DEVICE_ID_TI_1410 ||
	    dev->device == PCI_DEVICE_ID_TI_1510) {
		u8 val;
		/*                                                
                               */
		if (pci_read_config_byte(dev, 0x8c, &val) == 0)
			pci_write_config_byte(dev, 0x8c, (val & ~0x0f) | 2);
		/*                            */
		if (pci_read_config_byte(dev, 0x92, &val) == 0)
			pci_write_config_byte(dev, 0x92, val & ~0x06);
	}
}

DECLARE_PCI_FIXUP_FINAL(PCI_VENDOR_ID_TI, PCI_ANY_ID, pmac_pci_fixup_cardbus);

void pmac_pci_fixup_pciata(struct pci_dev* dev)
{
       u8 progif = 0;

       /*
                                                                  
                    
        */
	if (!machine_is(powermac))
		return;

	/*                                           */
	if (dev->vendor == PCI_VENDOR_ID_PROMISE)
		switch(dev->device) {
		case PCI_DEVICE_ID_PROMISE_20246:
		case PCI_DEVICE_ID_PROMISE_20262:
		case PCI_DEVICE_ID_PROMISE_20263:
		case PCI_DEVICE_ID_PROMISE_20265:
		case PCI_DEVICE_ID_PROMISE_20267:
		case PCI_DEVICE_ID_PROMISE_20268:
		case PCI_DEVICE_ID_PROMISE_20269:
		case PCI_DEVICE_ID_PROMISE_20270:
		case PCI_DEVICE_ID_PROMISE_20271:
		case PCI_DEVICE_ID_PROMISE_20275:
		case PCI_DEVICE_ID_PROMISE_20276:
		case PCI_DEVICE_ID_PROMISE_20277:
			goto good;
		}
	/*                         */
	if ((dev->class >> 8) != PCI_CLASS_STORAGE_IDE)
		return;
 good:
	pci_read_config_byte(dev, PCI_CLASS_PROG, &progif);
	if ((progif & 5) != 5) {
		printk(KERN_INFO "PCI: %s Forcing PCI IDE into native mode\n",
		       pci_name(dev));
		(void) pci_write_config_byte(dev, PCI_CLASS_PROG, progif|5);
		if (pci_read_config_byte(dev, PCI_CLASS_PROG, &progif) ||
		    (progif & 5) != 5)
			printk(KERN_ERR "Rewrite of PROGIF failed !\n");
		else {
			/*                                        */
			pci_write_config_dword(dev, PCI_BASE_ADDRESS_0, 0);
			pci_write_config_dword(dev, PCI_BASE_ADDRESS_1, 0);
			pci_write_config_dword(dev, PCI_BASE_ADDRESS_2, 0);
			pci_write_config_dword(dev, PCI_BASE_ADDRESS_3, 0);
		}
	}
}
DECLARE_PCI_FIXUP_EARLY(PCI_ANY_ID, PCI_ANY_ID, pmac_pci_fixup_pciata);
#endif /*              */

/*
                                                  
                                   
 */
static void fixup_k2_sata(struct pci_dev* dev)
{
	int i;
	u16 cmd;

	if (PCI_FUNC(dev->devfn) > 0) {
		pci_read_config_word(dev, PCI_COMMAND, &cmd);
		cmd &= ~(PCI_COMMAND_IO | PCI_COMMAND_MEMORY);
		pci_write_config_word(dev, PCI_COMMAND, cmd);
		for (i = 0; i < 6; i++) {
			dev->resource[i].start = dev->resource[i].end = 0;
			dev->resource[i].flags = 0;
			pci_write_config_dword(dev, PCI_BASE_ADDRESS_0 + 4 * i,
					       0);
		}
	} else {
		pci_read_config_word(dev, PCI_COMMAND, &cmd);
		cmd &= ~PCI_COMMAND_IO;
		pci_write_config_word(dev, PCI_COMMAND, cmd);
		for (i = 0; i < 5; i++) {
			dev->resource[i].start = dev->resource[i].end = 0;
			dev->resource[i].flags = 0;
			pci_write_config_dword(dev, PCI_BASE_ADDRESS_0 + 4 * i,
					       0);
		}
	}
}
DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_SERVERWORKS, 0x0240, fixup_k2_sata);

/*
                                                                               
                                                                            
                                                                             
                                                                             
                                                    
  
                                                                            
                                                                              
                                                       
  
                                          
  
                                                                           
                                                                        
                                                                          
                                                                     
 */
static void fixup_u4_pcie(struct pci_dev* dev)
{
	struct pci_controller *host = pci_bus_to_host(dev->bus);
	struct resource *region = NULL;
	u32 reg;
	int i;

	/*                          */
	if (!machine_is(powermac))
		return;

	/*                              */
	for (i = 0; i < 3; i++) {
		struct resource *r = &host->mem_resources[i];
		if (!(r->flags & IORESOURCE_MEM))
			continue;
		/*                                                    
                                        
   */
		if (r->start >= 0xf0000000 && r->start < 0xf3000000)
			continue;
		if (!region || resource_size(r) > resource_size(region))
			region = r;
	}
	/*                     */
	if (region == 0)
		return;

	/*                  */
	printk(KERN_INFO "PCI: Fixup U4 PCIe bridge range: %pR\n", region);

	/*                                                               
                                                                 
                  
  */
	reg = ((region->start >> 16) & 0xfff0) | (region->end & 0xfff00000);
	pci_write_config_dword(dev, PCI_MEMORY_BASE, reg);
	pci_write_config_dword(dev, PCI_PREF_BASE_UPPER32, 0);
	pci_write_config_dword(dev, PCI_PREF_LIMIT_UPPER32, 0);
	pci_write_config_dword(dev, PCI_PREF_MEMORY_BASE, 0);
}
DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_APPLE, PCI_DEVICE_ID_APPLE_U4_PCIE, fixup_u4_pcie);