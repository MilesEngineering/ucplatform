#ifndef SIMULINK_MSG_CLIENT_H
#define SIMULINK_MSG_CLIENT_H

#include "msg/message_client.h"
#include "msg/message_key.h"
#include "msg/message_pool.h"

//# Need to undef these before including <map>
#undef min
#undef max

#include <map>

class SimulinkMessageClient : public MessageClient
{
    public:
        typedef void* (*CreateModelFunctionT)(void);
        typedef void (*ModelFunctionT)(void *const);
        SimulinkMessageClient(MessagePool& pool, CreateModelFunctionT create_model, ModelFunctionT init_model, ModelFunctionT step_model);
        static SimulinkMessageClient* Instance();
        void HandleReceivedMessage(Message& msg);
        void PeriodicTask();
        MessageBuffer* RegisterRxBlock(MessageIdType id, unsigned src, unsigned dst);
    private:
        std::map<MessageKey, MessageBuffer*> m_all_msgs;
        void* m_model;
        CreateModelFunctionT m_create_model;
        ModelFunctionT m_init_model;
        ModelFunctionT m_step_model;
        static SimulinkMessageClient* s_instance;
};

#endif
