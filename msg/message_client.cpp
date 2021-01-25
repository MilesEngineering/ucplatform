#include "message_client.h"
#include "message_bus.h"
#include "tick.h"

extern "C" void client_callback( void* pvParameters );

MessageClient* MessageClient::s_firstClient = nullptr;
MessageClient* MessageClient::s_lastClient = nullptr;
MessageClient* MessageClient::s_isrCurrentClient = nullptr;

MessageClient::MessageClient(const char* name, MessagePool* pool, int period, int priority, int stacksize)
: m_period(period),
  m_msgPool(pool),
  m_rxMsgs(),
  m_nextClient(nullptr)
{
    BaseType_t xReturned;

    /* Create the task, storing the handle. */
    xReturned = xTaskCreate(
                    client_callback, /* Function that implements the task. */
                    name,            /* Text name for the task. */
                    stacksize,       /* Stack size in words, not bytes. */
                    (void*) this,    /* Parameter passed into the task. */
                    priority,        /* Priority at which the task is created. */
                    &m_taskHandle ); /* Used to pass out the created task's handle. */

    configASSERT( xReturned == pdPASS );
    
    if(MessageClient::s_firstClient == nullptr)
    {
        MessageClient::s_firstClient = this;
    }
    else
    {
        MessageClient::s_lastClient->m_nextClient = this;
    }
    MessageClient::s_lastClient = this;
}
void MessageClient::RunLoop()
{
    // Store a pointer to ourself in the FreeRTOS thread-local storage array,
    // so CurrentClient() can return a pointer to us while we're executing.
    vTaskSetThreadLocalStoragePointer( m_taskHandle,  /* Task handle. */
                                       0,             /* Index into the array. */
                                       (void*)this ); /* The value to store. */
    m_lastIdleTime = GetTickCount();
    while(1)
    {
        TickType_t now = GetTickCount();
        TickType_t wait_time = m_lastIdleTime + m_period - now;
        //printf("  waiting %ld (%ld + %d - %ld)\n", wait_time, m_lastIdleTime, m_period, now);
        MessageBuffer* msgbuf = m_rxMsgs.get(wait_time);
        if(msgbuf)
        {
            Message msg(msgbuf);
            HandleReceivedMessage(msg);
        }
        else
        {
            m_lastIdleTime = GetTickCount();
            PeriodicTask();
        }
    }
}
void MessageClient::SendMessage(Message& msg)
{
    MessageBus::SendMessage(msg, this);
}
MessageClient* MessageClient::CurrentClient()
{
    if(xPortIsInsideInterrupt())
    {
        return s_isrCurrentClient;
    }
    MessageClient* c = (MessageClient*)pvTaskGetThreadLocalStoragePointer( nullptr, 0 );
    return c;
}
MessagePool* MessageClient::GetMessagePool()
{
    return m_msgPool;
}
void MessageClient::DeliverMessage(Message& msg)
{
    m_rxMsgs.put(msg.m_buf);
}
void MessageClient::PeriodicTask()
{
}
void MessageClient::Initialize()
{
}
void MessageClient::Wake()
{
    // wake the queue.
    //# RunLoop() will get a nullptr, which is what happens when timeout for PeriodicTask occurs,
    //# which will wake the task without giving it a real message.
    m_rxMsgs.wake();
}
void MessageClient::InitializeAll()
{
    for(MessageClient* mc = MessageClient::s_firstClient; mc != nullptr;  mc = mc->m_nextClient)
    {
        mc->Initialize();
    }
}

extern "C" void client_callback( void* pvParameters )
{
    MessageClient* c= (MessageClient*)pvParameters;
    c->RunLoop();
}
