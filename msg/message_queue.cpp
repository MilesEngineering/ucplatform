#include "message_queue.h"

const void* MessageQueue::s_wakeSentinel = (void*)~0;

MessageQueue::MessageQueue(int count)
{
    m_msgQueue = xQueueCreate(count, sizeof(void*));
    configASSERT( m_msgQueue );
}
MessageBuffer* MessageQueue::get(TickType_t waitTime)
{
    MessageBuffer* msgbuf=0;
    BaseType_t ret;
    if(xPortIsInsideInterrupt())
    {
        ret = xQueueReceiveFromISR(m_msgQueue, (void*)&msgbuf, 0); //# do i need &xHigherPriorityTaskWoken?
    }
    else
    {
        ret = xQueueReceive(m_msgQueue, (void*)&msgbuf, waitTime);
    }
    if(ret && msgbuf)
    {
        //# can't decrement here, the caller needs to keep our reference!
        //msgbuf->decrement_refcount();
    }
    return msgbuf;
}
void MessageQueue::put(MessageBuffer* msgbuf)
{
    BaseType_t ret;
    msgbuf->increment_refcount();
    if(xPortIsInsideInterrupt())
    {
        ret = xQueueSendFromISR(m_msgQueue, (void*)&msgbuf, 0); //# do i need &xHigherPriorityTaskWoken?
    }
    else
    {
        ret = xQueueSend(m_msgQueue, (void*)&msgbuf, 0);
    }
    configASSERT(ret == pdTRUE);
}
void MessageQueue::put(Message& msg)
{
    put(msg.m_buf);
}

void MessageQueue::wake()
{
    if(xPortIsInsideInterrupt())
    {
        BaseType_t xHigherPriorityTaskWoken = 1;
        xQueueSendFromISR(m_msgQueue, &s_wakeSentinel, &xHigherPriorityTaskWoken);
    }
    else
    {
        xQueueSend(m_msgQueue, &s_wakeSentinel, 0);
    }
}
