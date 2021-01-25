#include "message_pool.h"

#include "queue.h"
#include "message_client.h"
#include "stdio.h"

MessagePool* MessagePool::s_interruptContextPool = 0;

MessagePool::MessagePool(uint8_t* buffer, int count, int size)
{
    m_freeList = xQueueCreate(count, sizeof(void*));
    configASSERT( m_freeList );
    
    for(int i=0; i<count; i++)
    {
        MessageBuffer* msg = (MessageBuffer*)&buffer[i*size];
#ifdef MSG_REFERENCE_COUNTING
        // set non-atomically before we put onto free list.
        // no one else can have a reference before that.
        std::atomic_store(&msg->m_referenceCount, 1);
#endif
        msg->m_owner = this;
        msg->m_bufferSize = size - offsetof(MessageBuffer, m_data);
        Free(msg);
    }
}
void MessagePool::SetInterruptContextPool(MessagePool& pool)
{
    s_interruptContextPool = &pool;
}

MessagePool* MessagePool::CurrentPool()
{
    if(xPortIsInsideInterrupt())
    {
        return s_interruptContextPool;
    }
    MessageClient* currentClient = MessageClient::CurrentClient();
    if(currentClient)
    {
        return currentClient->GetMessagePool();
    }
    printf("%s line %d, ERROR!  No current client defined!\n", __FILE__, __LINE__);
    return nullptr;
}
MessageBuffer* MessagePool::Allocate(int size)
{
    UNUSED(size);
    MessageBuffer* msg=0;
    //int old_len = uxQueueMessagesWaiting(m_freeList);
    if(xPortIsInsideInterrupt())
    {
        xQueueReceiveFromISR(m_freeList, (void*)&msg, 0); //# do i need &xHigherPriorityTaskWoken?
    }
    else
    {
        xQueueReceive(m_freeList, (void*)&msg, 0);
    }
#ifdef MSG_REFERENCE_COUNTING
    if(msg)
    {
        // set non-atomically after we get it from the free list and
        // before we return.
        // no one else can have a reference before that.
        std::atomic_store(&msg->m_referenceCount, 1);
    }
#endif
    if(msg && size > msg->m_bufferSize)
    {
        printf("ERROR!  Can't allocate %d byte buffer, max size is %d\n", size, msg->m_bufferSize);
        Free(msg);
        return 0;
    }
    return msg;
}
void MessagePool::Free(MessageBuffer* msg)
{
#ifdef MSG_REFERENCE_COUNTING
    if (msg->decrement_refcount() == 1)
#endif
    {
        int old_len;
        BaseType_t ret;
        if(xPortIsInsideInterrupt())
        {
            old_len = uxQueueMessagesWaitingFromISR(m_freeList);
            ret = xQueueSendFromISR(m_freeList, (void*)&msg, 0); //# do i need &xHigherPriorityTaskWoken?
        }
        else
        {
            old_len = uxQueueMessagesWaiting(m_freeList);
            ret = xQueueSend(m_freeList, (void*)&msg, 0);
        }
        if(!ret)
        {
            int new_len;
            if(xPortIsInsideInterrupt())
            {
                new_len = uxQueueMessagesWaitingFromISR(m_freeList);
            }
            else
            {
                new_len = uxQueueMessagesWaiting(m_freeList);
            }
            printf("MessagePool::Free(%p) FAIL with %d->%d len queue\n", (void*)msg, old_len, new_len);
            if(!xPortIsInsideInterrupt())
            {
                while(uxQueueMessagesWaiting(m_freeList) > 0)
                {
                    MessageBuffer* buf = Allocate(1);
                    printf("Had %p in queue", (void*)buf);
                }
            }
            configASSERT(false);
        }
    }
}
