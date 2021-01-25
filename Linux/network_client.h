#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#include <sys/socket.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <signal.h>

#include "Network/Connect.h"
#include "Network/MaskedSubscription.h"

class NetworkClient : public MessageClient
{
    public:
        NetworkClient(MessagePool& pool, int port=5678)
        : MessageClient("Network", &pool, 100),
          m_sock(0),
          m_port(port),
          m_gotHdr(false),
          m_connectRetries(25),
          m_buf(0)
        {
            MessageBus::SubscribeAll(this);
        }
        void OpenSocket()
        {
            m_sock = socket(AF_INET, SOCK_STREAM, 0);
            if(m_sock < 0) 
            {
                printf("\n Socket creation error \n");
                return;
            }
            struct sockaddr_in serv_addr;
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(m_port);
            
            if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
            {
                printf("\ninet_pton Failed \n");
                close(m_sock);
                m_sock = 0;
                return;
            }
            
            if (connect(m_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            {
                printf("\nConnection Failed \n");
                close(m_sock);
                m_sock = 0;
                return;
            }
            {
                ConnectMessage cm;
                strcpy((char*)cm.Name(), "FreeRTOS");
                SendSocketMsg(cm);
            }
            {
                MaskedSubscriptionMessage subscribeMsg;
                SendSocketMsg(subscribeMsg);
            }
        }
        void HandleReceivedMessage(Message& msg)
        {
            // send to network!
            SendSocketMsg(msg);
        }
        void SendSocketMsg(Message& msg)
        {
            if(m_sock)
                send(m_sock, msg.GetDataPointer(), msg.GetTotalLength(), 0);
        }
        void PeriodicTask()
        {
            if(m_sock == 0 && ++m_connectRetries > 25)
            {
                m_connectRetries = 0;
                OpenSocket();
            }
            if(m_sock == 0)
                return;
            // read from network!
            if(m_buf == 0)
            {
                m_buf = GetMessagePool()->Allocate(0);
                if(m_buf == 0)
                    printf("Alloc returned NULL!\n");
            }
            if(m_buf)
            {
                if(!m_gotHdr)
                {
                    int ret = recv(m_sock, &m_buf->m_hdr, sizeof(m_buf->m_hdr), MSG_DONTWAIT);
                    if(errno != 11)
                        printf("%d = recv(%d), errno = %d\n", ret, (int)sizeof(m_buf->m_hdr), errno);
                    if(ret < 0)
                    {
                        return;
                    }
                    else if(ret == sizeof(m_buf->m_hdr))
                    {
                        //printf("    Got hdr for %d byte msg\n", m_buf->m_hdr.GetDataLength());
                        m_gotHdr = true;
                    }
                    else
                    {
                        printf("    recv ret %d\n", ret);
                    }
                }
                if(m_gotHdr)
                {
                    int len = m_buf->m_hdr.GetDataLength();
                    if(len > m_buf->m_bufferSize)
                    {
                        printf("Error!  Received message too big for buffer!\n");
                        return;
                    }
                    int ret = recv(m_sock, m_buf->m_data, len, MSG_DONTWAIT);
                    if(ret < 0)
                    {
                        return;
                    }
                    else if(ret == len)
                    {
                        //printf("Got body\n");
                        Message msg(m_buf);
                        SendMessage(msg);
                        m_buf = 0;
                        m_gotHdr = false;
                    }
                }
            }
        }
    private:
        int m_sock;
        int m_port;
        bool m_gotHdr;
        int m_connectRetries;
        MessageBuffer* m_buf;
};

#endif
