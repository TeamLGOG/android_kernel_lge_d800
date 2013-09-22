/*
* Customer code to add GPIO control during WLAN start/stop
* Copyright (C) 1999-2012, Broadcom Corporation
* 
*      Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2 (the "GPL"),
* available at http://www.broadcom.com/licenses/GPLv2.php, with the
* following added to such license:
* 
*      As a special exception, the copyright holders of this software give you
* permission to link this software with independent modules, and to copy and
* distribute the resulting executable under terms of your choice, provided that
* you also meet, for each linked independent module, the terms and conditions of
* the license of that module.  An independent module is a module which is not
* derived from this software.  The special exception does not apply to any
* modifications of the software.
* 
*      Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*
* $Id: dhd_custom_gpio.c 353167 2012-08-24 22:11:30Z $
*/

#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>
#include <bcmutils.h>

#include <dngl_stats.h>
#include <dhd.h>

#include <wlioctl.h>
#include <wl_iw.h>

#define WL_ERROR(x) printf x
#define WL_TRACE(x)

#ifdef CUSTOMER_HW
extern  void bcm_wlan_power_off(int);
extern  void bcm_wlan_power_on(int);
#endif /*             */
#if defined(CUSTOMER_HW2) || defined(CUSTOMER_HW4)
#ifdef CONFIG_WIFI_CONTROL_FUNC
int wifi_set_power(int on, unsigned long msec);
int wifi_get_irq_number(unsigned long *irq_flags_ptr);
int wifi_get_mac_addr(unsigned char *buf);
void *wifi_get_country_code(char *ccode);
#else
int wifi_set_power(int on, unsigned long msec) { return -1; }
int wifi_get_irq_number(unsigned long *irq_flags_ptr) { return -1; }
int wifi_get_mac_addr(unsigned char *buf) { return -1; }
void *wifi_get_country_code(char *ccode) { return NULL; }
#endif /*                          */
#endif /*                              */

#if defined(OOB_INTR_ONLY) || defined(BCMSPI_ANDROID)

#if defined(BCMLXSDMMC)
extern int sdioh_mmc_irq(int irq);
#endif /*               */

#ifdef CUSTOMER_HW3
#include <mach/gpio.h>
#endif

/*                                        */
static int dhd_oob_gpio_num = -1;

module_param(dhd_oob_gpio_num, int, 0644);
MODULE_PARM_DESC(dhd_oob_gpio_num, "DHD oob gpio number");

/*                           
                                                                 
                                                              
  
          
                                                  
                               
                                                      
                                                         
  
 */
int dhd_customer_oob_irq_map(unsigned long *irq_flags_ptr)
{
	int  host_oob_irq = 0;

#if defined(CUSTOMER_HW2) || defined(CUSTOMER_HW4)
	host_oob_irq = wifi_get_irq_number(irq_flags_ptr);

#else
#if defined(CUSTOM_OOB_GPIO_NUM)
	if (dhd_oob_gpio_num < 0) {
		dhd_oob_gpio_num = CUSTOM_OOB_GPIO_NUM;
	}
#endif /*                       */

	if (dhd_oob_gpio_num < 0) {
		WL_ERROR(("%s: ERROR customer specific Host GPIO is NOT defined \n",
		__FUNCTION__));
		return (dhd_oob_gpio_num);
	}

	WL_ERROR(("%s: customer specific Host GPIO number is (%d)\n",
	         __FUNCTION__, dhd_oob_gpio_num));

#if defined CUSTOMER_HW
	host_oob_irq = MSM_GPIO_TO_INT(dhd_oob_gpio_num);
#elif defined CUSTOMER_HW3
	gpio_request(dhd_oob_gpio_num, "oob irq");
	host_oob_irq = gpio_to_irq(dhd_oob_gpio_num);
	gpio_direction_input(dhd_oob_gpio_num);
#endif /*             */
#endif /*                              */

	return (host_oob_irq);
}
#endif /*                                                   */

/*                                                     */
void
dhd_customer_gpio_wlan_ctrl(int onoff)
{
	switch (onoff) {
		case WLAN_RESET_OFF:
			WL_TRACE(("%s: call customer specific GPIO to insert WLAN RESET\n",
				__FUNCTION__));
#ifdef CUSTOMER_HW
			bcm_wlan_power_off(2);
#endif /*             */
#if defined(CUSTOMER_HW2) || defined(CUSTOMER_HW4)
			wifi_set_power(0, 0);
#endif
			WL_ERROR(("=========== WLAN placed in RESET ========\n"));
		break;

		case WLAN_RESET_ON:
			WL_TRACE(("%s: callc customer specific GPIO to remove WLAN RESET\n",
				__FUNCTION__));
#ifdef CUSTOMER_HW
			bcm_wlan_power_on(2);
#endif /*             */
#if defined(CUSTOMER_HW2) || defined(CUSTOMER_HW4)
			wifi_set_power(1, 0);
#endif
			WL_ERROR(("=========== WLAN going back to live  ========\n"));
		break;

		case WLAN_POWER_OFF:
			WL_TRACE(("%s: call customer specific GPIO to turn off WL_REG_ON\n",
				__FUNCTION__));
#ifdef CUSTOMER_HW
			bcm_wlan_power_off(1);
#endif /*             */
		break;

		case WLAN_POWER_ON:
			WL_TRACE(("%s: call customer specific GPIO to turn on WL_REG_ON\n",
				__FUNCTION__));
#ifdef CUSTOMER_HW
			bcm_wlan_power_on(1);
			/*                                   */
			OSL_DELAY(200);
#endif /*             */
		break;
	}
}

#ifdef GET_CUSTOM_MAC_ENABLE
/*                                    */
int
dhd_custom_get_mac_address(unsigned char *buf)
{
	int ret = 0;

	WL_TRACE(("%s Enter\n", __FUNCTION__));
	if (!buf)
		return -EINVAL;

	/*                                                             */
#if (defined(CUSTOMER_HW2) || defined(CUSTOMER_HW10)) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	ret = wifi_get_mac_addr(buf);
#endif

#ifdef EXAMPLE_GET_MAC
	/*              */
	{
		struct ether_addr ea_example = {{0x00, 0x11, 0x22, 0x33, 0x44, 0xFF}};
		bcopy((char *)&ea_example, buf, sizeof(struct ether_addr));
	}
#endif /*                 */

	return ret;
}
#endif /*                       */

#if !defined(CUSTOMER_HW4) || defined(CUSTOMER_HW10)
#ifdef CUSTOMER_HW10
#define EXAMPLE_TABLE
#endif
/*                                            */
const struct cntry_locales_custom translate_custom_table[] = {
/*                                                                            */
#ifdef EXAMPLE_TABLE
	{"",   "XY", 4},  /*                                               */
#if 0  //                                                      
	{"US", "US", 69}, /*                                  */
	{"CA", "US", 69}, /*                                  */
	{"EU", "EU", 5},  /*                                            */
	{"AT", "EU", 5},
	{"BE", "EU", 5},
	{"BG", "EU", 5},
	{"CY", "EU", 5},
	{"CZ", "EU", 5},
	{"DK", "EU", 5},
	{"EE", "EU", 5},
	{"FI", "EU", 5},
	{"FR", "EU", 5},
	{"DE", "EU", 5},
	{"GR", "EU", 5},
	{"HU", "EU", 5},
	{"IE", "EU", 5},
	{"IT", "EU", 5},
	{"LV", "EU", 5},
	{"LI", "EU", 5},
	{"LT", "EU", 5},
	{"LU", "EU", 5},
	{"MT", "EU", 5},
	{"NL", "EU", 5},
	{"PL", "EU", 5},
	{"PT", "EU", 5},
	{"RO", "EU", 5},
	{"SK", "EU", 5},
	{"SI", "EU", 5},
	{"ES", "EU", 5},
	{"SE", "EU", 5},
	{"GB", "EU", 5},
	{"KR", "XY", 3},
	{"AU", "XY", 3},
	{"CN", "XY", 3}, /*                                  */
	{"TW", "XY", 3},
	{"AR", "XY", 3},
	{"MX", "XY", 3},
	{"IL", "IL", 0},
	{"CH", "CH", 0},
	{"TR", "TR", 0},
	{"NO", "NO", 0},
#else

#if defined(CONFIG_MACH_APQ8064_GVAR_CMCC) //                                                                    
	{"AD", "IL", 10}, //       
	{"AE", "IL", 10}, //   
	{"AF", "IL" , 10}, //           
	{"AG", "US", 46}, //                 
	{"AI", "US", 46}, //        
	{"AL", "IL", 10}, //       
	{"AM", "IL", 10}, //       
	{"AN", "IL", 10}, //                    
	{"AO", "IL", 10}, //      
	{"AR", "IL", 10}, //         
	{"AS", "US", 46}, //                    
	{"AT", "IL", 10}, //       
	{"AU", "IL", 10}, //         
	{"AW", "IL", 10}, //     
	{"AZ", "IL", 10}, //          
	{"BA", "IL", 10}, //                      
	{"BB", "IL", 10}, //        
	{"BD", "IL", 10}, //          
	{"BE", "IL" , 10}, //       
	{"BF", "IL", 10}, //            
	{"BG", "IL", 10}, //        
	{"BH", "IL", 10}, //       
	{"BI", "IL" , 10}, //       
	{"BJ", "IL", 10}, //     
	{"BM", "US", 46}, //       
	{"BN", "IL", 10}, //      
	{"BO", "IL", 10}, //       
	{"BR", "IL", 10}, //      
	{"BS", "IL", 10}, //       
	{"BT", "IL", 10}, //      
	{"BW", "IL", 10}, //        
	{"BY", "IL", 10}, //       
	{"BZ", "IL" , 10}, //      
	{"CA", "US", 46}, //      
	{"CD", "IL", 10}, //                                 
	{"CF", "IL", 10}, //                        
	{"CG", "IL", 10}, //                      
	{"CH", "IL", 10}, //           
	{"CI", "IL", 10}, //             
	{"CK", "IL", 10}, //           
	{"CL", "IL", 10}, //     
	{"CM", "IL", 10}, //        
	{"CN", "IL", 10}, //     
	{"CO", "IL", 10}, //        
	{"CR", "IL", 10}, //          
	{"CU", "IL", 10}, //    
	{"CV", "IL", 10}, //          
	{"CX", "IL", 10}, //                            
	{"CY", "IL", 10}, //      
	{"CZ", "IL", 10}, //     
	{"DE", "IL", 10}, //       
	{"DJ", "IL", 10}, //        
	{"DK", "IL", 10}, //       
	{"DM", "IL", 10}, //        
	{"DO", "IL", 10}, //                  
	{"DZ", "IL", 10}, //       
	{"EC", "IL", 10}, //       
	{"EE", "IL", 10}, //       
	{"EG", "IL", 10}, //     
	{"ER", "IL", 10}, //       
	{"ES", "IL", 10}, //     
	{"ET", "IL", 10}, //        
	{"FI", "IL", 10}, //       
	{"FJ", "IL", 10}, //    
	{"FM", "US", 46}, //                              
	{"FO", "IL", 10}, //            
	{"FR", "IL", 10}, //      
	{"GA", "IL", 10}, //     
	{"GB", "IL", 10}, //              
	{"GD", "IL", 10}, //       
	{"GE", "IL", 10}, //       
	{"GF", "IL", 10}, //             
	{"GH", "IL", 10}, //     
	{"GI", "IL", 10}, //         
	{"GM", "IL", 10}, //      
	{"GN", "IL", 10}, //      
	{"GP", "IL", 10}, //          
	{"GQ", "IL", 10}, //                 
	{"GR", "IL", 10}, //      
	{"GT", "IL", 10}, //         
	{"GU", "US", 46}, //    
	{"GW", "IL", 10}, //             
	{"GY", "IL", 10}, //      
	{"HK", "IL", 10}, //         
	{"HN", "IL" , 10}, //        
	{"HR", "IL", 10}, //       
	{"HT", "IL", 10}, //     
	{"HU", "IL", 10}, //       
	{"ID", "IL", 10}, //         
	{"IE", "IL", 10}, //       
	{"IL", "IL" , 10}, //      
	{"IM", "IL", 10}, //           
	{"IN", "IL", 10}, //     
	{"IQ", "IL", 10}, //    
	{"IR", "IL", 10}, //    
	{"IS", "IL", 10}, //       
	{"IT", "IL", 10}, //     
	{"JE", "IL", 10}, //      
	{"JM", "IL", 10}, //       
	{"JO", "IL", 10}, //      
	{"JP", "IL", 10}, //     
	{"KE", "IL", 10}, //     
	{"KG", "IL", 10}, //          
	{"KH", "IL", 10}, //        
	{"KI", "IL", 10}, //        
	{"KM", "IL", 10}, //       
	{"KP", "IL", 10}, //           
	{"KR", "IL", 10}, //           
	{"KW", "IL", 10}, //      
	{"KY", "US", 46}, //              
	{"KZ", "IL", 10}, //          
	{"LA", "IL", 10}, //    
	{"LB", "IL" , 10}, //       
	{"LC", "IL", 10}, //           
	{"LI", "IL", 10}, //             
	{"LK", "IL" , 10}, //         
	{"LR", "IL", 10}, //       
	{"LS", "IL", 10}, //       
	{"LT", "IL", 10}, //         
	{"LU", "IL", 10}, //         
	{"LV", "IL", 10}, //      
	{"LY", "IL", 10}, //     
	{"MA", "IL", 1}, //       
	{"MC", "IL", 10}, //      
	{"MD", "IL", 10}, //       
	{"ME", "IL", 10}, //          
	{"MF", "IL", 10}, //                                                    
	{"MG", "IL" , 10}, //          
	{"MH", "IL", 10}, //                
	{"MK", "IL", 10}, //         
	{"ML", "IL" , 10}, //    
	{"MM", "IL", 10}, //               
	{"MN", "IL", 10}, //        
	{"MO", "IL", 10}, //     
	{"MP", "US", 46}, //                                                                
	{"MQ", "IL", 10}, //                   
	{"MR", "IL", 10}, //          
	{"MS", "IL", 10}, //               
	{"MT", "IL", 10}, //     
	{"MU", "IL", 10}, //         
	{"MV", "IL", 10}, //        
	{"MW", "IL", 10}, //      
	{"MX", "IL", 10}, //      
	{"MY", "IL", 10}, //        
	{"MZ", "IL", 10}, //          
	{"NA", "IL", 10}, //       
	{"NC", "IL", 10}, //             
	{"NE", "IL", 10}, //     
	{"NF", "IL", 10}, //              
	{"NG", "IL", 10}, //       
	{"NI", "IL" , 10}, //         
	{"NL", "IL", 10}, //           
	{"NO", "IL", 10}, //      
	{"NP", "IL", 10}, //     
	{"NR", "IL", 10}, //     
	{"NU", "IL", 10}, //    
	{"NZ", "IL", 10 }, //           
	{"OM", "IL", 10}, //    
	{"PA", "IL", 10}, //      
	{"PE", "IL", 10}, //    
	{"PF", "IL", 10}, //                         
	{"PG", "IL", 10}, //                
	{"PH", "IL", 10}, //           
	{"PK", "IL", 10}, //        
	{"PL", "IL", 10}, //      
	{"PM", "IL", 10}, //                         
	{"PN", "IL", 10}, //                
	{"PR", "US", 46}, //                 
	{"PS", "IL", 10}, //                     
	{"PT", "IL", 10}, //        
	{"PW", "IL", 10}, //     
	{"PY", "IL", 10}, //        
	{"QA", "IL", 10}, //     
	{"RE", "IL", 10}, //                
	{"RKS", "IL", 10}, //                              
	{"RO", "IL", 10}, //       
	{"RS", "IL", 10}, //      
	{"RU", "IL", 10}, //      
	{"RW", "IL", 10}, //      
	{"SA", "IL", 10}, //            
	{"SB", "IL", 10}, //               
	{"SC", "IL", 10}, //          
	{"SD", "IL", 10}, //     
	{"SE", "IL", 10}, //      
	{"SG", "IL", 10}, //         
	{"SI", "IL", 10}, //        
	{"SK", "IL", 10}, //        
	{"SKN", "IL", 10}, //                     
	{"SL", "IL", 10}, //            
	{"SM", "IL", 10}, //          
	{"SN", "IL", 10}, //       
	{"SO", "IL", 10}, //       
	{"SR", "IL", 10}, //        
	{"SS", "IL", 10}, //           
	{"ST", "IL", 10}, //                     
	{"SV", "IL", 10}, //           
	{"SY", "IL", 10}, //     
	{"SZ", "IL", 10}, //         
	{"TC", "IL", 10}, //                             
	{"TD", "IL", 10}, //    
	{"TF", "IL", 10}, //                                    
	{"TG", "IL", 10}, //    
	{"TH", "IL", 10}, //        
	{"TJ", "IL", 10}, //          
	{"TL", "IL", 10}, //          
	{"TM", "IL", 10}, //            
	{"TN", "IL", 10}, //       
	{"TO", "IL", 10}, //     
	{"TR", "IL", 10}, //      
	{"TT", "IL", 10}, //                   
	{"TV", "IL", 10}, //      
	{"TW", "US", 46}, //      
	{"TZ", "IL", 10}, //        
	{"UA", "IL", 10}, //       
	{"UG", "IL", 10}, //      
	{"US", "US", 46}, //  
	{"UY", "IL", 10}, //       
	{"UZ", "IL", 10}, //          
	{"VA", "IL", 10}, //                  
	{"VC", "IL", 10}, //                                
	{"VE", "IL", 10}, //         
	{"VG", "IL", 10}, //                      
	{"VI", "US", 46}, //                 
	{"VN", "IL", 10}, //       
	{"VU", "IL", 10}, //       
	{"WS", "IL", 10}, //     
	{"YE", "IL", 10}, //     
	{"YT", "IL", 10}, //                
	{"ZA", "IL", 10}, //            
	{"ZM", "IL", 10}, //      
	{"ZW", "IL", 10}, //        
#else
	{"AD", "GB", 0}, //       
	{"AE", "KR", 24}, //   
	{"AF", "AF" , 0}, //           
	{"AG", "US", 100}, //                 
	{"AI", "US", 100}, //        
	{"AL", "GB", 0}, //       
	{"AM", "IL", 10}, //       
	{"AN", "BR", 0}, //                    
	{"AO", "IL", 10}, //      
	{"AR", "BR", 0}, //         
	{"AS", "US", 100}, //                    
	{"AT", "GB", 0}, //       
	{"AU", "AU", 2}, //         
	{"AW", "KR", 24}, //     
	{"AZ", "BR", 0}, //          
	{"BA", "GB", 0}, //                      
	{"BB", "RU", 1}, //        
	{"BD", "CN", 0}, //          
	{"BE", "GB" , 0}, //       
	{"BF", "CN", 0}, //            
	{"BG", "GB", 0}, //        
	{"BH", "RU", 1}, //       
	{"BI", "IL" , 10}, //       
	{"BJ", "IL", 10}, //     
	{"BM", "US", 100}, //       
	{"BN", "RU", 1}, //      
	{"BO", "IL", 10}, //       
	{"BR", "BR", 0}, //      
	{"BS", "RU", 1}, //       
	{"BT", "IL", 10}, //      
	{"BW", "GB", 0}, //        
	{"BY", "GB", 0}, //       
	{"BZ", "IL" , 10}, //      
	{"CA", "US", 100}, //      
	{"CD", "IL", 10}, //                                 
	{"CF", "IL", 10}, //                        
	{"CG", "IL", 10}, //                      
	{"CH", "GB", 0}, //           
	{"CI", "IL", 10}, //             
	{"CK", "BR", 0}, //           
	{"CL", "RU", 1}, //     
	{"CM", "IL", 10}, //        
	{"CN", "CN", 0}, //     
	{"CO", "BR", 0}, //        
	{"CR", "BR", 0}, //          
	{"CU", "BR", 0}, //    
	{"CV", "GB", 0}, //          
	{"CX", "AU", 2}, //                            
	{"CY", "GB", 0}, //      
	{"CZ", "GB", 0}, //     
	{"DE", "GB", 0}, //       
	{"DJ", "IL", 10}, //        
	{"DK", "GB", 0}, //       
	{"DM", "BR", 0}, //        
	{"DO", "BR", 0}, //                  
	{"DZ", "KW", 1}, //       
	{"EC", "BR", 0}, //       
	{"EE", "GB", 0}, //       
	{"EG", "RU", 1}, //     
	{"ER", "IL", 10}, //       
	{"ES", "GB", 0}, //     
	{"ET", "GB", 0}, //        
	{"FI", "GB", 0}, //       
	{"FJ", "IL", 10}, //    
	{"FM", "US", 100}, //                              
	{"FO", "GB", 0}, //            
	{"FR", "GB", 0}, //      
	{"GA", "IL", 10}, //     
	{"GB", "GB", 0}, //              
	{"GD", "BR", 0}, //       
	{"GE", "GB", 0}, //       
	{"GF", "GB", 0}, //             
	{"GH", "BR", 0}, //     
	{"GI", "GB", 0}, //         
	{"GM", "IL", 10}, //      
	{"GN", "IL", 10}, //      
	{"GP", "GB", 0}, //          
	{"GQ", "IL", 10}, //                 
	{"GR", "GB", 0}, //      
	{"GT", "RU", 1}, //         
	{"GU", "US", 100}, //    
	{"GW", "IL", 10}, //             
	{"GY", "QA", 0}, //      
	{"HK", "BR", 0}, //         
	{"HN", "CN" , 0}, //        
	{"HR", "GB", 0}, //       
	{"HT", "RU", 1}, //     
	{"HU", "GB", 0}, //       
	{"ID", "QA", 0}, //         
	{"IE", "GB", 0}, //       
	{"IL", "IL" , 10}, //      
	{"IM", "GB", 0}, //           
	{"IN", "RU", 1}, //     
	{"IQ", "IL", 10}, //    
	{"IR", "IL", 10}, //    
	{"IS", "GB", 0}, //       
	{"IT", "GB", 0}, //     
	{"JE", "GB", 0}, //      
	{"JM", "GB", 0}, //       
	{"JO", "XY", 3}, //      
	{"JP", "JP", 5}, //     
	{"KE", "GB", 0}, //     
	{"KG", "IL", 10}, //          
	{"KH", "BR", 0}, //        
	{"KI", "AU", 2}, //        
	{"KM", "IL", 10}, //       
	{"KP", "IL", 10}, //           
	{"KR", "KR", 24}, //           
	{"KW", "KW", 1}, //      
	{"KY", "US", 100}, //              
	{"KZ", "BR", 0}, //          
	{"LA", "KR", 24}, //    
	{"LB", "BR" , 0}, //       
	{"LC", "BR", 0}, //           
	{"LI", "GB", 0}, //             
	{"LK", "BR" , 0}, //         
	{"LR", "BR", 0}, //       
	{"LS", "GB", 0}, //       
	{"LT", "GB", 0}, //         
	{"LU", "GB", 0}, //         
	{"LV", "GB", 0}, //      
	{"LY", "IL", 10}, //     
	{"MA", "KW", 1}, //       
	{"MC", "GB", 0}, //      
	{"MD", "GB", 0}, //       
	{"ME", "GB", 0}, //          
	{"MF", "GB", 0}, //                                                    
	{"MG", "IL" , 10}, //          
	{"MH", "BR", 0}, //                
	{"MK", "GB", 0}, //         
	{"ML", "IL" , 10}, //    
	{"MM", "IL", 10}, //               
	{"MN", "IL", 10}, //        
	{"MO", "CN", 0}, //     
	{"MP", "US", 100}, //                                                                
	{"MQ", "GB", 0}, //                   
	{"MR", "GB", 0}, //          
	{"MS", "GB", 0}, //               
	{"MT", "GB", 0}, //     
	{"MU", "GB", 0}, //         
	{"MV", "RU", 1}, //        
	{"MW", "CN", 0}, //      
	{"MX", "RU", 1}, //      
	{"MY", "RU", 1}, //        
	{"MZ", "BR", 0}, //          
	{"NA", "BR", 0}, //       
	{"NC", "IL", 10}, //             
	{"NE", "BR", 0}, //     
	{"NF", "BR", 0}, //              
	{"NG", "NG", 0}, //       
	{"NI", "BR" , 0}, //         
	{"NL", "GB", 0}, //           
	{"NO", "GB", 0}, //      
	{"NP", "SA", 0}, //     
	{"NR", "IL", 10}, //     
	{"NU", "BR", 0}, //    
	{"NZ", "BR", 0 }, //           
	{"OM", "GB", 0}, //    
	{"PA", "RU", 1}, //      
	{"PE", "BR", 0}, //    
	{"PF", "GB", 0}, //                         
	{"PG", "XY", 3}, //                
	{"PH", "BR", 0}, //           
	{"PK", "CN", 0}, //        
	{"PL", "GB", 0}, //      
	{"PM", "GB", 0}, //                         
	{"PN", "GB", 0}, //                
	{"PR", "US", 100}, //                 
	{"PS", "BR", 0}, //                     
	{"PT", "GB", 0}, //        
	{"PW", "BR", 0}, //     
	{"PY", "BR", 0}, //        
	{"QA", "CN", 0}, //     
	{"RE", "GB", 0}, //                
	{"RKS", "IL", 10}, //                              
	{"RO", "GB", 0}, //       
	{"RS", "GB", 0}, //      
	{"RU", "RU", 10}, //      
	{"RW", "CN", 0}, //      
	{"SA", "SA", 0}, //            
	{"SB", "IL", 10}, //               
	{"SC", "IL", 10}, //          
	{"SD", "GB", 0}, //     
	{"SE", "GB", 0}, //      
	{"SG", "BR", 0}, //         
	{"SI", "GB", 0}, //        
	{"SK", "GB", 0}, //        
	{"SKN", "CN", 0}, //                     
	{"SL", "IL", 10}, //            
	{"SM", "GB", 0}, //          
	{"SN", "GB", 0}, //       
	{"SO", "IL", 10}, //       
	{"SR", "IL", 10}, //        
	{"SS", "GB", 0}, //           
	{"ST", "IL", 10}, //                     
	{"SV", "RU", 1}, //           
	{"SY", "BR", 0}, //     
	{"SZ", "IL", 10}, //         
	{"TC", "GB", 0}, //                             
	{"TD", "IL", 10}, //    
	{"TF", "GB", 0}, //                                    
	{"TG", "IL", 10}, //    
	{"TH", "BR", 0}, //        
	{"TJ", "IL", 10}, //          
	{"TL", "BR", 0}, //          
	{"TM", "IL", 10}, //            
	{"TN", "KW", 1}, //       
	{"TO", "IL", 10}, //     
	{"TR", "GB", 0}, //      
	{"TT", "BR", 0}, //                   
	{"TV", "IL", 10}, //      
	{"TW", "TW", 2}, //      
	{"TZ", "CN", 0}, //        
	{"UA", "RU", 1}, //       
	{"UG", "BR", 0}, //      
	{"US", "US", 100}, //  
	{"UY", "BR", 0}, //       
	{"UZ", "IL", 10}, //          
	{"VA", "GB", 0}, //                  
	{"VC", "BR", 0}, //                                
	{"VE", "RU", 1}, //         
	{"VG", "GB", 0}, //                      
	{"VI", "US", 100}, //                 
	{"VN", "BR", 0}, //       
	{"VU", "IL", 10}, //       
	{"WS", "SA", 0}, //     
	{"YE", "IL", 10}, //     
	{"YT", "GB", 0}, //                
	{"ZA", "GB", 0}, //            
	{"ZM", "RU", 1}, //      
	{"ZW", "BR", 0}, //        
#endif //                                                                 
#endif //                                                      
#endif /*               */
};


/*                            
                                          
                           
*/
void get_customized_country_code(char *country_iso_code, wl_country_t *cspec)
{
//                                                    
#if defined(CUSTOMER_HW2) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39)) && !defined(CUSTOMER_HW10) 

	struct cntry_locales_custom *cloc_ptr;

	if (!cspec)
		return;

	cloc_ptr = wifi_get_country_code(country_iso_code);
	if (cloc_ptr) {
		strlcpy(cspec->ccode, cloc_ptr->custom_locale, WLC_CNTRY_BUF_SZ);
		cspec->rev = cloc_ptr->custom_locale_rev;
	}
	return;
#else
	int size, i;

	size = ARRAYSIZE(translate_custom_table);

	if (cspec == 0)
		 return;

	if (size == 0)
		 return;

	for (i = 0; i < size; i++) {
		if (strcmp(country_iso_code, translate_custom_table[i].iso_abbrev) == 0) {
			memcpy(cspec->ccode,
				translate_custom_table[i].custom_locale, WLC_CNTRY_BUF_SZ);
			cspec->rev = translate_custom_table[i].custom_locale_rev;

			WL_TRACE(("%s: finally set country code : country_iso_code(%s), ccode(%s), rev(%d) \n"
				,__FUNCTION__, country_iso_code, cspec->ccode, cspec->rev));

			return;
		}
	}
//                                                                                     
#if 0
#ifdef EXAMPLE_TABLE
	/*                                                                                    */
	memcpy(cspec->ccode, translate_custom_table[0].custom_locale, WLC_CNTRY_BUF_SZ);
	cspec->rev = translate_custom_table[0].custom_locale_rev;
#endif /*               */
#endif
//                                                                                     
	return;
#endif /*                                                                           */
}
#endif /*                                */
