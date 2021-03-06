/*
                                                        
 */

/*
 * Voltage Status Reg GPIO_GPLR.
 */
#define GPIO_GPLR	(PKUNITY_GPIO_BASE + 0x0000)
/*
                               
 */
#define GPIO_GPDR	(PKUNITY_GPIO_BASE + 0x0004)
/*
                                
 */
#define GPIO_GPSR	(PKUNITY_GPIO_BASE + 0x0008)
/*
                                  
 */
#define GPIO_GPCR	(PKUNITY_GPIO_BASE + 0x000C)
/*
                                   
 */
#define GPIO_GRER	(PKUNITY_GPIO_BASE + 0x0010)
/*
                                  
 */
#define GPIO_GFER	(PKUNITY_GPIO_BASE + 0x0014)
/*
                             
 */
#define GPIO_GEDR	(PKUNITY_GPIO_BASE + 0x0018)
/*
                                        
 */
#define GPIO_GPIR	(PKUNITY_GPIO_BASE + 0x0020)

#define GPIO_MIN	(0)
#define GPIO_MAX	(27)

#define GPIO_GPIO(Nb)	(0x00000001 << (Nb))	/*              */
#define GPIO_GPIO0	GPIO_GPIO(0)	/*           */
#define GPIO_GPIO1	GPIO_GPIO(1)	/*           */
#define GPIO_GPIO2	GPIO_GPIO(2)	/*           */
#define GPIO_GPIO3	GPIO_GPIO(3)	/*           */
#define GPIO_GPIO4	GPIO_GPIO(4)	/*           */
#define GPIO_GPIO5	GPIO_GPIO(5)	/*           */
#define GPIO_GPIO6	GPIO_GPIO(6)	/*           */
#define GPIO_GPIO7	GPIO_GPIO(7)	/*           */
#define GPIO_GPIO8	GPIO_GPIO(8)	/*           */
#define GPIO_GPIO9	GPIO_GPIO(9)	/*           */
#define GPIO_GPIO10	GPIO_GPIO(10)	/*           */
#define GPIO_GPIO11	GPIO_GPIO(11)	/*           */
#define GPIO_GPIO12	GPIO_GPIO(12)	/*           */
#define GPIO_GPIO13	GPIO_GPIO(13)	/*           */
#define GPIO_GPIO14	GPIO_GPIO(14)	/*           */
#define GPIO_GPIO15	GPIO_GPIO(15)	/*           */
#define GPIO_GPIO16	GPIO_GPIO(16)	/*           */
#define GPIO_GPIO17	GPIO_GPIO(17)	/*           */
#define GPIO_GPIO18	GPIO_GPIO(18)	/*           */
#define GPIO_GPIO19	GPIO_GPIO(19)	/*           */
#define GPIO_GPIO20	GPIO_GPIO(20)	/*           */
#define GPIO_GPIO21	GPIO_GPIO(21)	/*           */
#define GPIO_GPIO22	GPIO_GPIO(22)	/*           */
#define GPIO_GPIO23	GPIO_GPIO(23)	/*           */
#define GPIO_GPIO24	GPIO_GPIO(24)	/*           */
#define GPIO_GPIO25	GPIO_GPIO(25)	/*           */
#define GPIO_GPIO26	GPIO_GPIO(26)	/*           */
#define GPIO_GPIO27	GPIO_GPIO(27)	/*           */

