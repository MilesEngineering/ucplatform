/**
    copied from ./samv71/asf/sam/drivers/mcan/module_config/conf_mcan.h
*/
#ifndef CONF_MCAN_H_INCLUDED
#define CONF_MCAN_H_INCLUDED

/*
 * Below is the message RAM setting, it will be stored in the system RAM.
 * Please adjust the message size according to your application.
 */
/** Range: 1..64 */ 
#define CONF_MCAN0_RX_FIFO_0_NUM         16     
/** Range: 1..64 */        
#define CONF_MCAN0_RX_FIFO_1_NUM         0      
/** Range: 1..64 */      
#define CONF_MCAN0_RX_BUFFER_NUM         0   // zero for no dedicated rx buffers (fifo only)
/** Range: 1..16 */
#define CONF_MCAN0_TX_BUFFER_NUM         0   // zero for no dedicated transmit buffers (fifo/queue only)
/** Range: 1..16 */        
#define CONF_MCAN0_TX_FIFO_QUEUE_NUM     8   // transmit fifo/queue length
/** Range: 1..32 */       
#define CONF_MCAN0_TX_EVENT_FIFO         0
/** Range: 1..128 */
#define CONF_MCAN0_RX_STANDARD_ID_FILTER_NUM     0    
/** Range: 1..64 */
#define CONF_MCAN0_RX_EXTENDED_ID_FILTER_NUM     4    
/** Range: 1..64 */
#define CONF_MCAN1_RX_FIFO_0_NUM         16             
/** Range: 1..64 */
#define CONF_MCAN1_RX_FIFO_1_NUM         0  
/** Range: 1..64 */          
#define CONF_MCAN1_RX_BUFFER_NUM         0      
/** Range: 1..16 */     
#define CONF_MCAN1_TX_BUFFER_NUM         0 
/** Range: 1..16 */     
#define CONF_MCAN1_TX_FIFO_QUEUE_NUM     8     
/** Range: 1..32 */        
#define CONF_MCAN1_TX_EVENT_FIFO         0
/** Range: 1..128 */
#define CONF_MCAN1_RX_STANDARD_ID_FILTER_NUM     0
/** Range: 1..64 */
#define CONF_MCAN1_RX_EXTENDED_ID_FILTER_NUM     4    

/** The value should be 8/12/16/20/24/32/48/64. */
#define CONF_MCAN_ELEMENT_DATA_SIZE         64

/**
 * The setting of the nominal bit rate is based on the PCK5 which is 30M which you can
 * change in the conf_clock.h. Below is the default configuration. The
 * time quanta is 30MHz / (2+1) =  10MHz. And each bit is (1 + NTSEG1 + 1 + NTSEG2 + 1) = 20 time
 * quanta which means the bit rate is 10MHz/20=500KHz.
 */
/** Nominal bit Baud Rate Prescaler */
#define CONF_MCAN_NBTP_NBRP_VALUE    2
/** Nominal bit (Re)Synchronization Jump Width */
#define CONF_MCAN_NBTP_NSJW_VALUE    3
/** Nominal bit Time segment before sample point */
#define CONF_MCAN_NBTP_NTSEG1_VALUE  10
/** Nominal bit Time segment after sample point */
#define CONF_MCAN_NBTP_NTSEG2_VALUE  7

/*
 * The setting of the data bit rate is based on the GCLK_MCAN is 48M which you can
 * change in the conf_clock.h. Below is the default configuration. The
 * time quanta is 48MHz / (5+1) =  8MHz. And each bit is (1 + FTSEG1 + 1 + FTSEG2 + 1) = 16 time
 * quanta which means the bit rate is 8MHz/16=500KHz.
 */
/** Data bit Baud Rate Prescaler */
#define CONF_MCAN_FBTP_FBRP_VALUE    5
/** Data bit (Re)Synchronization Jump Width */
#define CONF_MCAN_FBTP_FSJW_VALUE    3
/** Data bit Time segment before sample point */
#define CONF_MCAN_FBTP_FTSEG1_VALUE  10
/** Data bit Time segment after sample point */
#define CONF_MCAN_FBTP_FTSEG2_VALUE  3

#endif
