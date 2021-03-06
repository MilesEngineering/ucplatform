#ifndef DEBUG_SERVER_H__
#define DEBUG_SERVER_H__

#include "message_client.h"

class DebugServer : public MessageClient
{
    public:
        DebugServer(const char* name);
        void PeriodicTask() override;
        void HandleReceivedMessage(Message& msg) override;
        virtual void Initialize() override;
        static MessageBuffer* GetBuffer(int size);
    private:
        const char* m_deviceName;
};

#endif
