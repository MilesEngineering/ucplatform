#include "simulink_message_client.h"

const char *RT_MEMORY_ALLOCATION_ERROR = "memory allocation error";

extern "C" bool allocate_msg(uint32_t id, int size, int src, int dst, void** msgbuf, uint8_t** data)
{
    //# could add more parameters here if needed, like source, destination?!?
    MessagePool* pool = MessagePool::CurrentPool();
    if(pool)
    {
        // allocate sets reference count to 1, and that gets decremented after send is called.
        MessageBuffer* cpp_msgbuf = pool->Allocate(size);
        if(cpp_msgbuf)
        {
            Message msg(cpp_msgbuf);
            msg.SetMessageID(id);
            msg.SetDataLength(size);
            msg.SetSource(src);
            msg.SetDestination(dst);
            *msgbuf = (void*)cpp_msgbuf;
            *data = msg.GetDataPointer();
            // Don't let the Message destructor free the message.
            msg.TakeBuffer();
            return true;
        }
    }
    *msgbuf = NULL;
    *data = NULL;
    return false;
}

extern "C" void send_msg(void** msgbuf)
{
    MessageBuffer* cpp_msgbuf = (MessageBuffer*)*msgbuf;
    Message msg(cpp_msgbuf);
    SimulinkMessageClient::Instance()->SendMessage(msg);
    // clear the user's pointer to make sure they can't use it any more.
    *msgbuf = NULL;
    //# The Message destructor will free the buffer when this function returns.
}

extern "C"  uint8_t* message_rx_data(uint32_t message_id, int src, int dst)
{
    MessageBuffer* msgbuf = SimulinkMessageClient::Instance()->RegisterRxBlock(message_id, src, dst);
    if(msgbuf)
    {
        Message msg(msgbuf);
        uint8_t* ret = msg.GetDataPointer();
        // Don't let the Message destructor free the message.
        msg.TakeBuffer();
        return ret;
    }
    return NULL;
}
