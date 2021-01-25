#ifndef MESSAGE_BUS_H
#define MESSAGE_BUS_H

//# Need to undef these before including <map>
#undef min
#undef max

//# Either from STL, or ETL: https://www.etlcpp.com/map.html
#include <map>
#include <vector>

class MessageBus
{
    public:
        MessageBus()
        : m_subscriptions(),
          m_greedyClients()
        {
        }
        static MessageBus* Instance()
        {
            static MessageBus mb;
            return &mb;
        }
        static void SubscribeAll(MessageClient* client)
        {
            Instance()->m_greedyClients.push_back(client);
        }
        static void Subscribe(MessageClient* client, MessageIdType msgid)
        {
            Instance()->m_subscriptions.insert(std::pair<MessageIdType, MessageClient*>(msgid, client));
        }
        static void SendMessage(Message& msg, void* sender)
        {
            int subcount = Instance()->m_subscriptions.count(msg.GetMessageID()) + Instance()->m_greedyClients.size();
            if(subcount == 0)
            {
                return;
            }
            //printf("sub count = %d\n");
            
            // See who subscribed to it, and give it to them.
            int i=0;
            std::multimap<MessageIdType, MessageClient*>::iterator it;
            for (it=Instance()->m_subscriptions.equal_range(msg.GetMessageID()).first; it!=Instance()->m_subscriptions.equal_range(msg.GetMessageID()).second; ++it)
            {
                //MessageIdType id = (*it).first;
                MessageClient* c = (*it).second;
                if(sender != c)
                    DeliverToClient(c, msg, i == subcount);
                i++;
            }
            for (auto it = Instance()->m_greedyClients.begin(); it != Instance()->m_greedyClients.end(); ++it)
            {
                MessageClient* c = *it;
                if(sender != c)
                    DeliverToClient(c, msg, i == subcount);
                i++;
            }
        }
    private:
        static void DeliverToClient(MessageClient* c, Message& msg, bool last)
        {
#ifdef MSG_REFERENCE_COUNTING
            if(last)
            {
                c->DeliverMessage(msg);
            }
            else
#else
            UNUSED(last);
#endif
            {
                Message clonedMsg(msg);
                //# error counter?
                if(clonedMsg.Exists())
                {
                    c->DeliverMessage(clonedMsg);
#ifndef MSG_REFERENCE_COUNTING
                    //# since we sent a copy of the cloned message, the recipient owns it now,
                    //# and we need to make sure we don't free it here.
                    clonedMsg.m_buf = nullptr;
#endif
                }
            }
        }
        std::multimap<MessageIdType, MessageClient*> m_subscriptions;
        std::vector<MessageClient*> m_greedyClients;
};

#endif
