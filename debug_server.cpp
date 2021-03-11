#include "debug_server.h"
#include "msg/message_bus.h"
#include "msg/message_pool.h"
#include "debug_printf.h"
#include <string.h>

// auto-generated header files for messages to output debug info
#include "msg_aliases.h"

#define BUF_COUNT    12
#if defined(ENABLE_DEBUG_MSG_STRINGS)
#define BUF_PAYLOAD_SIZE (PrintfMessage::MSG_SIZE)
const uint8_t DICTIONARY_ID[16] = {0};
#elif defined(ENABLE_DEBUG_MSGS)
#define BUF_PAYLOAD_SIZE (PrintfIDMessage::MSG_SIZE)
const uint8_t DICTIONARY_ID[] = {FORMAT_STR_DICTIONARY_ID};
#endif

#define ESCAPED_FILE_PATH ___ucplatform_debug_server

extern int global_debug_threshold;

static void SetStreamDebugThreshold(int streamID, int threshold)
{
    if(streamID == 0)
    {
        global_debug_threshold = threshold;
    }
    else
    {
        MessageClient* c = MessageClient::GetClient(streamID);
        if(c)
        {
            c->SetDebugThreshold(threshold);
        }
        else
        {
            debugWarn("Invalid streamID %d\n", streamID);
        }
    }
}

static MessagePool* DebugPool()
{
    static MessagePoolWithStorage<BUF_PAYLOAD_SIZE, BUF_COUNT> debug_pool;
    return &debug_pool;
}

DebugServer::DebugServer(const char* name)
: MessageClient("DebugServer", DebugPool(), 1000),
  m_deviceName(name)
{
    MessageBus::Subscribe(this, GetDeviceInfoMessage::MSG_ID);
    MessageBus::Subscribe(this, SetDebugThresholdMessage::MSG_ID);
}

void DebugServer::Initialize()
{
}

void DebugServer::PeriodicTask()
{
    //# output statistics, and dictionary ID?
    //# or only on request?  or maybe once, in Initialize() above?
}

void DebugServer::HandleReceivedMessage(Message& msg)
{
    switch(msg.GetMessageID())
    {
        case GetDeviceInfoMessage::MSG_ID:
        {
            DeviceInfoMessage msg;
            msg.CopyInName((const uint8_t*)m_deviceName, strlen(m_deviceName));
            msg.CopyInDebugStringDictionaryID(DICTIONARY_ID, sizeof(DICTIONARY_ID));
            SendMessage(msg);
            break;
        }
        case SetDebugThresholdMessage::MSG_ID:
        {
            SetDebugThresholdMessage* m = (SetDebugThresholdMessage*)&msg;
            SetStreamDebugThreshold(m->GetStreamID(), (int)m->GetDebugThreshold());
            break;
        }
    }
}
MessageBuffer* DebugServer::GetBuffer(int size)
{
    return DebugPool()->Allocate(size);
}
