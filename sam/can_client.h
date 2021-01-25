#ifndef CAN_CLIENT_H__
#define CAN_CLIENT_H__

#include "FreeRTOS.h"
#include "task.h"
#include "message_client.h"
#include "message_bus.h"
#include "message.h"
#include "message_pool.h"
#include "message_queue.h"
#include "headers/CANHeader.h"

#include <sam/drivers/mcan/mcan.h>

//# Consider adding an option to poll for Rx messages from PeriodicTask, at perhaps 1ms.
//# It would add a bit of latency but improve performance considerably with high CAN traffic.
class CanClient : public MessageClient
{
    public:
        CanClient(const char* name, Mcan* mcan_module, MessagePool& pool);
        void HandleReceivedMessage(Message& msg) override;
        void PeriodicTask() override;
        virtual void Initialize() override;
        static CanClient* Can0(MessagePool* pool);
        static CanClient* Can1(MessagePool* pool);
        void HandleInterrupt();

        // Currently we only support filtering by range or bitmask,
        // of extended (29-bit) IDs, with messages going to FIFO zero.
        // This is intentional, to simplify the API for users.
        enum RxFilterType
        { RANGE_FILTER = 3,
          BITMASK_FILTER = 2};
        // for type=BITMASK, id2 has bits 1=care and 0=dontcare
        bool AddRxFilter(RxFilterType type, uint32_t id1, uint32_t id2);

    private: // internal functions
        void ConfigureHardware();
        bool ProcessRxPacket(mcan_rx_element_fifo_0& packet);
        bool AddToTransmitFIFO(const Message& msg);

    private:
        Mcan* m_mcan_module;
        struct mcan_module mcan_instance;

        // CAN peripheral receive FIFO index.
        volatile uint32_t m_rx_fifo_receive_index = 0;

        // Used by the Rx ISR, declared here to reduce stack usage.
        struct mcan_rx_element_fifo_0 m_rx_packet;
        
        // for adding multiple filters (removing filters is not supported!)
        int m_last_rx_filter_index_used;
    
        // statistics
        int m_tx_count;
        int m_rx_count;
        int m_tx_overflow;
        int m_rx_overflow;
        int m_msg_too_big_for_can;
};

#endif
