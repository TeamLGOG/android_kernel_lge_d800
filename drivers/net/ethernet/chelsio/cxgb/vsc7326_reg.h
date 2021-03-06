/*                                                                           */
#ifndef _VSC7321_REG_H_
#define _VSC7321_REG_H_

/*                                                        
  
                                                      
                                   
 */

/*                                                  */

#define CRA(blk,sub,adr) ((((blk) & 0x7) << 13) | (((sub) & 0xf) << 9) | (((adr) & 0xff) << 1))

/*                                 */
#define REG_CHIP_ID		CRA(0x7,0xf,0x00)	/*         */
#define REG_BLADE_ID		CRA(0x7,0xf,0x01)	/*          */
#define REG_SW_RESET		CRA(0x7,0xf,0x02)	/*                   */
#define REG_MEM_BIST		CRA(0x7,0xf,0x04)	/*     */
#define REG_IFACE_MODE		CRA(0x7,0xf,0x07)	/*                */
#define REG_MSCH		CRA(0x7,0x2,0x06)	/*                 */
#define REG_CRC_CNT		CRA(0x7,0x2,0x0a)	/*                 */
#define REG_CRC_CFG		CRA(0x7,0x2,0x0b)	/*            */
#define REG_SI_TRANSFER_SEL	CRA(0x7,0xf,0x18)	/*                    */
#define REG_PLL_CLK_SPEED	CRA(0x7,0xf,0x19)	/*                       */
#define REG_SYS_CLK_SELECT	CRA(0x7,0xf,0x1c)	/*                     */
#define REG_GPIO_CTRL		CRA(0x7,0xf,0x1d)	/*              */
#define REG_GPIO_OUT		CRA(0x7,0xf,0x1e)	/*          */
#define REG_GPIO_IN		CRA(0x7,0xf,0x1f)	/*         */
#define REG_CPU_TRANSFER_SEL	CRA(0x7,0xf,0x20)	/*                     */
#define REG_LOCAL_DATA		CRA(0x7,0xf,0xfe)	/*                         */
#define REG_LOCAL_STATUS	CRA(0x7,0xf,0xff)	/*                           */

/*                      */
#define REG_AGGR_SETUP		CRA(0x7,0x1,0x00)	/*                  */
#define REG_PMAP_TABLE		CRA(0x7,0x1,0x01)	/*                */
#define REG_MPLS_BIT0		CRA(0x7,0x1,0x08)	/*                    */
#define REG_MPLS_BIT1		CRA(0x7,0x1,0x09)	/*                    */
#define REG_MPLS_BIT2		CRA(0x7,0x1,0x0a)	/*                    */
#define REG_MPLS_BIT3		CRA(0x7,0x1,0x0b)	/*                    */
#define REG_MPLS_BITMASK	CRA(0x7,0x1,0x0c)	/*               */
#define REG_PRE_BIT0POS		CRA(0x7,0x1,0x10)	/*                        */
#define REG_PRE_BIT1POS		CRA(0x7,0x1,0x11)	/*                        */
#define REG_PRE_BIT2POS		CRA(0x7,0x1,0x12)	/*                        */
#define REG_PRE_BIT3POS		CRA(0x7,0x1,0x13)	/*                        */
#define REG_PRE_ERR_CNT		CRA(0x7,0x1,0x14)	/*                             */

/*                */
/*                                          */	/*                           */
/*                                             */	/*                             */
#define REG_RAM_BIST_CMD	CRA(0x7,0x1,0x00)	/*                           */
#define REG_RAM_BIST_RESULT	CRA(0x7,0x1,0x01)	/*                             */
#define   BIST_PORT_SELECT	0x00			/*                  */
#define   BIST_COMMAND		0x01			/*                     */
#define   BIST_STATUS		0x02			/*                       */
#define   BIST_ERR_CNT_LSB	0x03			/*                        */
#define   BIST_ERR_CNT_MSB	0x04			/*                        */
#define   BIST_ERR_SEL_LSB	0x05			/*                         */
#define   BIST_ERR_SEL_MSB	0x06			/*                         */
#define   BIST_ERROR_STATE	0x07			/*                            */
#define   BIST_ERR_ADR0		0x08			/*                          */
#define   BIST_ERR_ADR1		0x09			/*                             */
#define   BIST_ERR_ADR2		0x0a			/*                             */
#define   BIST_ERR_ADR3		0x0b			/*                          */

/*               
                                     
                          
 */
#define REG_TEST(ie,fn)		CRA(0x2,ie&1,0x00+fn)	/*                      */
#define REG_TOP_BOTTOM(ie,fn)	CRA(0x2,ie&1,0x10+fn)	/*                          */
#define REG_TAIL(ie,fn)		CRA(0x2,ie&1,0x20+fn)	/*                    */
#define REG_HEAD(ie,fn)		CRA(0x2,ie&1,0x30+fn)	/*                   */
#define REG_HIGH_LOW_WM(ie,fn)	CRA(0x2,ie&1,0x40+fn)	/*                          */
#define REG_CT_THRHLD(ie,fn)	CRA(0x2,ie&1,0x50+fn)	/*                       */
#define REG_FIFO_DROP_CNT(ie,fn) CRA(0x2,ie&1,0x60+fn)	/*                          */
#define REG_DEBUG_BUF_CNT(ie,fn) CRA(0x2,ie&1,0x70+fn)	/*                          */
#define REG_BUCKI(fn) CRA(0x2,2,0x20+fn)	/*                          */
#define REG_BUCKE(fn) CRA(0x2,3,0x20+fn)	/*                          */

/*                       
                                     
                                              
 */
/*                                                                   */
#define REG_TRAFFIC_SHAPER_BUCKET(ie,bn) CRA(0x2,ie&1,0x0a + (bn>7) | ((bn&7)<<4))
#define REG_TRAFFIC_SHAPER_CONTROL(ie)	CRA(0x2,ie&1,0x3b)

#define REG_SRAM_ADR(ie)	CRA(0x2,ie&1,0x0e)	/*                   */
#define REG_SRAM_WR_STRB(ie)	CRA(0x2,ie&1,0x1e)	/*                        */
#define REG_SRAM_RD_STRB(ie)	CRA(0x2,ie&1,0x2e)	/*                       */
#define REG_SRAM_DATA_0(ie)	CRA(0x2,ie&1,0x3e)	/*                      */
#define REG_SRAM_DATA_1(ie)	CRA(0x2,ie&1,0x4e)	/*                         */
#define REG_SRAM_DATA_2(ie)	CRA(0x2,ie&1,0x5e)	/*                         */
#define REG_SRAM_DATA_3(ie)	CRA(0x2,ie&1,0x6e)	/*                      */
#define REG_SRAM_DATA_BLK_TYPE(ie) CRA(0x2,ie&1,0x7e)	/*               */
/*                                                                                    */
#define REG_CONTROL(ie)		CRA(0x2,ie&1,0x0f)	/*              */
#define REG_ING_CONTROL		CRA(0x2,0x0,0x0f)	/*                         */
#define REG_EGR_CONTROL		CRA(0x2,0x1,0x0f)	/*                        */
#define REG_AGE_TIMER(ie)	CRA(0x2,ie&1,0x1f)	/*             */
#define REG_AGE_INC(ie)		CRA(0x2,ie&1,0x2f)	/*                 */
#define DEBUG_OUT(ie)		CRA(0x2,ie&1,0x3f)	/*                              */
#define DEBUG_CNT(ie)		CRA(0x2,ie&1,0x4f)	/*                      */

/*                */
#define REG_SPI4_MISC		CRA(0x5,0x0,0x00)	/*               */
#define REG_SPI4_STATUS		CRA(0x5,0x0,0x01)	/*            */
#define REG_SPI4_ING_SETUP0	CRA(0x5,0x0,0x02)	/*                              */
#define REG_SPI4_ING_SETUP1	CRA(0x5,0x0,0x03)	/*                             */
#define REG_SPI4_ING_SETUP2	CRA(0x5,0x0,0x04)	/*                               */
#define REG_SPI4_EGR_SETUP0	CRA(0x5,0x0,0x05)	/*                             */
#define REG_SPI4_DBG_CNT(n)	CRA(0x5,0x0,0x10+n)	/*                    */
#define REG_SPI4_DBG_SETUP	CRA(0x5,0x0,0x1A)	/*                      */
#define REG_SPI4_TEST		CRA(0x5,0x0,0x20)	/*                     */
#define REG_TPGEN_UP0		CRA(0x5,0x0,0x21)	/*                                       */
#define REG_TPGEN_UP1		CRA(0x5,0x0,0x22)	/*                                       */
#define REG_TPCHK_UP0		CRA(0x5,0x0,0x23)	/*                                     */
#define REG_TPCHK_UP1		CRA(0x5,0x0,0x24)	/*                                     */
#define REG_TPSAM_P0		CRA(0x5,0x0,0x25)	/*                   */
#define REG_TPSAM_P1		CRA(0x5,0x0,0x26)	/*                   */
#define REG_TPERR_CNT		CRA(0x5,0x0,0x27)	/*                               */
#define REG_SPI4_STICKY		CRA(0x5,0x0,0x30)	/*                      */
#define REG_SPI4_DBG_INH	CRA(0x5,0x0,0x31)	/*                               */
#define REG_SPI4_DBG_STATUS	CRA(0x5,0x0,0x32)	/*                        */
#define REG_SPI4_DBG_GRANT	CRA(0x5,0x0,0x33)	/*                              */

#define REG_SPI4_DESKEW 	CRA(0x5,0x0,0x43)	/*                              */

/*                           */
/*                                                                     
                                                                        
                           
  
                                                                           
                                                                         
                                        
 */
/*                                              */
#define REG_MISC_10G		CRA(0x1,0xa,0x00)	/*                  */
#define REG_PAUSE_10G		CRA(0x1,0xa,0x01)	/*                */
#define REG_NORMALIZER_10G	CRA(0x1,0xa,0x05)	/*                */
#define REG_STICKY_RX		CRA(0x1,0xa,0x06)	/*                   */
#define REG_DENORM_10G		CRA(0x1,0xa,0x07)	/*               */
#define REG_STICKY_TX		CRA(0x1,0xa,0x08)	/*                */
#define REG_MAX_RXHIGH		CRA(0x1,0xa,0x0a)	/*                      */
#define REG_MAX_RXLOW		CRA(0x1,0xa,0x0b)	/*                      */
#define REG_MAC_TX_STICKY	CRA(0x1,0xa,0x0c)	/*                           */
#define REG_MAC_TX_RUNNING	CRA(0x1,0xa,0x0d)	/*                            */
#define REG_TX_ABORT_AGE	CRA(0x1,0xa,0x14)	/*                          */
#define REG_TX_ABORT_SHORT	CRA(0x1,0xa,0x15)	/*                           */
#define REG_TX_ABORT_TAXI	CRA(0x1,0xa,0x16)	/*                             */
#define REG_TX_ABORT_UNDERRUN	CRA(0x1,0xa,0x17)	/*                           */
#define REG_TX_DENORM_DISCARD	CRA(0x1,0xa,0x18)	/*                          */
#define REG_XAUI_STAT_A		CRA(0x1,0xa,0x20)	/*               */
#define REG_XAUI_STAT_B		CRA(0x1,0xa,0x21)	/*               */
#define REG_XAUI_STAT_C		CRA(0x1,0xa,0x22)	/*               */
#define REG_XAUI_CONF_A		CRA(0x1,0xa,0x23)	/*                      */
#define REG_XAUI_CONF_B		CRA(0x1,0xa,0x24)	/*                      */
#define REG_XAUI_CODE_GRP_CNT	CRA(0x1,0xa,0x25)	/*                             */
#define REG_XAUI_CONF_TEST_A	CRA(0x1,0xa,0x26)	/*                      */
#define REG_PDERRCNT		CRA(0x1,0xa,0x27)	/*                      */

/*                                                  */
/*                          */
#define REG_MAX_LEN(pn)		CRA(0x1,pn,0x02)	/*            */
#define REG_MAC_HIGH_ADDR(pn)	CRA(0x1,pn,0x03)	/*                           */
#define REG_MAC_LOW_ADDR(pn)	CRA(0x1,pn,0x04)	/*                           */

/*               
                        
 */
#define REG_MODE_CFG(pn)	CRA(0x1,pn,0x00)	/*                    */
#define REG_PAUSE_CFG(pn)	CRA(0x1,pn,0x01)	/*                     */
#define REG_NORMALIZER(pn)	CRA(0x1,pn,0x05)	/*            */
#define REG_TBI_STATUS(pn)	CRA(0x1,pn,0x06)	/*            */
#define REG_PCS_STATUS_DBG(pn)	CRA(0x1,pn,0x07)	/*                  */
#define REG_PCS_CTRL(pn)	CRA(0x1,pn,0x08)	/*             */
#define REG_TBI_CONFIG(pn)	CRA(0x1,pn,0x09)	/*                   */
#define REG_STICK_BIT(pn)	CRA(0x1,pn,0x0a)	/*             */
#define REG_DEV_SETUP(pn)	CRA(0x1,pn,0x0b)	/*                       */
#define REG_DROP_CNT(pn)	CRA(0x1,pn,0x0c)	/*              */
#define REG_PORT_POS(pn)	CRA(0x1,pn,0x0d)	/*                        */
#define REG_PORT_FAIL(pn)	CRA(0x1,pn,0x0e)	/*                        */
#define REG_SERDES_CONF(pn)	CRA(0x1,pn,0x0f)	/*                      */
#define REG_SERDES_TEST(pn)	CRA(0x1,pn,0x10)	/*             */
#define REG_SERDES_STAT(pn)	CRA(0x1,pn,0x11)	/*               */
#define REG_SERDES_COM_CNT(pn)	CRA(0x1,pn,0x12)	/*                      */
#define REG_DENORM(pn)		CRA(0x1,pn,0x15)	/*                       */
#define REG_DBG(pn)		CRA(0x1,pn,0x16)	/*                 */
#define REG_TX_IFG(pn)		CRA(0x1,pn,0x18)	/*               */
#define REG_HDX(pn)		CRA(0x1,pn,0x19)	/*                    */

/*            */
/*                 */
/*           */
/*                                  */

enum {
	RxInBytes		= 0x00,	//               
	RxSymbolCarrier		= 0x01,	//                        
	RxPause			= 0x02,	//                        
	RxUnsupOpcode		= 0x03,	//                                         
	RxOkBytes		= 0x04,	//                        
	RxBadBytes		= 0x05,	//                       
	RxUnicast		= 0x06,	//                      
	RxMulticast		= 0x07,	//                        
	RxBroadcast		= 0x08,	//                        
	Crc			= 0x09,	//                         
	RxAlignment		= 0x0a,	//                          
	RxUndersize		= 0x0b,	//                   
	RxFragments		= 0x0c,	//                              
	RxInRangeLengthError	= 0x0d,	//                           
	RxOutOfRangeError	= 0x0e,	//                                   
	RxOversize		= 0x0f,	//                  
	RxJabbers		= 0x10,	//                             
	RxSize64		= 0x11,	//                        
	RxSize65To127		= 0x12,	//                       
	RxSize128To255		= 0x13,	//                 
	RxSize256To511		= 0x14,	//                 
	RxSize512To1023		= 0x15,	//                  
	RxSize1024To1518	= 0x16,	//                   
	RxSize1519ToMax		= 0x17,	//                  

	TxOutBytes		= 0x18,	//            
	TxPause			= 0x19,	//                    
	TxOkBytes		= 0x1a, //               
	TxUnicast		= 0x1b,	//                 
	TxMulticast		= 0x1c,	//                   
	TxBroadcast		= 0x1d,	//                   
	TxMultipleColl		= 0x1e,	//                                      
	TxLateColl		= 0x1f,	//                           
	TxXcoll			= 0x20,	//                                    
	TxDefer			= 0x21,	//                                      
	TxXdefer		= 0x22,	//                              
	TxCsense		= 0x23,	//                                  
	TxSize64		= 0x24,	//                        
	TxSize65To127		= 0x25,	//                       
	TxSize128To255		= 0x26,	//                 
	TxSize256To511		= 0x27,	//                 
	TxSize512To1023		= 0x28,	//                  
	TxSize1024To1518	= 0x29,	//                   
	TxSize1519ToMax		= 0x2a,	//                  
	TxSingleColl		= 0x2b,	//                                   
	TxBackoff2		= 0x2c,	//                                           
	TxBackoff3		= 0x2d,	//                              
	TxBackoff4		= 0x2e,	//          
	TxBackoff5		= 0x2f,	//          
	TxBackoff6		= 0x30,	//          
	TxBackoff7		= 0x31,	//          
	TxBackoff8		= 0x32,	//          
	TxBackoff9		= 0x33,	//          
	TxBackoff10		= 0x34,	//           
	TxBackoff11		= 0x35,	//           
	TxBackoff12		= 0x36,	//           
	TxBackoff13		= 0x37,	//           
	TxBackoff14		= 0x38,	//           
	TxBackoff15		= 0x39,	//           
	TxUnderrun		= 0x3a,	//                               
	//                                       
	RxIpgShrink		= 0x3c,	//                          
	//                                         
	StatSticky1G		= 0x3e,	//                      
	StatInit		= 0x3f	//                     
};

#define REG_RX_XGMII_PROT_ERR	CRA(0x4,0xa,0x3b)		/*                                               */
#define REG_STAT_STICKY10G	CRA(0x4,0xa,StatSticky1G)	/*                   */

#define REG_RX_OK_BYTES(pn)	CRA(0x4,pn,RxOkBytes)
#define REG_RX_BAD_BYTES(pn)	CRA(0x4,pn,RxBadBytes)
#define REG_TX_OK_BYTES(pn)	CRA(0x4,pn,TxOkBytes)

/*                                */
/*                                                                           
                                                                            
                                                                           
 */

#define REG_MIIM_STATUS		CRA(0x3,0x0,0x00)	/*              */
#define REG_MIIM_CMD		CRA(0x3,0x0,0x01)	/*               */
#define REG_MIIM_DATA		CRA(0x3,0x0,0x02)	/*            */
#define REG_MIIM_PRESCALE	CRA(0x3,0x0,0x03)	/*                    */

#define REG_ING_FFILT_UM_EN	CRA(0x2, 0, 0xd)
#define REG_ING_FFILT_BE_EN	CRA(0x2, 0, 0x1d)
#define REG_ING_FFILT_VAL0	CRA(0x2, 0, 0x2d)
#define REG_ING_FFILT_VAL1	CRA(0x2, 0, 0x3d)
#define REG_ING_FFILT_MASK0	CRA(0x2, 0, 0x4d)
#define REG_ING_FFILT_MASK1	CRA(0x2, 0, 0x5d)
#define REG_ING_FFILT_MASK2	CRA(0x2, 0, 0x6d)
#define REG_ING_FFILT_ETYPE	CRA(0x2, 0, 0x7d)


/*       */

#endif
