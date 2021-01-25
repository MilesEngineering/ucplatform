#ifndef SERIAL_CLIENT_H__
#define SERIAL_CLIENT_H__

#include "FreeRTOS.h"
#include "task.h"
#include "message_client.h"
#include "message_bus.h"
#include "message.h"
#include "message_pool.h"
#include "message_queue.h"
#include "headers/SerialHeader.h"

class SerialHeaderWithHelpers : public SerialHeader
{
    public:
        SerialHeaderWithHelpers();
        void InitializeFromMessage(const Message& msg);
        void SetMessage(Message& msg) const;
        bool HeaderValid();
        bool BodyValid(const Message& msg);
};

class SerialClient : public MessageClient
{
    public:
        SerialClient(const char* name, MessagePool& pool);
        void receiveByte(uint8_t byte);
    protected:
        SerialHeaderWithHelpers rxInProgressHdr;
        Message rxInProgressMsg;
        int rxProgress;
};

class UsartClient : public SerialClient
{
    public:
        UsartClient(const char* name, Usart* uart, MessagePool& pool);
        virtual void Initialize() override;
        void HandleReceivedMessage(Message& msg) override;
        static UsartClient* Usart0(MessagePool* p=0)
        {
            static UsartClient usart("USART0", USART0, *p);
            return &usart;
        }
        void PeriodicTask() override
        {
            printf("UsartClient::PeriodicTask()\n");
        }
        void HandleInterrupt();
    private:
        bool sendNextByte(uint8_t& byte);
    private:
        Usart* m_uart;
        // this queue is inserted into by main thread, removed from by ISR
        MessageQueue m_txMsgs;
        SerialHeaderWithHelpers txInProgressHdr;
        Message txInProgressMsg;
        int txProgress;
};

class UsbCdcClient : public SerialClient
{
    public:
        UsbCdcClient(MessagePool& pool);
        virtual void Initialize() override;
        void HandleReceivedMessage(Message& msg) override;
        static UsbCdcClient* Instance(MessagePool* p=0)
        {
            static UsbCdcClient c(*p);
            return &c;
        }
        void PeriodicTask() override;
        bool m_connected = false;
    private:
        Usbhs* m_usb;
};

#endif
