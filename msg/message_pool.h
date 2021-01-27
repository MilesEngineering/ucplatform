#ifndef MESSAGE_POOL_H
#define MESSAGE_POOL_H

#include "message.h"
#include "FreeRTOS.h"
#include "queue.h"

// a pool of messages that can be allocated and freed.
class MessagePool
{
    public:
        MessagePool(uint8_t* buffer, int buffer_size, int buf_count);
        static MessagePool* CurrentPool();
        static void SetInterruptContextPool(MessagePool& pool);
        MessageBuffer* Allocate(int size);
        void Free(MessageBuffer* msg);
    private:
        QueueHandle_t m_freeList;
        static MessagePool* s_interruptContextPool;
};

template <int buf_size, int buf_count>
class MessagePoolWithStorage : public MessagePool
{
    public:
        MessagePoolWithStorage()
        : MessagePool(m_buffer, buf_size, buf_count)
        {
        }
    private:
        uint8_t m_buffer[buf_size*buf_count];
};

#endif
