#ifndef MESSAGE_POOL_H
#define MESSAGE_POOL_H

#include <cstddef>
#include "message.h"
#include "FreeRTOS.h"
#include "queue.h"

// a pool of messages that can be allocated and freed.
class MessagePool
{
    protected:
        // constructor is protected, so only sub-classes can use it.
        MessagePool(uint8_t* buffer, int buffer_size, int buf_count);
        static constexpr std::size_t BufferOverhead() { return offsetof(MessageBuffer, m_data); }
    public:
        static MessagePool* CurrentPool();
        static void SetInterruptContextPool(MessagePool& pool);
        MessageBuffer* Allocate(int size);
        void Free(MessageBuffer* msg);
        static void FreeToOwner(MessageBuffer* msg);
    private:
        QueueHandle_t m_freeList;
        static MessagePool* s_interruptContextPool;
};

// a class to get MessagePool with correctly sized buffers,
// with padding as necessary to ensure proper alignment.
template <int BUF_PAYLOAD_SIZE, int BUF_COUNT>
class MessagePoolWithStorage : public MessagePool
{
    public:
        static constexpr size_t round_to_alignment(size_t in)
        {
            constexpr size_t multiple = alignof(std::max_align_t);
            // a few sanity checks for alignment > 0 and divisible by two
            static_assert(multiple>0, "alignof(std::max_align_t) must be greater than zero");
            static_assert((multiple & (multiple - 1)) == 0, "alignof(std::max_align_t) must be divisible by two");
            // quick way to round to multiple of a power of two without branching
            return ((in + multiple - 1) / multiple) * multiple;
        }
        static constexpr size_t MSG_BUFFER_SIZE = round_to_alignment(BUF_PAYLOAD_SIZE+MessagePool::BufferOverhead());
        MessagePoolWithStorage()
        : MessagePool((uint8_t*)m_buffer, MSG_BUFFER_SIZE, BUF_COUNT)
        {
        }
    private:
        uint8_t m_buffer[MSG_BUFFER_SIZE][BUF_COUNT];
};

#endif
