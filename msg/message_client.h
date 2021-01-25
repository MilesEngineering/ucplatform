#ifndef MESSAGE_CLIENT_H
#define MESSAGE_CLIENT_H

#include "message.h"
#include "message_queue.h"

#define DEFAULT_STACK_SIZE 256
// Higher numerical value is higher priority
// Idle task uses priority zero.
#define DEFAULT_PRIORITY     1

/*
    This class handles most of the commonality associated with any application.
    Descendants of this class need to:
        * implement HandleReceivedMessage(), to do what they wish with received msgs
        * call SendMessage() when they want to transmit
*/
class MessageClient
{
    public:
        MessageClient(const char* name, MessagePool* pool, int period=0, int priority=DEFAULT_PRIORITY, int stacksize=DEFAULT_STACK_SIZE);
        void RunLoop();
		void SendMessage(Message& msg);
        static MessageClient* CurrentClient();
        MessagePool* GetMessagePool();
        void DeliverMessage(Message& msg);
        static void InitializeAll();
        void Wake();
	protected:
        virtual void HandleReceivedMessage(Message& msg/*, MsgInfo* msgInfo*/) = 0;
		virtual void PeriodicTask();
        virtual void Initialize();
	private:
        TaskHandle_t m_taskHandle;
		// how often the periodic function should be invoked - zero for never
		int m_period;
        // last time we called our periodic function
		TickType_t m_lastIdleTime;
		// count of errors encountered when trying to transmit
		int m_txErrors;
		int m_allocErrors;
        MessagePool* m_msgPool;
        MessageQueue m_rxMsgs;
    
        // statistics related to messages.
        int m_msg_buffers_allocated;
        // count of messages this client freed.  Not necessarily deallocated, unless refcount gets to zero.
        int m_msgs_freed;
        int m_msgs_stale;
        
        // to manage initialization after construction
        static MessageClient* s_firstClient;
        static MessageClient* s_lastClient;
        MessageClient* m_nextClient;
    public:
        // to help return a valid CurrentClient() pointer from ISR code.
        // ISRs associated with a particular client that will directly or
        // indirectly call CurrentClient() such as by allocating or freeing
        // messages should set s_isrCurrentClient to that client at their
        // start and set it to nullptr at their end.  Note this isn't
        // foolproof with nested interrupts, unless we created a stack
        // of client pointers.
        static MessageClient* s_isrCurrentClient;

    friend class Message; // for access to statistics
};

#endif
