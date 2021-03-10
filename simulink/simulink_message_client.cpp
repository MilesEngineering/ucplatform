#include "simulink_message_client.h"
#include "msg/message_bus.h"
#include "msg/message_pool.h"

SimulinkMessageClient* SimulinkMessageClient::s_instance = 0;

SimulinkMessageClient::SimulinkMessageClient
(MessagePool& pool,
 SimulinkMessageClient::CreateModelFunctionT create_model,
 SimulinkMessageClient::ModelFunctionT init_model,
 SimulinkMessageClient::ModelFunctionT step_model)
: MessageClient("Simulink", &pool, 100),
  m_create_model(create_model),
  m_init_model(init_model),
  m_step_model(step_model)
{
    m_model = m_create_model();
    m_init_model(m_model);
    //# should we subscribe to everything, or just what we care about?
    MessageBus::SubscribeAll(this);
    
    s_instance = this;
}
SimulinkMessageClient* SimulinkMessageClient::Instance()
{
    return s_instance;
}
void SimulinkMessageClient::HandleReceivedMessage(Message& msg)
{
    MessageKey key(msg);
    if(auto it{ m_all_msgs.find(key) }; it != std::end(m_all_msgs))
    {
        auto msg_ptr{ it->second };
        if(msg_ptr)
        {
            MessagePool::FreeToOwner(msg_ptr);
        }
        m_all_msgs[key] = msg.TakeBuffer();
    }
}
void SimulinkMessageClient::PeriodicTask()
{
    //# run the simulink model
    m_step_model(m_model);
}
MessageBuffer* SimulinkMessageClient::RegisterRxBlock(MessageIdType id, unsigned src, unsigned dst)
{
    MessageKey key(id, src, dst);
    if(m_all_msgs.count(key) == 0)
    {
        // store a null pointer to indicate simulink wants the message
        m_all_msgs[key] = NULL;
    }
    return m_all_msgs[key];
}
