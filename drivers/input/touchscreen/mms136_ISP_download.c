/*                                                    */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/irq.h>

#include "mms136_ISP_download.h"

#if defined(CONFIG_USING_INNOTEK_PANEL_3_5) || defined(CONFIG_USING_INNOTEK_PANEL_4_3)
#include <linux/regulator/driver.h>
#include <linux/regulator/gpio-regulator.h>
#include <linux/mfd/pm8xxx/pm8921.h>
#include <mach/board_lge.h>
#endif
/*                                                            
  
                                                                
  
                   
                                                            
                                     
  
                                                            */

#if defined(CONFIG_USING_INNOTEK_PANEL_4_5) /*     */
#include "MDH_LD1LF_R10_VA5.c" /*             */
#include "MDH_LD1LF_R10_V09.c" /*       */
#elif defined(CONFIG_USING_INNOTEK_PANEL_4_7)  /*     */
/*                                                              */
/*                                                                                                 */
/*                                                        */
/*                                                                   */
/*                                                                   */
/*                                                                                        */
/*                                                                                        */
#include "MTH-LD1LVF_RC20_C_V12.c" /*                                              */
#include "MTH-LD1LVF_RC20_D_V12.c" /*                                              */

#elif defined(CONFIG_USING_TOVIS_PANEL) /*     */
/*                                                                                 */
#include "MCH-LD1LKT_B_Vcb.c"   /*                                                           */
/*                                                                    */
/*                                                                     */
#include "MDH-LD1LKF_RO13_VO07.c" /*                                   */
#elif defined(CONFIG_USING_INNOTEK_PANEL_3_5) /*         */
#include "L0-R00-V04_bin.c" /*       */
#include "L0_R01_V15_02_bin.c" /*            */
#elif defined(CONFIG_USING_INNOTEK_PANEL_4_3) /*        */
#include "MTH_LL1_V0D_US.c"
#include "MTH_LL1_V51_US_Dummy.c"
#endif

extern UINT16 MELFAS_binary_nLength;
extern UINT8 *MELFAS_binary;

UINT8  ucVerifyBuffer[MELFAS_TRANSFER_LENGTH];          /*                                                */

/*                                 
                             
                                 */
/*                                                                                  */

static void mcsdl_set_ready(void);
static void mcsdl_reboot_mcs(void);

static int  mcsdl_erase_flash(INT8 IdxNum);
static int  mcsdl_program_flash(UINT8 *pDataOriginal, UINT16 unLength, INT8 IdxNum);
static void mcsdl_program_flash_part(UINT8 *pData);

static int  mcsdl_verify_flash(UINT8 *pData, UINT16 nLength, INT8 IdxNum);

static void mcsdl_read_flash(UINT8 *pBuffer);
static int  mcsdl_read_flash_from(UINT8 *pBuffer, UINT16 unStart_addr, UINT16 unLength, INT8 IdxNum);

static void mcsdl_select_isp_mode(UINT8 ucMode);
static void mcsdl_unselect_isp_mode(void);

static int  mcsdl_program_flash_sys(UINT8 *pDataOriginal, UINT16 unLength, INT8 IdxNum);
static int  mcsdl_verify_flash_sys(UINT8 *pData, UINT16 nLength, INT8 IdxNum);
static void  mcsdl_read_flash_sys(UINT8 *pBuffer);
static void mcsdl_select_isp_mode_sys(UINT8 ucMode);
static void mcsdl_unselect_isp_mode_sys(void);

static void mcsdl_read_32bits(UINT8 *pData);
static void mcsdl_write_bits(UINT32 wordData, int nBits);
/*                                          */

/*                                 
                       
                                   */
void mcsdl_delay(UINT32 nCount);

/*                                 
                             
                                   */
#if MELFAS_ENABLE_DBG_PRINT
static void mcsdl_print_result(int nRet);
#endif


/*                                  
                          
                                    */
#if MELFAS_USE_PROTOCOL_COMMAND_FOR_DOWNLOAD
void melfas_send_download_enable_command(void)
{
    /*                      */
}
#endif

#if defined(CONFIG_USING_INNOTEK_PANEL_3_5)
void vdd_set_on_off(bool on)
{
	int rc = -EINVAL;
	static struct regulator *vreg_l17;
	if (lge_get_board_revno() >= HW_REV_B) {
		vreg_l17 = regulator_get(NULL, "8921_l17");	/*               */
		if (IS_ERR(vreg_l17)) {
			pr_err("%s: regulator get of touch_3p0 failed (%ld)\n"
				, __func__, PTR_ERR(vreg_l17));
			rc = PTR_ERR(vreg_l17);
			return ;
		}
		rc = regulator_set_voltage(vreg_l17,
			MELFAS_VD33_MIN_UV, MELFAS_VD33_MAX_UV);

		if (on) {
			rc = regulator_set_optimum_mode(vreg_l17, MELFAS_VD33_CURR_UA);
			rc = regulator_enable(vreg_l17);
		} else {
			rc = regulator_disable(vreg_l17);
		}
	} else {
		gpio_set_value(GPIO_TOUCH_EN, on);
	}
}
#elif defined(CONFIG_USING_INNOTEK_PANEL_4_3)
void vdd_set_on_off(bool on)
{
	int rc = -EINVAL;
	static struct regulator *vreg_l17;
	vreg_l17 = regulator_get(NULL, "8921_l17");	/*               */
	if (IS_ERR(vreg_l17)) {
		pr_err("%s: regulator get of touch_3p0 failed (%ld)\n"
			, __func__, PTR_ERR(vreg_l17));
		rc = PTR_ERR(vreg_l17);
		return ;
	}
	rc = regulator_set_voltage(vreg_l17,
		MELFAS_VD33_MIN_UV, MELFAS_VD33_MAX_UV);

	if (on) {
		rc = regulator_set_optimum_mode(vreg_l17, MELFAS_VD33_CURR_UA);
		rc = regulator_enable(vreg_l17);
	} else {
		rc = regulator_disable(vreg_l17);
	}
}
#endif

/*                                                            
  
                               
  
                                                                             
                                          
                                    
  
  
                                                            */

int mms100_ISP_download_binary_data(int dl_mode)
{
	int nRet = 0;
	int retry_cnt = 0;

#if 0
#if MELFAS_USE_PROTOCOL_COMMAND_FOR_DOWNLOAD
	melfas_send_download_enable_command();
	mcsdl_delay(MCSDL_DELAY_100US);
#endif

	MELFAS_DISABLE_BASEBAND_ISR();			/*                                       */
	MELFAS_DISABLE_WATCHDOG_TIMER_RESET();		/*                                 */
#endif

	/*                        
                
                           */

	for (retry_cnt = 0; retry_cnt < 5; retry_cnt++) {
		if (dl_mode == 0x01)/*                     */
			nRet = mcsdl_download((const UINT8 *) MELFAS_binary, (const UINT16)MELFAS_binary_nLength , 0);
		else /*                     */
			nRet = mcsdl_download((const UINT8 *) MELFAS_binary, (const UINT16)MELFAS_binary_nLength , 0);
#if MELFAS_2CHIP_DOWNLOAD_ENABLE
		if (!nRet) {
			if (dl_mode == 0x01) /*                     */
				nRet = mcsdl_download((const UINT8 *) MELFAS_binary, (const UINT16)MELFAS_binary_nLength, 1); /*                            */
			else /*                     */
				nRet = mcsdl_download((const UINT8 *) MELFAS_binary, (const UINT16)MELFAS_binary_nLength , 1);
		}
#endif
		if (!nRet)
			break;
	}

	MELFAS_ROLLBACK_BASEBAND_ISR();			/*                                       */
	MELFAS_ROLLBACK_WATCHDOG_TIMER_RESET();		/*                                 */
/*
         
            
                       
                       
  */
	return nRet;
}

#if 0
int mms100_ISP_download_binary_file(void)
{
	int nRet;
	int i;

	UINT8  *pBinary[2] = {NULL, NULL};
	UINT16 nBinary_length[2] = {0, 0};
	UINT8 IdxNum = MELFAS_2CHIP_DOWNLOAD_ENABLE;
	/*                                                  
   
                            
                                                                  
                                                        
                                                                    
                                                            
                                                                                 
                                           
                                     
   
                                                     */


	/*                                                     
                                                 */

	FILE *fp;
	INT  nRead;

	/*                              
               
                                 */

	if (fopen(fp, "MELFAS_FIRMWARE.bin", "rb") == NULL) {
		return MCSDL_RET_FILE_ACCESS_FAILED;
	}

	/*                              
                   
                                 */

	fseek(fp, 0, SEEK_END);
	nBinary_length = (UINT16)ftell(fp);

	/*                              
                     
                                 */

	pBinary = (UINT8 *)malloc((INT)nBinary_length);
	if (pBinary == NULL) {
		return MCSDL_RET_FILE_ACCESS_FAILED;
	}

	/*                              
                    
                                 */

	fseek(fp, 0, SEEK_SET);
	nRead = fread(pBinary, 1, (INT)nBinary_length, fp);             /*                  */
	if (nRead != (INT)nBinary_length) {
		fclose(fp);                                             /*            */
		if (pBinary != NULL)                                    /*                      */
			free(pBinary);
		return MCSDL_RET_FILE_ACCESS_FAILED;
	}

	/*                              
              
                                 */

	fclose(fp);

#if MELFAS_USE_PROTOCOL_COMMAND_FOR_DOWNLOAD
	melfas_send_download_enable_command();
	mcsdl_delay(MCSDL_DELAY_100US);
#endif

	MELFAS_DISABLE_BASEBAND_ISR();                  /*                                      */
	MELFAS_DISABLE_WATCHDOG_TIMER_RESET();          /*                                 */

	for (i = 0; i <= IdxNum; i++) {
		if (pBinary[0] != NULL && nBinary_length[0] > 0 && nBinary_length[0] < MELFAS_FIRMWARE_MAX_SIZE)
		/*                        
                 
                            */
			nRet = mcsdl_download((const UINT8 *)pBinary[0], (const UINT16)nBinary_length[0], i);
		else
			nRet = MCSDL_RET_WRONG_BINARY;
	}

	MELFAS_ROLLBACK_BASEBAND_ISR();                 /*                                        */
	MELFAS_ROLLBACK_WATCHDOG_TIMER_RESET();         /*                                   */

#if MELFAS_ENABLE_DBG_PRINT
	mcsdl_print_result(nRet);
#endif

	return (nRet == MCSDL_RET_SUCCESS);

}
#endif

/*                                                                  
  
                         
  
                                                                    */

int mcsdl_download(const UINT8 *pBianry, const UINT16 unLength, INT8 IdxNum)
{
	int nRet;

	/*                                 
                     
                                    */
	if (unLength > MELFAS_FIRMWARE_MAX_SIZE) {
		nRet = MCSDL_RET_PROGRAM_SIZE_IS_WRONG;
		goto MCSDL_DOWNLOAD_FINISH;
	}


#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
	printk(" - Starting download...\n");
#endif

	/*                                 
                 
                                    */
#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
	printk(" > Ready\n");
#endif
	mcsdl_set_ready();

	/*                                 
               
                                    */
#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
	printk(" > Erase\n");
#endif
	nRet = mcsdl_erase_flash(IdxNum);
	if (nRet != MCSDL_RET_SUCCESS)
		goto MCSDL_DOWNLOAD_FINISH;

	/*                                 
                 
                                    */
#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
	printk("\n > Program   ");
#endif

	nRet = mcsdl_program_flash((UINT8 *)pBianry, (UINT16)unLength, IdxNum);
	if (nRet != MCSDL_RET_SUCCESS)
		goto MCSDL_DOWNLOAD_FINISH;

	/*                                 
                
                                    */
#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
	printk("\n > Verify    ");
#endif
	nRet = mcsdl_verify_flash((UINT8 *)pBianry, (UINT16)unLength, IdxNum);
	if (nRet != MCSDL_RET_SUCCESS)
		goto MCSDL_DOWNLOAD_FINISH;

	nRet = MCSDL_RET_SUCCESS;

MCSDL_DOWNLOAD_FINISH:

#if MELFAS_ENABLE_DBG_PRINT
	mcsdl_print_result(nRet);       /*           */
#endif
#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
	printk(" > Rebooting\n");
	printk(" - Fin.\n\n");
#endif
	mcsdl_reboot_mcs();

	return nRet;
}

/*                                                                  
  
                     
  
                                                                    */
static int mcsdl_erase_flash(INT8 IdxNum)
{
	int         i;
	UINT8 readBuffer[32];
	int eraseCompareValue = 0xFF;
	/*                                        
                 
                                           */
	if (IdxNum > 0) {
		mcsdl_select_isp_mode(ISP_MODE_NEXT_CHIP_BYPASS);
		mcsdl_delay(MCSDL_DELAY_3US);
	}
	mcsdl_select_isp_mode(ISP_MODE_ERASE_FLASH);
	mcsdl_unselect_isp_mode();

	/*                                        
                            
                                           */

	mcsdl_read_flash_from(readBuffer, 0x0000, 16, IdxNum);
	mcsdl_read_flash_from(&readBuffer[16], 0x7FF0, 16, IdxNum);

	if (IdxNum > 0) {
		eraseCompareValue = 0x00;
	}
	for (i = 0; i < 32; i++) {
		if (readBuffer[i] != eraseCompareValue)
			return MCSDL_RET_ERASE_FLASH_VERIFY_FAILED;
	}

	return MCSDL_RET_SUCCESS;
}

static int mcsdl_program_flash(UINT8 *pDataOriginal, UINT16 unLength, INT8 IdxNum)
{
	int             i;

	UINT8   *pData;
	UINT8   ucLength;

	UINT16  addr;
	UINT32  header;

	addr   = 0;
	pData  = pDataOriginal;
	ucLength = MELFAS_TRANSFER_LENGTH;

	while ((addr*4) < (int)unLength) {
		if ((unLength - (addr*4)) < MELFAS_TRANSFER_LENGTH)
			ucLength  = (UINT8)(unLength - (addr * 4));

		/*                                      
                         
                                          */

		mcsdl_delay(MCSDL_DELAY_40US);

		if (IdxNum > 0) {
			mcsdl_select_isp_mode(ISP_MODE_NEXT_CHIP_BYPASS);
			mcsdl_delay(MCSDL_DELAY_3US);
		}
		mcsdl_select_isp_mode(ISP_MODE_SERIAL_WRITE);

		/*                                             
                
                             
                                                 */
		header = ((addr & 0x1FFF) << 1) | 0x0 ;
		header = header << 14;
		mcsdl_write_bits(header, 18); /*            */

		/*                                 
                 
                                     */
		/*                         */
		addr += 1;

#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
		/*            */
#endif
		mcsdl_program_flash_part(pData);
		pData  += ucLength;

		/*                                             
              
                                                 */
		MCSDL_GPIO_SDA_SET_HIGH();
		mcsdl_delay(MCSDL_DELAY_40US);

		for (i = 0; i < 6; i++) {
			if (i == 2)
				mcsdl_delay(MCSDL_DELAY_20US);
			else if (i == 3)
				mcsdl_delay(MCSDL_DELAY_40US);

			MCSDL_GPIO_SCL_SET_HIGH();
			mcsdl_delay(MCSDL_DELAY_10US);
			MCSDL_GPIO_SCL_SET_LOW();
			mcsdl_delay(MCSDL_DELAY_10US);
		}
		MCSDL_GPIO_SDA_SET_LOW();

		mcsdl_unselect_isp_mode();
		mcsdl_delay(MCSDL_DELAY_300US);
	}

	return MCSDL_RET_SUCCESS;
}

static void mcsdl_program_flash_part(UINT8 *pData)
{
	UINT32  data;
	/*                                 
             
                                    */
	data  = (UINT32)pData[0] <<  0;
	data |= (UINT32)pData[1] <<  8;
	data |= (UINT32)pData[2] << 16;
	data |= (UINT32)pData[3] << 24;
	mcsdl_write_bits(data, 32);
}

static int mcsdl_verify_flash(UINT8 *pDataOriginal, UINT16 unLength, INT8 IdxNum)
{
	int       j;
	int       nRet;

	UINT8 *pData;
	UINT8 ucLength;

	UINT16 addr;
	UINT32 wordData;

	addr  = 0;
	pData = (UINT8 *) pDataOriginal;
	ucLength  = MELFAS_TRANSFER_LENGTH;

	while ((addr*4) < (int)unLength) {
		if ((unLength - (addr*4)) < MELFAS_TRANSFER_LENGTH)
			ucLength = (UINT8)(unLength - (addr * 4));

		/*                 */
		mcsdl_delay(MCSDL_DELAY_40US);

		/*                                      
                         
                                          */
		if (IdxNum > 0) {
			mcsdl_select_isp_mode(ISP_MODE_NEXT_CHIP_BYPASS);
			mcsdl_delay(MCSDL_DELAY_3US);
		}

		mcsdl_select_isp_mode(ISP_MODE_SERIAL_READ);

		/*                                             
                
                             
                                                 */
		wordData   = ((addr & 0x1FFF) << 1) | 0x0;
		wordData <<= 14;
		mcsdl_write_bits(wordData, 18);
		addr += 1;

#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
		/*              */
#endif
		/*                    
               
                        */
		mcsdl_read_flash(ucVerifyBuffer);

		MCSDL_GPIO_SDA_SET_LOW();
		MCSDL_GPIO_SDA_SET_OUTPUT(0);

		/*                    
              
                        */
		if (IdxNum == 0) {
			for (j = 0; j < (int)ucLength; j++) {
#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
				/*                                     */
				udelay(2);
#endif
				if (ucVerifyBuffer[j] != pData[j]) {
#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
					printk("\n [Error] Address : 0x%04X : 0x%02X - 0x%02X\n", addr, pData[j], ucVerifyBuffer[j]);
#endif
					nRet = MCSDL_RET_PROGRAM_VERIFY_FAILED;
					goto MCSDL_VERIFY_FLASH_FINISH;
				}
			}
		} else /*       */ {
			for (j = 0; j < (int)ucLength; j++) {
#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
				printk(" %02X", ucVerifyBuffer[j]);
#endif
				if ((0xff - ucVerifyBuffer[j]) != pData[j]) {
#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
					printk("\n [Error] Address : 0x%04X : 0x%02X - 0x%02X\n", addr, pData[j], ucVerifyBuffer[j]);
#endif
					nRet = MCSDL_RET_PROGRAM_VERIFY_FAILED;
					goto MCSDL_VERIFY_FLASH_FINISH;
				}
			}
		}
		pData += ucLength;
		mcsdl_unselect_isp_mode();
	}

	nRet = MCSDL_RET_SUCCESS;

MCSDL_VERIFY_FLASH_FINISH:

#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
	printk("\n");
#endif

	mcsdl_unselect_isp_mode();
	return nRet;
}


static void mcsdl_read_flash(UINT8 *pBuffer)
{
	int i;

	MCSDL_GPIO_SCL_SET_LOW();
	MCSDL_GPIO_SDA_SET_LOW();
	mcsdl_delay(MCSDL_DELAY_40US);

	for (i = 0; i < 6; i++) {
		MCSDL_GPIO_SCL_SET_HIGH();
		mcsdl_delay(MCSDL_DELAY_10US);
		MCSDL_GPIO_SCL_SET_LOW();
		mcsdl_delay(MCSDL_DELAY_10US);
	}
	mcsdl_read_32bits(pBuffer);
}

static int mcsdl_read_flash_from(UINT8 *pBuffer, UINT16 unStart_addr, UINT16 unLength, INT8 IdxNum)
{
	int i;
	int j;

	UINT8  ucLength;

	UINT16 addr;
	UINT32 wordData;

	if (unLength >= MELFAS_FIRMWARE_MAX_SIZE)
		return MCSDL_RET_PROGRAM_SIZE_IS_WRONG;

	addr  = 0;
	ucLength  = MELFAS_TRANSFER_LENGTH;

#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
	printk(" %04X : ", unStart_addr);
#endif
	for (i = 0; i < (int)unLength; i += (int)ucLength) {
		addr = (UINT16)i;
		if (IdxNum > 0) {
			mcsdl_select_isp_mode(ISP_MODE_NEXT_CHIP_BYPASS);
			mcsdl_delay(MCSDL_DELAY_3US);
		}

		mcsdl_select_isp_mode(ISP_MODE_SERIAL_READ);
		wordData   = (((unStart_addr + addr)&0x1FFF) << 1) | 0x0;
		wordData <<= 14;

		mcsdl_write_bits(wordData, 18);

		if ((unLength - addr) < MELFAS_TRANSFER_LENGTH)
			ucLength = (UINT8)(unLength - addr);

		/*                    
               
                        */
		mcsdl_read_flash(&pBuffer[addr]);

#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
		for (j = 0; j < (int)ucLength; j++)
			printk("%02X ", pBuffer[j]);
#endif
		mcsdl_unselect_isp_mode();
	}

#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
	/*             */
#endif
	return MCSDL_RET_SUCCESS;
}


static void mcsdl_set_ready(void)
{
	/*                                            
                     
                                               */

	MCSDL_VDD_SET_LOW(); /*     */

	MCSDL_CE_SET_LOW();
	MCSDL_CE_SET_OUTPUT(0);

	MCSDL_SET_GPIO_I2C();

	MCSDL_GPIO_SDA_SET_LOW();
	MCSDL_GPIO_SDA_SET_OUTPUT(0);

	MCSDL_GPIO_SCL_SET_LOW();
	MCSDL_GPIO_SCL_SET_OUTPUT(0);

	MCSDL_RESETB_SET_LOW();
	MCSDL_RESETB_SET_OUTPUT(0);

	mcsdl_delay(MCSDL_DELAY_25MS);                                          /*                      */

	MCSDL_VDD_SET_HIGH();
	/*                    */

	MCSDL_GPIO_SCL_SET_LOW();
	MCSDL_GPIO_SDA_SET_HIGH();
	mcsdl_delay(MCSDL_DELAY_40MS);                                          /*                  */
}


static void mcsdl_reboot_mcs(void)
{
	MCSDL_VDD_SET_LOW();

	MCSDL_CE_SET_LOW();
	MCSDL_CE_SET_OUTPUT(0);

	MCSDL_GPIO_SDA_SET_HIGH();
	MCSDL_GPIO_SDA_SET_OUTPUT(1);

	MCSDL_GPIO_SCL_SET_HIGH();
	MCSDL_GPIO_SCL_SET_OUTPUT(1);

	MCSDL_RESETB_SET_LOW();
	MCSDL_RESETB_SET_OUTPUT(0);

	mcsdl_delay(MCSDL_DELAY_25MS);                                          /*                      */

	MCSDL_VDD_SET_HIGH();
	/*                    */

	MCSDL_RESETB_SET_HIGH();
	MCSDL_RESETB_SET_INPUT();
	MCSDL_GPIO_SCL_SET_INPUT();
	MCSDL_GPIO_SDA_SET_INPUT();

	mcsdl_delay(MCSDL_DELAY_30MS);                                          /*                 */
}


/*                                            
  
                                   
  
                                              */
static void mcsdl_select_isp_mode(UINT8 ucMode)
{
	int    i;

	UINT8 enteringCodeMassErase[16]   = { 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1 };
	UINT8 enteringCodeSerialWrite[16] = { 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1 };
	UINT8 enteringCodeSerialRead[16]  = { 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1 };
	UINT8 enteringCodeNextChipBypass[16]  = { 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1 };

	UINT8 *pCode = 0;

	/*                                    
                              
                                       */
	if (ucMode == ISP_MODE_ERASE_FLASH)
		pCode = enteringCodeMassErase;
	else if (ucMode == ISP_MODE_SERIAL_WRITE)
		pCode = enteringCodeSerialWrite;
	else if (ucMode == ISP_MODE_SERIAL_READ)
		pCode = enteringCodeSerialRead;
	else if (ucMode == ISP_MODE_NEXT_CHIP_BYPASS)
		pCode = enteringCodeNextChipBypass;

	MCSDL_RESETB_SET_LOW();
	MCSDL_GPIO_SCL_SET_LOW();
	MCSDL_GPIO_SDA_SET_HIGH();

	for (i = 0; i < 16; i++) {
		if (pCode[i] == 1)
			MCSDL_RESETB_SET_HIGH();
		else
			MCSDL_RESETB_SET_LOW();

		mcsdl_delay(MCSDL_DELAY_3US);
		MCSDL_GPIO_SCL_SET_HIGH();
		mcsdl_delay(MCSDL_DELAY_3US);
		MCSDL_GPIO_SCL_SET_LOW();
		mcsdl_delay(MCSDL_DELAY_3US);
	}

	MCSDL_RESETB_SET_LOW();

	/*                                                   
                                                  
                                                      */
	mcsdl_delay(MCSDL_DELAY_7US);

	MCSDL_GPIO_SCL_SET_LOW();
	MCSDL_GPIO_SDA_SET_HIGH();
	if (ucMode == ISP_MODE_ERASE_FLASH) {
		mcsdl_delay(MCSDL_DELAY_7US);
		for (i = 0; i < 4; i++) {
			if (i == 2)
				mcsdl_delay(MCSDL_DELAY_25MS);
			else if (i == 3)
				mcsdl_delay(MCSDL_DELAY_150US);

			MCSDL_GPIO_SCL_SET_HIGH();
			mcsdl_delay(MCSDL_DELAY_3US);
			MCSDL_GPIO_SCL_SET_LOW();
			mcsdl_delay(MCSDL_DELAY_7US);
		}
	}
	MCSDL_GPIO_SDA_SET_LOW();
}


static void mcsdl_unselect_isp_mode(void)
{
	int i;

	MCSDL_RESETB_SET_LOW();
	mcsdl_delay(MCSDL_DELAY_3US);

	for (i = 0; i < 10; i++) {
		MCSDL_GPIO_SCL_SET_HIGH();
		mcsdl_delay(MCSDL_DELAY_3US);
		MCSDL_GPIO_SCL_SET_LOW();
		mcsdl_delay(MCSDL_DELAY_3US);
	}
}

static void mcsdl_read_32bits(UINT8 *pData)
{
	int i, j;

	MCSDL_GPIO_SCL_SET_LOW();
	MCSDL_GPIO_SDA_SET_LOW();
	MCSDL_GPIO_SDA_SET_INPUT();

	for (i = 3; i >= 0; i--) {
		pData[i] = 0;
		for (j = 0; j < 8; j++) {
			pData[i] <<= 1;
			MCSDL_GPIO_SCL_SET_LOW();
			mcsdl_delay(MCSDL_DELAY_1US);
			MCSDL_GPIO_SCL_SET_HIGH();
			mcsdl_delay(MCSDL_DELAY_1US);
			if (MCSDL_GPIO_SDA_IS_HIGH())
				pData[i] |= 0x01;
		}
	}

	MCSDL_GPIO_SDA_SET_LOW();
	MCSDL_GPIO_SDA_SET_OUTPUT(0);
}

static void mcsdl_write_bits(UINT32 wordData, int nBits)
{
	int i;

	MCSDL_GPIO_SCL_SET_LOW();
	MCSDL_GPIO_SDA_SET_LOW();

	for (i = 0; i < nBits; i++) {
		if (wordData & 0x80000000)
			MCSDL_GPIO_SDA_SET_HIGH();
		else
			MCSDL_GPIO_SDA_SET_LOW();

		mcsdl_delay(MCSDL_DELAY_3US);
		MCSDL_GPIO_SCL_SET_HIGH();
		mcsdl_delay(MCSDL_DELAY_3US);
		MCSDL_GPIO_SCL_SET_LOW();
		mcsdl_delay(MCSDL_DELAY_3US);

		wordData <<= 1;
	}
}

/*
                                        
 
                           
                              

                                                           
                                                           

                                                           
                                                           
 
*/

/*                                                            
  
                    
  
                                                              */
int mcsdl_download_sys(const UINT8 *pBianry, const UINT16 unLength, INT8 IdxNum)
{
	int nRet;

	/*                                 
                     
                                    */
	if (unLength > MELFAS_FIRMWARE_MAX_SIZE) {
		nRet = MCSDL_RET_PROGRAM_SIZE_IS_WRONG;
		goto MCSDL_DOWNLOAD_FINISH;
	}


#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
	printk(" - Starting download...\n");
#endif

	/*                                 
                 
                                    */
#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
	printk(" > Ready\n");
#endif
	mcsdl_set_ready();

	/*                                 
               
                                    */
#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
	printk(" > Erase\n");
#endif
	nRet = mcsdl_erase_flash(IdxNum);
	if (nRet != MCSDL_RET_SUCCESS)
		goto MCSDL_DOWNLOAD_FINISH;


	/*                                 
                 
                                    */
#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
	printk("\n > Program Test  ");
#endif

	nRet = mcsdl_program_flash_sys((UINT8 *)pBianry, (UINT16)unLength, IdxNum);
	if (nRet != MCSDL_RET_SUCCESS)
		goto MCSDL_DOWNLOAD_FINISH;

	/*                                 
                
                                    */
#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
	printk("\n > Verify    ");
#endif
	nRet = mcsdl_verify_flash_sys((UINT8 *)pBianry, (UINT16)unLength, IdxNum);
	if (nRet != MCSDL_RET_SUCCESS)
		goto MCSDL_DOWNLOAD_FINISH;

	nRet = MCSDL_RET_SUCCESS;

MCSDL_DOWNLOAD_FINISH:

#if MELFAS_ENABLE_DBG_PRINT
	mcsdl_print_result(nRet);       /*           */
#endif
#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
	printk(" > Rebooting\n");
	printk(" - Fin.\n\n");
#endif
	mcsdl_reboot_mcs();

	return nRet;
}

static int mcsdl_program_flash_sys(UINT8 *pDataOriginal, UINT16 unLength, INT8 IdxNum)
{
	int             i;

	UINT8   *pData;
	UINT8   ucLength;

	UINT16  addr;
	UINT32  header;

	addr   = 0;
	pData  = pDataOriginal;
	ucLength = MELFAS_TRANSFER_LENGTH;

	while ((addr*4) < (int)unLength) {
		if ((unLength - (addr*4)) < MELFAS_TRANSFER_LENGTH)
			ucLength  = (UINT8)(unLength - (addr * 4));

		/*                                      
                         
                                          */

		mcsdl_delay(MCSDL_DELAY_10US); /*      */

		if (IdxNum > 0) {
			mcsdl_select_isp_mode_sys(ISP_MODE_NEXT_CHIP_BYPASS);
			mcsdl_delay(MCSDL_DELAY_3US);
		}
		mcsdl_select_isp_mode_sys(ISP_MODE_SERIAL_WRITE);

		/*                                             
                
                             
                                                 */
		header = ((addr & 0x1FFF) << 1) | 0x0 ;
		header = header << 14;
		mcsdl_write_bits(header, 18); /*            */

		/*                                 
                 
                                     */
		/*                         */
		addr += 1;

#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
		/*            */
#endif

		mcsdl_program_flash_part(pData);
		pData  += ucLength;

		/*                                             
              
                                                 */

		MCSDL_RESETB_SET_OUTPUT(1);

		MCSDL_GPIO_SDA_SET_HIGH();
		mcsdl_delay(MCSDL_DELAY_10US); /*     */
		MCSDL_GPIO_SCL_SET_LOW();
		mcsdl_delay(MCSDL_DELAY_5US);

		for (i = 0; i < 6; i++) {
			if (i == 2)
				mcsdl_delay(MCSDL_DELAY_10US);
			else if (i == 3)
				mcsdl_delay(MCSDL_DELAY_20US);

			MCSDL_GPIO_SCL_SET_HIGH();
			mcsdl_delay(MCSDL_DELAY_5US); /*     */
			MCSDL_GPIO_SCL_SET_LOW();
			mcsdl_delay(MCSDL_DELAY_5US); /*     */

		}
		MCSDL_GPIO_SDA_SET_LOW();
		MCSDL_RESETB_SET_LOW();

		mcsdl_unselect_isp_mode_sys();
		/*                               */
	}

	return MCSDL_RET_SUCCESS;
}

static int mcsdl_verify_flash_sys(UINT8 *pDataOriginal, UINT16 unLength, INT8 IdxNum)
{
	int       j;
	int       nRet;

	UINT8 *pData;
	UINT8 ucLength;

	UINT16 addr;
	UINT32 wordData;

	addr  = 0;
	pData = (UINT8 *) pDataOriginal;
	ucLength  = MELFAS_TRANSFER_LENGTH;

	while ((addr*4) < (int)unLength) {
		if ((unLength - (addr*4)) < MELFAS_TRANSFER_LENGTH)
			ucLength = (UINT8)(unLength - (addr * 4));

		/*                 */
		mcsdl_delay(MCSDL_DELAY_20US); /*      */

		/*                                      
                         
                                          */
		if (IdxNum > 0) {
			mcsdl_select_isp_mode_sys(ISP_MODE_NEXT_CHIP_BYPASS);
			mcsdl_delay(MCSDL_DELAY_3US);
		}

		mcsdl_select_isp_mode_sys(ISP_MODE_SERIAL_READ);

		/*                                             
                
                             
                                                 */
		wordData   = ((addr & 0x1FFF) << 1) | 0x0;
		wordData <<= 14;
		mcsdl_write_bits(wordData, 18);
		addr += 1;

#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
		/*              */
#endif
		/*                    
               
                        */
		mcsdl_read_flash_sys(ucVerifyBuffer);

		MCSDL_GPIO_SDA_SET_LOW();
		MCSDL_GPIO_SDA_SET_OUTPUT(0);

		/*                    
              
                        */
		if (IdxNum == 0) {
			for (j = 0; j < (int)ucLength; j++) {
#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
				udelay(2);
#endif
				if (ucVerifyBuffer[j] != pData[j]) {
#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
					printk("\n [Error] Address : 0x%04X : 0x%02X - 0x%02X\n", addr, pData[j], ucVerifyBuffer[j]);
#endif
					nRet = MCSDL_RET_PROGRAM_VERIFY_FAILED;
					goto MCSDL_VERIFY_FLASH_FINISH;
				}
			}
		}
		pData += ucLength;
		mcsdl_unselect_isp_mode_sys();
	}

	nRet = MCSDL_RET_SUCCESS;

MCSDL_VERIFY_FLASH_FINISH:

#if MELFAS_ENABLE_DBG_PROGRESS_PRINT
	printk("\n");
#endif

	mcsdl_unselect_isp_mode();
	return nRet;
}

static void mcsdl_select_isp_mode_sys(UINT8 ucMode)
{
	int    i;

	UINT8 enteringCodeMassErase[16]   = { 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1 };
	UINT8 enteringCodeSerialWrite[16] = { 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1 };
	UINT8 enteringCodeSerialRead[16]  = { 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1 };
	UINT8 enteringCodeNextChipBypass[16]  = { 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1 };

	UINT8 *pCode = 0;

	/*                                    
                              
                                       */
	if (ucMode == ISP_MODE_ERASE_FLASH)
		pCode = enteringCodeMassErase;
	else if (ucMode == ISP_MODE_SERIAL_WRITE)
		pCode = enteringCodeSerialWrite;
	else if (ucMode == ISP_MODE_SERIAL_READ)
		pCode = enteringCodeSerialRead;
	else if (ucMode == ISP_MODE_NEXT_CHIP_BYPASS)
		pCode = enteringCodeNextChipBypass;

	/*                       */
	MCSDL_RESETB_SET_OUTPUT(0);
	MCSDL_GPIO_SCL_SET_LOW();
	MCSDL_GPIO_SDA_SET_HIGH();

	for (i = 0; i < 16; i++) {
		if (pCode[i] == 1)
			MCSDL_RESETB_SET_HIGH();
		else
			MCSDL_RESETB_SET_LOW();

		mcsdl_delay(MCSDL_DELAY_1US); /*    */
		MCSDL_GPIO_SCL_SET_HIGH();
		mcsdl_delay(MCSDL_DELAY_1US); /*    */
		MCSDL_GPIO_SCL_SET_LOW();
		mcsdl_delay(MCSDL_DELAY_1US); /*    */
	}

	MCSDL_RESETB_SET_LOW();

	/*                                                   
                                                  
                                                      */
	mcsdl_delay(MCSDL_DELAY_7US);

	MCSDL_GPIO_SCL_SET_LOW();
	MCSDL_GPIO_SDA_SET_HIGH();
	if (ucMode == ISP_MODE_ERASE_FLASH) {
		mcsdl_delay(MCSDL_DELAY_7US);
		for (i = 0; i < 4; i++) {
			if (i == 2)
				mcsdl_delay(MCSDL_DELAY_25MS);
			else if (i == 3)
				mcsdl_delay(MCSDL_DELAY_150US);

			MCSDL_GPIO_SCL_SET_HIGH();
			mcsdl_delay(MCSDL_DELAY_3US);
			MCSDL_GPIO_SCL_SET_LOW();
			mcsdl_delay(MCSDL_DELAY_7US);
		}
	}
	MCSDL_GPIO_SDA_SET_LOW();
}

static void mcsdl_unselect_isp_mode_sys(void)
{
	int i;

	MCSDL_RESETB_SET_OUTPUT(0);
	mcsdl_delay(MCSDL_DELAY_3US);

	for (i = 0; i < 10; i++) {
		MCSDL_GPIO_SCL_SET_HIGH();
		mcsdl_delay(MCSDL_DELAY_1US); /*    */
		MCSDL_GPIO_SCL_SET_LOW();
		mcsdl_delay(MCSDL_DELAY_1US); /*    */
	}
}

static void mcsdl_read_flash_sys(UINT8 *pBuffer)
{
	int i;

	MCSDL_GPIO_SCL_SET_LOW();
	MCSDL_GPIO_SDA_SET_LOW();
	mcsdl_delay(MCSDL_DELAY_40US);

	MCSDL_RESETB_SET_OUTPUT(1);
	for (i = 0; i < 6; i++) {
		MCSDL_GPIO_SCL_SET_HIGH();
		mcsdl_delay(MCSDL_DELAY_5US);
		MCSDL_GPIO_SCL_SET_LOW();
		mcsdl_delay(MCSDL_DELAY_5US);
	}
	MCSDL_RESETB_SET_LOW();
	mcsdl_read_32bits(pBuffer);
}
/*                                                            
  
                      
  
                                                              */
void mcsdl_delay(UINT32 nCount)
{
	if (nCount > 10000)
		mdelay(nCount / 1000);
	else
		udelay(nCount);
}

/*                                                            
  
                                  
  
                                                              */

#ifdef MELFAS_ENABLE_DBG_PRINT
static void mcsdl_print_result(int nRet)
{
	if (nRet == MCSDL_RET_SUCCESS) {
		printk(" > MELFAS Firmware downloading SUCCESS.\n");
	} else {
		printk(" > MELFAS Firmware downloading FAILED  :  ");
		switch (nRet) {
		case MCSDL_RET_SUCCESS:
			printk("MCSDL_RET_SUCCESS\n");
			break;
		case MCSDL_RET_ERASE_FLASH_VERIFY_FAILED:
			printk("MCSDL_RET_ERASE_FLASH_VERIFY_FAILED\n");
			break;
		case MCSDL_RET_PROGRAM_VERIFY_FAILED:
			printk("MCSDL_RET_PROGRAM_VERIFY_FAILED\n");
			break;
		case MCSDL_RET_PROGRAM_SIZE_IS_WRONG:
			printk("MCSDL_RET_PROGRAM_SIZE_IS_WRONG\n");
			break;
		case MCSDL_RET_VERIFY_SIZE_IS_WRONG:
			printk("MCSDL_RET_VERIFY_SIZE_IS_WRONG\n");
			break;
		case MCSDL_RET_WRONG_BINARY:
			printk("MCSDL_RET_WRONG_BINARY\n");
			break;
		case MCSDL_RET_READING_HEXFILE_FAILED:
			printk("MCSDL_RET_READING_HEXFILE_FAILED\n");
			break;
		case MCSDL_RET_FILE_ACCESS_FAILED:
			printk("MCSDL_RET_FILE_ACCESS_FAILED\n");
			break;
		case MCSDL_RET_MELLOC_FAILED:
			printk("MCSDL_RET_MELLOC_FAILED\n");
			break;
		case MCSDL_RET_WRONG_MODULE_REVISION:
			printk("MCSDL_RET_WRONG_MODULE_REVISION\n");
			break;
		default:
			printk("UNKNOWN ERROR. [0x%02X].\n", nRet);
		break;
		}
		printk("\n");
	}
}

#endif


#if MELFAS_ENABLE_DELAY_TEST

/*                                                            
  
                                                     
  
                                                                             
  
                                                              */

void mcsdl_delay_test(INT32 nCount)
{
	INT16 i;
	MELFAS_DISABLE_BASEBAND_ISR();                          /*                                    */
	MELFAS_DISABLE_WATCHDOG_TIMER_RESET();                  /*                                */

	/*                                
                                 
                                   */
	/*                     */
	MCSDL_GPIO_SCL_SET_OUTPUT(0);
	MCSDL_GPIO_SDA_SET_OUTPUT(0);
	MCSDL_RESETB_SET_OUTPUT(0);

	MCSDL_GPIO_SCL_SET_HIGH();

	for (i = 0; i < nCount; i++) {
#if 1
		MCSDL_GPIO_SCL_SET_LOW();
		mcsdl_delay(MCSDL_DELAY_20US);
		MCSDL_GPIO_SCL_SET_HIGH();
		mcsdl_delay(MCSDL_DELAY_100US);
#elif 0
		MCSDL_GPIO_SCL_SET_LOW();
		mcsdl_delay(MCSDL_DELAY_500US);
		MCSDL_GPIO_SCL_SET_HIGH();
		mcsdl_delay(MCSDL_DELAY_1MS);
#else
		MCSDL_GPIO_SCL_SET_LOW();
		mcsdl_delay(MCSDL_DELAY_25MS);
		TKEY_INTR_SET_LOW();
		mcsdl_delay(MCSDL_DELAY_45MS);
		TKEY_INTR_SET_HIGH();
#endif
	}

	MCSDL_GPIO_SCL_SET_HIGH();

	MELFAS_ROLLBACK_BASEBAND_ISR();                         /*                                        */
	MELFAS_ROLLBACK_WATCHDOG_TIMER_RESET();                 /*                                   */
}
#endif



