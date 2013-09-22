#ifndef _COLORADO_H
#define _COLORADO_H
#define __LINUX_ENV__
//                      

#ifdef __LINUX_ENV__
#include <linux/gpio.h>
#include <linux/delay.h>

#define BYTE unsigned char
#define WORD unsigned short


#define keil_51_code


/*
                                                                              
                                                                                   
 */

/*                                                         */
/*
                                                                        
                                                                        
                                                                        
                                                                            

                       
                       
*/
//              
//                 
//               

#ifdef SET_INT_MODE_EN
#ifndef SLEEP_MILLI_SEC  

#define SLEEP_MILLI_SEC(nMilliSec) \
	do { \
		long timeout = (nMilliSec) * HZ / 1000; \
		while(timeout > 0) \
		{ \
			timeout = schedule_timeout(timeout); \
		} \
	}while(0);  
#endif  
#endif


#define AUX_ERR  1
#define AUX_OK   0


#define D(fmt, arg...) printk("<1>```%s:%d: " fmt, __FUNCTION__, __LINE__, ##arg)
#define debug_printf(fmt, arg...) printk(fmt,##arg)
#define debug_puts(fmt) printk(fmt)
#define delay_ms(time) mdelay(time)

#define MIPI_EN  1
#define EN_3D    1 
#define MIPI_LANE_SEL_0  0
#define MIPI_LANE_SEL_1  0
/*                      */
/*                            
                                       
                                           
                                                     
                                    */
#define AUD_IN_SEL_1  1    
#define AUD_IN_SEL_2  0
#define SSC_EN   1
#define HDCP_EN   1

#define BIST_EN 0 


void Anx7805_work_func(struct work_struct * work);

unsigned char SP_TX_Write_Reg(unsigned char dev,unsigned char offset, unsigned char d);
unsigned char SP_TX_Read_Reg(unsigned char dev,unsigned char offset, unsigned char *d);

int hardware_power_ctl(int stat);
int get_cable_detect_status(void);

#endif

#ifdef __KEIL51_ENV__

#include "..\I2C_intf.h"
#include "..\mcu.h"
#include "..\uart_int.h"
#include "..\timer.h"

#define keil_51_code code

int hardware_power_ctl(int stat);
int get_cable_detect_status(void);


#endif




#endif
