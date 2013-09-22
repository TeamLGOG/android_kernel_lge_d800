/*
** =========================================================================
** File:
**     ImmVibeSPI.c
**
** Description:
**     Device-dependent functions called by Immersion TSP API
**     to control PWM duty cycle, amp enable/disable, save IVT file, etc...
**
** Portions Copyright (c) 2008-2010 Immersion Corporation. All Rights Reserved.
**
** This file contains Original Code and/or Modifications of Original Code
** as defined in and that are subject to the GNU Public License v2 -
** (the 'License'). You may not use this file except in compliance with the
** License. You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA or contact
** TouchSenseSales@immersion.com.
**
** The Original Code and all software distributed under the License are
** distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
** EXPRESS OR IMPLIED, AND IMMERSION HEREBY DISCLAIMS ALL SUCH WARRANTIES,
** INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS
** FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. Please see
** the License for the specific language governing rights and limitations
** under the License.
** =========================================================================
*/

#include <linux/types.h>
#include <linux/err.h>
#include <mach/msm_iomap.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <mach/gpiomux.h>
#include <linux/clk.h>

#include <linux/regulator/consumer.h>
#include <linux/i2c.h>

#include <linux/lge_sm100.h>
#include <linux/of_gpio.h>
#include <mach/board_lge.h>

/*                           
                                      
                                       
                                      
  */

#define ImmPorting

#ifdef ImmPorting
#define DEVICE_NAME		"lge_sm100"

static struct clk *cam_gp1_clk;
static void __iomem *virt_bases_v = NULL;
#define MMSS_CC_GP1_CMD_RCGR(x) (void __iomem *)(virt_bases_v + (x))

#define REG_WRITEL(value, reg)		writel(value, reg)
#define REG_READL(reg)			readl(reg)

#define MMSS_CC_PWM_SET		0xFD8C3450
#define GCC_GP1_PWM_SET		0xFC401900

#define MMSS_CC_PWM_SIZE	SZ_1K

#define GPIO_LIN_MOTOR_EN 60
#define GPIO_LIN_MOTOR_PWM 27
/*                                       
                              
         */

#define GP_CLK_ID				0 /*          */
#define GP_CLK_M_DEFAULT		1
#if defined(CONFIG_MACH_MSM8974_VU3_KR)
#define GP_CLK_N_DEFAULT		110
#else
#define GP_CLK_N_DEFAULT		92
#endif
#define GP_CLK_D_MAX			GP_CLK_N_DEFAULT
#define GP_CLK_D_HALF			(GP_CLK_N_DEFAULT >> 1)

static int mmss_cc_n_default;
static int mmss_cc_d_max;
static int mmss_cc_d_half;
#if defined(CONFIG_MACH_MSM8974_VU3_KR)
VibeInt8 previous_nForce=0;
#endif

struct timed_vibrator_data {
	atomic_t gp1_clk_flag;
	int amp;
	int vibe_n_value;
	int haptic_en_gpio;
/*                                       
                     
        */
	int motor_pwm_gpio;
	int vpwr_on;
	struct regulator *vreg_l21;
};
struct timed_vibrator_data vib;
static DEFINE_MUTEX(vib_lock);

#ifdef CONFIG_OF
static void vibrator_parse_dt(struct device *dev, struct timed_vibrator_data *vib_data)
{
	struct device_node *np = dev->of_node;

	of_property_read_u32(np, "syncoam,vpwr-on", &vib_data->vpwr_on);
	INFO_MSG("[sm100] vib->vpwr_on : %d!!\n", vib_data->vpwr_on);

	vib_data->haptic_en_gpio = of_get_named_gpio_flags(np, "syncoam,haptic-pwr-gpio", 0, NULL);
	vib_data->motor_pwm_gpio = of_get_named_gpio_flags(np, "syncoam,motor-pwm-gpio", 0, NULL);
/*                                       
                                                                                             
        */

	of_property_read_u32(np, "syncoam,motor-amp", &vib_data->amp);
	of_property_read_u32(np, "syncoam,n-value", &vib_data->vibe_n_value);

	INFO_MSG("[sm100] gpio en : %d, pwm : %d, amp : %d, n_value : %d\n",
		   vib_data->haptic_en_gpio, vib_data->motor_pwm_gpio,
		   vib_data->amp, vib_data->vibe_n_value);
}

static struct of_device_id sm100_match_table[] = {
    { .compatible = "syncoam,sm100",},
    { },
};
#endif

static int vibrator_pwm_set(int enable, int amp, int n_value)
{
	/*                       */
	//                              
	//                          
	//                                                                        

	//                    
	//                                             
	uint d_val;
	//                                                       
	d_val = mmss_cc_d_half + (mmss_cc_n_default-1)*amp/256;

	if (virt_bases_v == NULL)
		virt_bases_v = ioremap(MMSS_CC_PWM_SET, MMSS_CC_PWM_SIZE);

	//                                             

	if (enable) {
		REG_WRITEL(
			((~(d_val << 1)) & 0xffU),	/*        */
			MMSS_CC_GP1_CMD_RCGR(0x10));
		REG_WRITEL(
			(1 << 1U) +	/*            */
			(1),		/*           */
			MMSS_CC_GP1_CMD_RCGR(0));
	} else {
		REG_WRITEL(
			(0 << 1U) +	/*            */
			(0),		/*           */
			MMSS_CC_GP1_CMD_RCGR(0));
	}


#if 0
	if (enable) {
		D_VAL = ((GP_CLK_D_MAX * amp) >> 7);
		if (D_VAL > GP_CLK_D_HALF) {
			if (D_VAL == GP_CLK_D_MAX) {      /*                 */
				D_VAL = 2;
			} else {
				D_VAL = GP_CLK_D_MAX - D_VAL;
			}
			D_INV = 1;
		}

		REG_WRITEL(
			(((M_VAL & 0xffU) << 16U) + /*              */
			((~(D_VAL << 1)) & 0xffU)),  /*            */
			GPn_MD_REG(GP_CLK_ID));

		REG_WRITEL(
			((((~(n_value-M_VAL)) & 0xffU) << 16U) + /*              */
			(1U << 11U) +  /*                               */
			((D_INV & 0x01U) << 10U) +  /*                                */
			(1U << 9U) +   /*                               */
			(1U << 8U) +   /*                               */
			(0U << 7U) +   /*                                   */
			(2U << 5U) +   /*                                       */
			(3U << 3U) +   /*                               */
			(5U << 0U)),   /*                              */
			GPn_NS_REG(GP_CLK_ID));
		//                                                                                    
	} else {
		REG_WRITEL(
			((((~(n_value-M_VAL)) & 0xffU) << 16U) + /*              */
			(0U << 11U) +  /*                                */
			(0U << 10U) +  /*                              */
			(0U << 9U) +	 /*                                */
			(0U << 8U) +   /*                                */
			(0U << 7U) +   /*                                   */
			(2U << 5U) +   /*                                       */
			(3U << 3U) +   /*                               */
			(5U << 0U)),   /*                              */
			GPn_NS_REG(GP_CLK_ID));
		//                                           
	}
#endif
	return 0;
}

static int android_vibrator_probe(struct platform_device *pdev)
{
	if (pdev->dev.of_node) {
		INFO_MSG("[sm100] probe : pdev->dev.of_node\n");
		vibrator_parse_dt(&pdev->dev, &vib);
	}

	if (vib.vpwr_on != 1) {
		if (!(vib.vreg_l21)) {
			vib.vreg_l21 = regulator_get(&pdev->dev, "vdd_ana");
			if (IS_ERR(vib.vreg_l21)) {
				pr_err("%s: regulator get of pm8941_l21 failed (%ld)\n",
						__func__, PTR_ERR(vib.vreg_l21));
				vib.vreg_l21 = NULL;
			}
		}
	}

	pdev->dev.init_name = "vibrator";
	printk("[sm100] dev->init_name : %s, dev->kobj : %s\n",
				pdev->dev.init_name, pdev->dev.kobj.name);

	cam_gp1_clk = clk_get(&pdev->dev, "cam_gp1_clk");
#if defined(CONFIG_MACH_MSM8974_G2_KR)
	if(lge_get_board_revno() >= HW_REV_E) {
		mmss_cc_n_default = 92;		/*                 */
		mmss_cc_d_max = mmss_cc_n_default;
		mmss_cc_d_half = (mmss_cc_n_default >> 1);
		clk_set_rate(cam_gp1_clk, 29813);
	} else {
		mmss_cc_n_default = 54;		/*                 */
		mmss_cc_d_max = mmss_cc_n_default;
		mmss_cc_d_half = (mmss_cc_n_default >> 1);
		clk_set_rate(cam_gp1_clk, 22222);
	}
#elif defined(CONFIG_MACH_MSM8974_G2_VZW) || defined(CONFIG_MACH_MSM8974_G2_ATT) || defined(CONFIG_MACH_MSM8974_G2_TEL_AU)
	if(lge_get_board_revno() >= HW_REV_D) {
		mmss_cc_n_default = 92;		/*                 */
		mmss_cc_d_max = mmss_cc_n_default;
		mmss_cc_d_half = (mmss_cc_n_default >> 1);
		clk_set_rate(cam_gp1_clk, 29813);
	} else {
		mmss_cc_n_default = 54;		/*                 */
		mmss_cc_d_max = mmss_cc_n_default;
		mmss_cc_d_half = (mmss_cc_n_default >> 1);
		clk_set_rate(cam_gp1_clk, 22222);
	}
#elif defined(CONFIG_MACH_MSM8974_G2_DCM) || defined(CONFIG_MACH_MSM8974_G2_SPR) || defined(CONFIG_MACH_MSM8974_G2_TMO_US) || defined(CONFIG_MACH_MSM8974_G2_CA) || defined(CONFIG_MACH_MSM8974_G2_OPEN_COM) || defined(CONFIG_MACH_MSM8974_G2_VDF_COM)
	if(lge_get_board_revno() >= HW_REV_C) {
		mmss_cc_n_default = 92;		/*                 */
		mmss_cc_d_max = mmss_cc_n_default;
		mmss_cc_d_half = (mmss_cc_n_default >> 1);
		clk_set_rate(cam_gp1_clk, 29813);
	} else {
		mmss_cc_n_default = 54;		/*                 */
		mmss_cc_d_max = mmss_cc_n_default;
		mmss_cc_d_half = (mmss_cc_n_default >> 1);
		clk_set_rate(cam_gp1_clk, 22222);
	}
#elif defined(CONFIG_MACH_MSM8974_VU3_KR)
		mmss_cc_n_default = 110; 	/*                 */
		mmss_cc_d_max = mmss_cc_n_default;
		mmss_cc_d_half = (mmss_cc_n_default >> 1);
		clk_set_rate(cam_gp1_clk, 29090);
#elif defined(CONFIG_MACH_MSM8974_Z_KR) || defined(CONFIG_MACH_MSM8974_Z_US)
	if(lge_get_board_revno() >= HW_REV_B) {
		mmss_cc_n_default = 82; 	/*                 */
		mmss_cc_d_max = mmss_cc_n_default;
		mmss_cc_d_half = (mmss_cc_n_default >> 1);
		clk_set_rate(cam_gp1_clk, 29268);
	} else {
		mmss_cc_n_default = 54;		/*                 */
		mmss_cc_d_max = mmss_cc_n_default;
		mmss_cc_d_half = (mmss_cc_n_default >> 1);
		clk_set_rate(cam_gp1_clk, 22222);
	}
#elif defined(CONFIG_MACH_MSM8974_Z_KDDI)
	mmss_cc_n_default = 82;     /*                 */
	mmss_cc_d_max = mmss_cc_n_default;
	mmss_cc_d_half = (mmss_cc_n_default >> 1);
	clk_set_rate(cam_gp1_clk, 29268);
#else
	mmss_cc_n_default = 54;		/*                 */
	mmss_cc_d_max = mmss_cc_n_default;
	mmss_cc_d_half = (mmss_cc_n_default >> 1);
	clk_set_rate(cam_gp1_clk, 22222);
#endif
	atomic_set(&vib.gp1_clk_flag, 0);

	return 0;
}

static int android_vibrator_remove(struct platform_device *pdev)
{
	return 0;
}

static void android_vibrator_shutdown(struct platform_device *pdev)
{
}

static int android_vibrator_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int android_vibrator_resume(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver android_vibrator_driver = {
	.probe = android_vibrator_probe,
	.remove = android_vibrator_remove,
	.shutdown = android_vibrator_shutdown,
	.suspend = android_vibrator_suspend,
	.resume = android_vibrator_resume,
	.driver = {
		.name = DEVICE_NAME,
#ifdef CONFIG_OF
		.of_match_table = sm100_match_table,
#endif
	},
};
#endif

#if 0
#define REG_WRITEL(value, reg)		writel(value, (MSM_CLK_CTL_BASE+reg))
#define REG_READL(reg)			readl((MSM_CLK_CTL_BASE+reg))

#define GPn_MD_REG(n)                           (0x2D00+32*(n))
#define GPn_NS_REG(n)                           (0x2D24+32*(n))
#endif

#ifdef IMMVIBESPIAPI
#undef IMMVIBESPIAPI
#endif
#define IMMVIBESPIAPI static

/*
                                       
*/
#define NUM_ACTUATORS 1

#define PWM_DUTY_MAX    579 /*                             */

static bool g_bAmpEnabled = false;


/*                

      */

/*                            */
static int vibrator_power_set(int enable, struct timed_vibrator_data *vib_data)
{
/*                                       
          

                                     

            
                                                      
     
                                                      
                                                  
                                                

        */
	int rc;

	INFO_MSG("pwr_enable=%d\n", enable);

	mutex_lock(&vib_lock);
	if (vib_data->vpwr_on != 1) {
		if (enable) {
			rc = regulator_enable(vib_data->vreg_l21);
			if (rc < 0)
				pr_err("%s: regulator_enable failed\n", __func__);
		} else {
			if (regulator_is_enabled(vib_data->vreg_l21) > 0) {
				rc = regulator_disable(vib_data->vreg_l21);
				if (rc < 0)
					pr_err("%s: regulator_disable failed\n", __func__);
			}
		}
	}
	mutex_unlock(&vib_lock);
/*       */
	return 0;
}

static int vibrator_ic_enable_set(int enable, struct timed_vibrator_data *vib_data)
{
	int gpio;

	INFO_MSG("ic_enable=%d\n", enable);

	if (enable)
		gpio_direction_output(vib_data->haptic_en_gpio, 1);
	else
		gpio_direction_output(vib_data->haptic_en_gpio, 0);

	gpio = gpio_get_value(vib_data->haptic_en_gpio);
	INFO_MSG("Haptic_EN_GPIO Value : %d\n", gpio);

	return 0;
}


/*
                                               
*/
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_AmpDisable(VibeUInt8 nActuatorIndex)
{

    if (g_bAmpEnabled)
    {

        DbgOut((KERN_DEBUG "ImmVibeSPI_ForceOut_AmpDisable.\n"));

        vibrator_ic_enable_set(0, &vib);
        vibrator_pwm_set(0, 0, GP_CLK_N_DEFAULT);
        vibrator_power_set(0, &vib);

	if (atomic_read(&vib.gp1_clk_flag) == 1) {
		clk_disable_unprepare(cam_gp1_clk);
		atomic_set(&vib.gp1_clk_flag, 0);
	}

        g_bAmpEnabled = false;
#if defined(CONFIG_MACH_MSM8974_VU3_KR)
		previous_nForce=0;
#endif

    }

    return VIBE_S_SUCCESS;
}

/*
                                             
*/
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_AmpEnable(VibeUInt8 nActuatorIndex)
{
    if (!g_bAmpEnabled)
    {
        DbgOut((KERN_DEBUG "ImmVibeSPI_ForceOut_AmpEnable.\n"));

	if (atomic_read(&vib.gp1_clk_flag) == 0) {
		clk_prepare_enable(cam_gp1_clk);
		atomic_set(&vib.gp1_clk_flag, 1);
	}

        vibrator_power_set(1, &vib);
        //                                         
        vibrator_ic_enable_set(1, &vib);

        g_bAmpEnabled = true;
#if defined(CONFIG_MACH_MSM8974_VU3_KR)
		previous_nForce=127;
#endif

    }

    return VIBE_S_SUCCESS;
}

/*
                                                                     
*/
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_Initialize(void)
{

	int rc;
#ifdef ImmPorting
	rc = platform_driver_register(&android_vibrator_driver);
#endif
    DbgOut((KERN_DEBUG "ImmVibeSPI_ForceOut_Initialize.\n"));

	/*                                          */
	/*                                                          
          
                                                                         
                    
  */


	/*                                       */
	rc = gpio_request(vib.haptic_en_gpio, "lin_motor_en");
	if (rc) {
		printk("GPIO_LIN_MOTOR_EN %d request failed\n", vib.haptic_en_gpio);
		return VIBE_E_FAIL;
	}

	/*           */
	rc = gpio_request(vib.motor_pwm_gpio, "lin_motor_pwm");
	if (unlikely(rc < 0)) {
		printk("not able to get gpio %d\n", vib.motor_pwm_gpio);
		return VIBE_E_FAIL;
	}
//                                       
	/*                                   */
	//                                                           
	//         
	//                                                                       
	//                    
	// 
//      
	/*                                      */
	/*                                                     
          
                                                                     
                     
  */

	/*           */
	/*                                                       
                      
                                                                       
 */

	vibrator_ic_enable_set(0, &vib);
	vibrator_pwm_set(0, 0, GP_CLK_N_DEFAULT);
	vibrator_power_set(0, &vib);

    g_bAmpEnabled = true;   /*                                                           */

    /*
                   
                                                                    
                                                                                      
                       
    */
    ImmVibeSPI_ForceOut_AmpDisable(0);

    return VIBE_S_SUCCESS;
}

/*
                                                                  
*/
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_Terminate(void)
{
    DbgOut((KERN_DEBUG "ImmVibeSPI_ForceOut_Terminate.\n"));

    /*
                   
                                                                    
                                                                                      
                       
    */
    ImmVibeSPI_ForceOut_AmpDisable(0);
#ifdef ImmPorting
    platform_driver_unregister(&android_vibrator_driver);
#endif
    return VIBE_S_SUCCESS;
}

/*
                                                     
*/
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_SetSamples(VibeUInt8 nActuatorIndex, VibeUInt16 nOutputSignalBitDepth, VibeUInt16 nBufferSizeInBytes, VibeInt8* pForceOutputBuffer)
{

    VibeInt8 nForce;

    switch (nOutputSignalBitDepth)
    {
        case 8:
            /*                                                  */
            if (nBufferSizeInBytes != 1) return VIBE_E_FAIL;

            nForce = pForceOutputBuffer[0];
            break;
        case 16:
            /*                                                  */
            if (nBufferSizeInBytes != 2) return VIBE_E_FAIL;

            /*                           */
            nForce = ((VibeInt16*)pForceOutputBuffer)[0] >> 8;
            break;
        default:
            /*                      */
            return VIBE_E_FAIL;
    }
#if defined(CONFIG_MACH_MSM8974_VU3_KR)
	if(nForce==previous_nForce)
		return VIBE_S_SUCCESS;
	previous_nForce=nForce;
#endif
    if (nForce == 0)
    {
        //                                         
        ImmVibeSPI_ForceOut_AmpDisable(nActuatorIndex);

    }
    else
    {
        ImmVibeSPI_ForceOut_AmpEnable(nActuatorIndex);
        vibrator_pwm_set(1, nForce, GP_CLK_N_DEFAULT);
    }

    return VIBE_S_SUCCESS;
}

#if 0
/*
                                                  
*/
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_SetFrequency(VibeUInt8 nActuatorIndex, VibeUInt16 nFrequencyParameterID, VibeUInt32 nFrequencyParameterValue)
{
    /*                                            */

    return VIBE_S_SUCCESS;
}
#endif

/*
                                                                            
*/
IMMVIBESPIAPI VibeStatus ImmVibeSPI_Device_GetName(VibeUInt8 nActuatorIndex, char *szDevName, int nSize)
{

    if ((!szDevName) || (nSize < 1)) return VIBE_E_FAIL;

    DbgOut((KERN_DEBUG "ImmVibeSPI_Device_GetName.\n"));
#ifdef ImmPorting
    strncpy(szDevName, "LGE A1", nSize-1);
    szDevName[nSize - 1] = '\0';    /*                                         */
#endif
    return VIBE_S_SUCCESS;
}
