#include "debug_server.h"
#include "msg/message_bus.h"
#include "msg/message_pool.h"
#include "debug_printf.h"

// auto-generated header files for messages to output debug info
#include "Printf.h"
#include "PrintfID.h"

#define POOL_SIZE    12
#if defined(OUTPUT_DEBUG_STRINGS) || defined(USE_PRINTF_DIRECTLY)
#define POOL_BUF_LEN (PrintfMessage::MSG_SIZE*offsetof(MessageBuffer, m_data))
#else
#define POOL_BUF_LEN (PrintfIDMessage::MSG_SIZE*offsetof(MessageBuffer, m_data))
const uint8_t DICTIONARY_ID[] = {FORMAT_STR_DICTIONARY_ID};
#endif

static MessagePool* DebugPool()
{
    static MessagePoolWithStorage<POOL_BUF_LEN, POOL_SIZE> debug_pool;
    return &debug_pool;
}

DebugServer::DebugServer()
: MessageClient("DebugServer", DebugPool(), 1000)
{
    //#MessageBus::Subscribe(this, DebugControl::MSG_ID);
}

void DebugServer::Initialize()
{
}

void DebugServer::PeriodicTask()
{
    //# output statistics, and dictionary ID?
    //# or only on request?
}

void DebugServer::HandleReceivedMessage(Message& msg)
{
    switch(msg.GetMessageID())
    {
        //#case DebugControl::MSG_ID:
            break;
    }
}
MessageBuffer* DebugServer::GetBuffer(int size)
{
    return DebugPool()->Allocate(size);
}
