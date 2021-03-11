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

// Client to exchange messages with TCP socket server, on PC only.
// This uses printf to print relevant messages to the console, and
// should not use ucplatform's debugPrintf, because on the PC, those go through
// the network.
class NetworkClient : public MessageClient
{
    public:
        NetworkClient(MessagePool& pool, int port=5678)
        : MessageClient("Network", &pool, 100),
          m_sock(0),
          m_port(port),
          m_connectRetries(25),
          m_rx_buf(0)
        {
            MessageBus::SubscribeAll(this);
        }
        bool OpenSocket()
        {
            if(m_sock != 0)
            {
                return true;
            }
            if(++m_connectRetries < 25)
            {
                return false;
            }
            m_connectRetries = 0;
            m_sock = socket(AF_INET, SOCK_STREAM, 0);
            if(m_sock < 0) 
            {
                printf("\n Socket creation error \n");
                return false;
            }
            struct sockaddr_in serv_addr;
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(m_port);
            
            if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
            {
                printf("\ninet_pton Failed \n");
                close(m_sock);
                m_sock = 0;
                return false;
            }
            
            if (connect(m_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            {
                printf("\nConnection Failed \n");
                close(m_sock);
                m_sock = 0;
                return false;
            }
            {
                Network_ConnectMessage cm;
                cm.CopyInName((const uint8_t*)"FreeRTOS", strlen("FreeRTOS"));
                SendSocketMsg(cm);
            }
            {
                Network_MaskedSubscriptionMessage subscribeMsg;
                SendSocketMsg(subscribeMsg);
            }
            return true;
        }
        void HandleReceivedMessage(Message& msg)
        {
            // send to network!
            SendSocketMsg(msg);
        }
        void SendSocketMsg(Message& msg)
        {
            if(m_sock)
            {
                //# This relies on MessageBuffer containing a NetworkHeader!
                //# if it doesn't, we need to declare a NetworkHeader here,
                //# and copy from the MessageBuffer's header to the NetworkHeader
                //# and send that out the socket.
                //# If MessageBuffer changed to a CANHeader, we should also modify
                //# can_client.cpp to use the NetworkHeader directly, and not copy
                //# from the MessageBuffer's header to the CANHeader.
                send(m_sock, &msg.m_buf->m_hdr, sizeof(msg.m_buf->m_hdr), 0);
                send(m_sock, msg.GetDataPointer(), msg.GetDataLength(), 0);
            }
        }
        void PeriodicTask()
        {
            // keep reading while there's data
            while(1)
            {
                // if the socket can't be opened, just return.
                // the server might not be running, and we should ignore that and
                // reconnect when the server is running.
                if(!OpenSocket())
                {
                    return;
                }
                // allocate a buffer if we need one.
                if(m_rx_buf == 0)
                {
                    //# Should allocate based on message size, if size of messages
                    //# varies and we have multiple pools.  That would require reading
                    //# the header to a temporary buffer, then allocating, copying the
                    //# temp header, and reading the data.
                    m_rx_buf = GetMessagePool()->Allocate(0);
                    if(m_rx_buf == 0)
                    {
                        printf("NetworkClient GetMessagePool()->Allocate(0) returned NULL!\n");
                        return;
                    }
                }

                //# This relies on MessageBuffer containing a NetworkHeader!
                //# if it doesn't, we need to declare a NetworkHeader here,
                //# and copy from the MessageBuffer's header to the NetworkHeader
                //# and send that out the socket.
                //# If MessageBuffer changed to a CANHeader, we should also modify
                //# can_client.cpp to use the NetworkHeader directly, and not copy
                //# from the MessageBuffer's header to the CANHeader.
                int ret = recv(m_sock, &m_rx_buf->m_hdr, sizeof(m_rx_buf->m_hdr), MSG_DONTWAIT);
                if(ret < 0)
                {
                    if(errno != EAGAIN)
                    {
                        printf("%d = recv(%d), errno = %d\n", ret, (int)sizeof(m_rx_buf->m_hdr), errno);
                    }
                    return;
                }
                else if(ret == 0)
                {
                    // recv returns zero when the socket is closed
                    close(m_sock);
                    m_sock = 0;
                    return;
                }
                else if(ret != sizeof(m_rx_buf->m_hdr))
                {
                    printf("    recv ret %d\n", ret);
                    return;
                }

                int len = m_rx_buf->m_hdr.GetDataLength();
                // special case of message body length == 0
                if(len == 0)
                {
                    //printf("Got body\n");
                    Message msg(m_rx_buf);
                    SendMessage(msg);
                    m_rx_buf = 0;
                }
                else
                {
                    if(len > m_rx_buf->m_bufferSize)
                    {
                        printf("Error!  Received message too big for buffer!\n");
                        return;
                    }
                    // do a blocking read for data, which should be available as soon as header is
                    int ret = recv(m_sock, m_rx_buf->m_data, len, 0);
                    if(ret == len)
                    {
                        //printf("Got body\n");
                        Message msg(m_rx_buf);
                        SendMessage(msg);
                        m_rx_buf = 0;
                    }
                    else if(ret == 0)
                    {
                        // recv returns zero when the socket is closed
                        close(m_sock);
                        m_sock = 0;
                    }
                    else
                    {
                        printf("    recv ret %d\n", ret);
                    }
                }
            }
        }
    private:
        int m_sock;
        int m_port;
        int m_connectRetries;
        MessageBuffer* m_rx_buf;
};

#endif
