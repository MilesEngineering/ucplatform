#include "can_client.h"
#include <headers/CANHeader.h>

#include <inttypes.h>
#include <string.h>
#include <sam/drivers/mcan/module_config/conf_mcan.h>
#include "services/ioport/ioport.h"

//# This #define allows two modes of operation:
//# 1) HANDLE_RX_IN_ISR: process Rx FIFO received packets, allocate Messages, send them, all from ISR.
//#    This saves about 1ms vs. processing them in the main loop, but results in a much longer
//#    execution time of the ISR.
//# 2) undefined HANDLE_RX_IN_ISR: from ISR Wake the PeriodicTask(), which will poll
//#    the Rx FIFO, and allocate Messages, and send them.  Currently it interrupts on each
//#    new message, and does a Wake() each time.  We could explore using a watermark on the
//#    the Rx FIFO to generate fewer interrupts (but still Wake() before danger over overrunning
//#    the FIFO), or even polling without using interrupts at all.  The fewer the interrupts, the
//#    higher the absolute performance, at the expense of latency.
//#define HANDLE_RX_IN_ISR

// DLC length coding
static const uint8_t DLC_LENGTH_CODING[] = 
{
    0, 1, 2, 3, 4, 5, 6, 7, 8,
    /*9=*/12,
    /*10=*/16,
    /*11=*/20,
    /*12=*/24,
    /*13=*/32,
    /*14=*/48,
    /*15=*/64
};

static const uint8_t DLC_FROM_LENGTH[65] =
{
    0,1,2,3,4,5,6,7,8,
    9,9,9,9, // 9-12
    10,10,10,10, // 13-16
    11,11,11,11, // 17-20
    12,12,12,12, // 21-24
    13,13,13,13,13,13,13,13, // 25-32
    14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14, // 33-48
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15 // 49-64
};

bool CANPacketFromMsg(mcan_tx_element& packet, const Message& msg)
{
    //# should we handle fragmentation/reassembly of packets > 64 bytes?
    if(msg.GetDataLength() > 64)
    {
        return false;
    }
    
    // initialize the packet
    mcan_get_tx_buffer_element_defaults(&packet);
    
    // Data Length Code
    packet.T1.bit.DLC = DLC_FROM_LENGTH[msg.GetDataLength()];

    CANHeader hdr;
    hdr.SetPriority(msg.GetPriority());
    hdr.SetID(msg.GetMessageID());
    hdr.SetSource(msg.GetSource());
    hdr.SetDestination(msg.GetDestination());
    hdr.SetLengthDecrement(DLC_LENGTH_CODING[packet.T1.bit.DLC] - msg.GetDataLength());

    packet.T0.bit.ID = hdr.GetCanID();
    packet.T0.bit.RTR = 0; // remote transmission request, otherwise normal dataframe
    packet.T0.bit.XTD = 1; // Extended (29-bit) identifier
    packet.T0.bit.ESI = 0; // Error State Indicator
    
    packet.T1.bit.BRS = 1; // Bit Rate Switch
    packet.T1.bit.FDF = 1; // CAN-FD format
    //# If this was enabled, tx would pause until we read corresponding events out.
    //# also need to modify conf_mcan.h to set CONF_MCANx_TX_EVENT_FIFO count to > 0.
    //# also need to enable interrupt if we want to be interrupted, or poll for events.
    packet.T1.bit.EFCC = 0; //# Event FIFO control.
    packet.T1.bit.MM = 0; //# message marker, gets put into tx event FIFO for user to use to match up with what was transmitted

    memcpy(packet.data, msg.GetDataPointer(), msg.GetDataLength());
    
    return true;
}
bool MsgFromCANPacket(Message& msg, const mcan_rx_element_fifo_0& packet)
{
    if(!packet.R0.bit.XTD) // Extended (29-bit) identifier
    {
        return false;
    }

    CANHeader hdr;
    hdr.SetCanID(packet.R0.bit.ID);
    int len = DLC_LENGTH_CODING[packet.R1.bit.DLC] - hdr.GetLengthDecrement();
    msg.Allocate(len);
    msg.SetPriority(hdr.GetPriority());
    msg.SetMessageID(hdr.GetID());
    msg.SetSource(hdr.GetSource());
    msg.SetDestination(hdr.GetDestination());
    msg.SetTime(packet.R1.bit.RXTS); // Receive Timestamp

    // ignored
    packet.R0.bit.RTR; // remote transmission request
    packet.R0.bit.ESI; // Error State Indicator
    packet.R1.bit.BRS; // Bit Rate Switch
    packet.R1.bit.EDL; // Extended Data Length, AKA CAN-FD
    packet.R1.bit.FIDX; // filter index
    packet.R1.bit.ANMF; // Accepted Non-matching Frame

    return true;
}

CanClient::CanClient(const char* name, Mcan* mcan_module, MessagePool& pool)
: MessageClient(name, &pool, 5000/*ms*/), //# has to be fast for polling CAN Rx, until we get 'wake' working
  m_mcan_module(mcan_module),
  m_last_rx_filter_index_used(0)
{
}

CanClient* CanClient::Can0(MessagePool* pool=0)
{
    static CanClient instance("canfd0", MCAN0, *pool);
    return &instance;
}

CanClient* CanClient::Can1(MessagePool* pool=0)
{
    static CanClient instance("canfd1", MCAN1, *pool);
    return &instance;
}

/**
 * \brief MCAN module initialization.
 *
 */
void CanClient::ConfigureHardware()
{
    /* Initialize the module. */
    struct mcan_config config_mcan;
    mcan_get_config_defaults(&config_mcan);
    config_mcan.rx_fifo_0_overwrite = false;
    mcan_init(&mcan_instance, m_mcan_module, &config_mcan);

    mcan_enable_fd_mode(&mcan_instance);

    //# loopback mode
    mcan_enable_test_mode(&mcan_instance);

    mcan_start(&mcan_instance);

    /* Enable interrupts for this MCAN module */
    if(mcan_instance.hw == MCAN0)
    {
        irq_register_handler(MCAN0_INT0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+1);
    }
    else
    {
        irq_register_handler(MCAN1_INT0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+1);
    }
    mcan_enable_interrupt
    (&mcan_instance,
     (mcan_interrupt_source) (MCAN_FORMAT_ERROR | MCAN_ACKNOWLEDGE_ERROR | MCAN_BUS_OFF | MCAN_RX_FIFO_0_LOST_MESSAGE));
}

void CanClient::Initialize()
{
    // from MsgToolsExample/embedded/samv71/asf/sam/boards/same70_xplained/same70_xplained.h
    if(mcan_instance.hw == MCAN0)
    {
        //# These are defined in amv71/asf/sam/boards/same70_xplained/same70_xplained.h or
        //# samv71/asf/sam/boards/samv71_xplained_ultra/samv71_xplained_ultra.h,
        //# and will need to be defined appropriately for any new hardware.

        /* Configure the CAN0 TX and RX pins. */
        ioport_set_pin_peripheral_mode(PIN_CAN0_RX_IDX, PIN_CAN0_RX_FLAGS);
        ioport_set_pin_peripheral_mode(PIN_CAN0_TX_IDX, PIN_CAN0_TX_FLAGS);
        /* Configure the transiver0 RS & EN pins. */
        //# these don't appear to be used anywhere...
        ioport_set_pin_dir(PIN_CAN0_TR_RS_IDX, IOPORT_DIR_OUTPUT);
        ioport_set_pin_dir(PIN_CAN0_TR_EN_IDX, IOPORT_DIR_OUTPUT);
    }
    else if(mcan_instance.hw == MCAN1)
    {
        /* Configure the CAN1 TX and RX pin. */
        ioport_set_pin_peripheral_mode(PIN_CAN1_RX_IDX, PIN_CAN1_RX_FLAGS);
        ioport_set_pin_peripheral_mode(PIN_CAN1_TX_IDX, PIN_CAN1_TX_FLAGS);
        //# Need to figure out where transceiver 1 RS and EN pins connect!
    }
    
    ConfigureHardware();
    
    //# This may need to change in the future by deleting this line, and having
    //# multiple specific calls to AddRxFilter done externally.
    // Add Rx filter to receive everything (bitmask id2=0).
    AddRxFilter(BITMASK_FILTER, 0, 0);
    
    MessageBus::SubscribeAll(this);
}

void CanClient::HandleReceivedMessage(Message& msg)
{
    if(AddToTransmitFIFO(msg))
    {
        m_tx_count++;
    }
    else
    {
        m_tx_overflow++;
        printf("CAN transmit FIFO append failed\n");
    }
}

#define MCAN_RX_FIFO_NUMBER 0
void CanClient::PeriodicTask()
{
#ifndef HANDLE_RX_IN_ISR
    int rx_count = 0;
    while(1)
    {
        uint32_t status = mcan_rx_get_fifo_status(&mcan_instance, MCAN_RX_FIFO_NUMBER);
        uint32_t num_elements = status & MCAN_RXF1S_F1FL_Msk;
        uint32_t get_index = (status & MCAN_RXF1S_F1GI_Msk) >> MCAN_RXF1S_F1GI_Pos;
        if (num_elements > 0)
        {
            //# calling mcan_get_rx_fifo_0_element() to copy to a local var and then
            //# translating from mcan_rx_element_fifo_0 to a Message isn't very efficient.
            //# It'd be quicker to make a version of mcan_get_rx_fifo_0_element
            //# that accepts a Message pointer to copy into as a parameter.
            mcan_get_rx_fifo_0_element(&mcan_instance, &m_rx_packet, get_index);
            mcan_rx_fifo_acknowledge(&mcan_instance, MCAN_RX_FIFO_NUMBER, get_index);

            ProcessRxPacket(m_rx_packet);
            rx_count++;
        }
        else
        {
            break;
        }
    }
    // reenable the interrupt, now that we've emptied the Rx FIFO
    mcan_enable_interrupt(&mcan_instance, MCAN_RX_FIFO_0_NEW_MESSAGE);
    if(rx_count > 0)
    {
        printf("CANrx=%d rx_overflow=%d @ %" PRId32 "\n", rx_count, m_rx_overflow, xTaskGetTickCount());
    }
#else
    printf("CAN rx_overflow=%d\n", m_rx_overflow);
#endif
}

bool CanClient::AddRxFilter(RxFilterType type, uint32_t filter_value_1, uint32_t filter_value_2=0)
{
    if((mcan_instance.hw == MCAN0 && m_last_rx_filter_index_used >= CONF_MCAN0_RX_EXTENDED_ID_FILTER_NUM) ||
       (mcan_instance.hw == MCAN1 && m_last_rx_filter_index_used >= CONF_MCAN1_RX_EXTENDED_ID_FILTER_NUM))
    {
        printf("error!  filter %d/%lu/%lu rejected\n", type, filter_value_1, filter_value_2);
        return false;
    }

    struct mcan_extended_message_filter_element et_filter;

    mcan_get_extended_message_filter_element_default(&et_filter);
    // always store to FIFO 0
    et_filter.F0.bit.EFEC = MCAN_EXTENDED_MESSAGE_FILTER_ELEMENT_F0_EFEC_STF0M_Val;
    et_filter.F1.bit.EFT = type;
    et_filter.F0.bit.EFID1 = filter_value_1;
    et_filter.F1.bit.EFID2 = filter_value_2;

    //#printf("setting filter #%d %d 0x%lX 0x%lX\n", m_last_rx_filter_index_used, type, filter_value_1, filter_value_2);
    mcan_set_rx_extended_filter(&mcan_instance, &et_filter, m_last_rx_filter_index_used++);
    mcan_enable_interrupt(&mcan_instance, MCAN_RX_FIFO_0_NEW_MESSAGE);
    return true;
}

bool CanClient::ProcessRxPacket(mcan_rx_element_fifo_0& packet)
{
    uint8_t data_len = DLC_LENGTH_CODING[packet.R1.bit.DLC];

    //# As an optimization, consider checking for subscribers>0 based on just
    //# the ID, without copying the whole packet and allocating a Message.
    //# That way if no one wants it, we can throw it away quicker.
    Message msg;
    MsgFromCANPacket(msg, packet);
    if(msg.Exists())
    {
        SendMessage(msg);
        m_rx_count++;
        return true;
    }
    m_rx_overflow++;
    return false;
}

bool CanClient::AddToTransmitFIFO(const Message& msg)
{
    uint32_t tfqf = mcan_tx_get_fifo_queue_status(&mcan_instance);
    // If tx FIFO is full, return error status
    if (tfqf & MCAN_TXFQS_TFQF)
    {
        return false;
    }

    // create the CAN packet based on the message
    struct mcan_tx_element tx_packet;
    if(!CANPacketFromMsg(tx_packet, msg))
    {
        m_msg_too_big_for_can++;
        printf("CANPacketFromMsg of msg len %d failed\n", msg.GetDataLength());
        return false;
    }
    
    // Get next put buffer index
    int can_tx_buffer_index = (tfqf & MCAN_TXFQS_TFQPI_Msk) >> MCAN_TXFQS_TFQPI_Pos;
    //# while debugging, try using dedicated tx buffer instead of fifo
    //#int can_tx_buffer_index = 0;

    // put the packet in the tx queue
    //# translating from Message to mcan_tx_element and then calling mcan_set_tx_buffer_element to copy
    //# it isn't very efficient.  It'd be quicker to make a version of mcan_set_tx_buffer_element
    //# that accepts a Message as an input parameter.
    enum status_code sc1 = mcan_set_tx_buffer_element(&mcan_instance, &tx_packet, can_tx_buffer_index);
    enum status_code sc2 = mcan_tx_transfer_request(&mcan_instance, 1 << can_tx_buffer_index);
    if(sc1 != STATUS_OK || sc2 != STATUS_OK)
    {
        printf("tried to tx, got %d %d\n", sc1, sc2);
    }
    printf("    CANtx[%d] @ %ld\n", can_tx_buffer_index, xTaskGetTickCount());

    return true;
}

extern "C" void MCAN0_INT0_Handler(void)
{
    //#printf("MCAN0_INT0\n");
    CanClient::Can0()->HandleInterrupt();
}
extern "C" void MCAN1_INT0_Handler(void)
{
    //#printf("MCAN1_INT0\n");
    CanClient::Can1()->HandleInterrupt();
}

void CanClient::HandleInterrupt()
{
    volatile uint32_t status;
    status = mcan_read_interrupt_status(&mcan_instance);
    // if no interrupts for us occurred, return quickly without setting s_isrCurrentClient
    if(!status)
    {
        return;
    }

    s_isrCurrentClient = this;
    

    if (status & MCAN_RX_FIFO_0_NEW_MESSAGE) {
#ifdef HANDLE_RX_IN_ISR
        mcan_clear_interrupt_status(&mcan_instance, MCAN_RX_FIFO_0_NEW_MESSAGE);
        mcan_get_rx_fifo_0_element(&mcan_instance, &m_rx_packet, m_rx_fifo_receive_index);
        mcan_rx_fifo_acknowledge(&mcan_instance, 0, m_rx_fifo_receive_index);
        m_rx_fifo_receive_index++;
        if (m_rx_fifo_receive_index == CONF_MCAN0_RX_FIFO_0_NUM) {
            m_rx_fifo_receive_index = 0;
        }
        
        if(ProcessRxPacket(m_rx_packet))
        {
            printf("CAN rx @ %" PRId32 "\n", xTaskGetTickCountFromISR());
        }
#else
        // disable the interrupt.  The mainloop will reenable it, after it processes all packets in Rx FIFO.
        mcan_clear_interrupt_status(&mcan_instance, MCAN_RX_FIFO_0_NEW_MESSAGE);
        mcan_disable_interrupt(&mcan_instance, MCAN_RX_FIFO_0_NEW_MESSAGE);
        // wake the PeriodicTask() to poll the Rx FIFO.
        Wake();
#endif
    }

    if ((status & MCAN_ACKNOWLEDGE_ERROR) || (status & MCAN_FORMAT_ERROR)) {
        printf("Protocol error, please double check the clock in two boards. \r\n\r\n");
        mcan_clear_interrupt_status
        (&mcan_instance,
         (mcan_interrupt_source)(MCAN_ACKNOWLEDGE_ERROR | MCAN_FORMAT_ERROR));
    }

    if (status & MCAN_BUS_OFF) {
        printf(": MCAN bus off error, re-initialization. \r\n\r\n");
        mcan_clear_interrupt_status(&mcan_instance, MCAN_BUS_OFF);
        mcan_stop(&mcan_instance);
        //#qs_mcan_fd.c had ConfigureHardware(); here, but https://borkedlabs.com/blog/2017/09-24-samc21-same70-samv71-canbus-bosch-lessons-learned/
        //# notes that mcan_start() is sufficient.
        mcan_start(&mcan_instance);
    }
    if (status & MCAN_RX_FIFO_0_LOST_MESSAGE) {
        mcan_clear_interrupt_status(&mcan_instance, MCAN_RX_FIFO_0_LOST_MESSAGE);
        m_rx_overflow++;
    }
    //#printf("end ISR\n");
    
    s_isrCurrentClient = nullptr;
}
