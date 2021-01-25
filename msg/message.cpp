#include "message_pool.h"
#include "message_client.h"
#include "tick.h"

//# Need to undef these before #including <algorithm>
#undef min
#undef max
#include <algorithm> 

//# for memcpy
#include <string.h>
#include <stdio.h>

// Threshold for deciding if a message buffer is undesirably old,
// indicating a Client is holding on to messages too long before
// freeing them.
#define STALE_MSG_TICK_THRESHOLD 10

int MessageBuffer::increment_refcount()
{
#if defined(MSG_REFERENCE_COUNTING)
    int ret = std::atomic_fetch_add(&m_referenceCount, 1);
    configASSERT(ret > 0);
    return ret;
#else
    return 0;
#endif
}
int MessageBuffer::decrement_refcount()
{
#if defined(MSG_REFERENCE_COUNTING)
    return std::atomic_fetch_sub(&m_referenceCount, 1);
#else
    return 0;
#endif
}

Message::Message()
: m_buf(nullptr),
  m_data(nullptr)
{
}
Message::Message(MessageBuffer* buf)
: m_buf(buf),
  m_data(buf->m_data)
{
    //# can't increment here, we're inheriting a non-zero count from our caller (in MessageQueue::get() or MessagePool::Allocate())
    //buf->increment_refcount();
}
Message::Message(int size)
: m_buf(nullptr),
  m_data(nullptr)
{
    if(Allocate(size))
    {
        InitializeTime();
    }
}
void Message::InitializeTime()
{
    SetTime(GetTickCount());
}
bool Message::Allocate(int size)
{
    if(!m_buf)
    {
        //# could add more parameters here if needed, like source, destination?!?
        MessagePool* pool = MessagePool::CurrentPool();
        if(pool)
        {
            MessageBuffer* msg_buffer = pool->Allocate(size);
            if(msg_buffer)
            {
                m_buf = msg_buffer;
                m_data = msg_buffer->m_data;
                SetDataLength(size);
                return true;
            }
            printf("%s line %d, ERROR!  Failed to allocate\n", __FILE__, __LINE__);
        }
        else
        {
            printf("%s line %d, ERROR!  No pool defined\n", __FILE__, __LINE__);
        }
    }
    else
    {
        printf("%s line %d, ERROR!  Allocating, but already allocated\n", __FILE__, __LINE__);
    }
    return false;
}
// Copy constructor
Message::Message(const Message &rhs)
: m_buf(nullptr),
  m_data(nullptr)
{
#ifdef MSG_REFERENCE_COUNTING
    m_buf = rhs.m_buf;
    m_data = rhs.m_data;
    rhs.m_buf->increment_refcount();
#else
    if(Allocate(rhs.GetDataLength()))
    {
        // copy the contents
        memcpy(&m_buf->m_hdr, &rhs.m_buf->m_hdr, sizeof(m_buf->m_hdr));
        memcpy(m_data, rhs.m_data, std::min(GetDataLength(), rhs.GetDataLength()));
    }
#endif
}
Message::~Message()
{
    //# could add check for current time vs msg time set during allocation.
    //# that'd let us know if someone held on to a message longer than they
    //# likely should've.  perhaps have a time threshold like "max_msg_hold_time",
    //# and if more than that has elapsed, increment a stats counter in
    //# MessageClient accessible via CurrentClient(), or figure something
    //# else out for inside ISRs.  Probably most ISRs that Free messages
    //# will be subclasses of MessageClient, so that might be the right
    //# MessageClient to increment stats for.  How to get it from this
    //# function or a static member of MessageClient like CurrentClient(), though?
    //# We can of course keep one counter for *all* ISRs, separate from each client,
    //# but that won't be very useful for diagnosing issues with ISRs holding
    //# messages too long.
    MessageClient* currentClient = MessageClient::CurrentClient();
    if(currentClient)
    {
        currentClient->m_msgs_freed++;
        if(GetTickCount() > GetTime() + STALE_MSG_TICK_THRESHOLD)
        {
            currentClient->m_msgs_stale++;
        }
    }
    Deallocate();
}
void Message::Deallocate()
{
    if(m_buf != 0)
    {
        // Free back to whatever pool it came from.
        if(m_buf && m_buf->m_owner)
        {
            m_buf->m_owner->Free(m_buf);
        }
        else
        {
            printf("ERROR Deallocating!\n");
        }
        m_buf = 0;
        m_data = 0;
    }
}
void Message::SetMessageID(MessageIdType id)
{
    m_buf->m_hdr.SetMessageID(id);
}
void Message::SetDataLength(uint16_t len)
{
    m_buf->m_hdr.SetDataLength(len);
}
void Message::SetSource(int source)
{
    m_buf->m_hdr.SetSource(source);
}
void Message::SetDestination(int dest)
{
    m_buf->m_hdr.SetDestination(dest);
}
void Message::SetPriority(int prio)
{
    m_buf->m_hdr.SetPriority(prio);
}
void Message::SetTime(TimeType time)
{
    m_buf->m_hdr.SetTime(time*0.001);
}
MessageIdType Message::GetMessageID() const
{
    return m_buf->m_hdr.GetMessageID();
}
int Message::GetDataLength() const
{
    return m_buf->m_hdr.GetDataLength();
}
int Message::GetSource() const
{
    return m_buf->m_hdr.GetSource();
}
int Message::GetDestination() const
{
    return m_buf->m_hdr.GetDestination();
}
int Message::GetPriority() const
{
    return m_buf->m_hdr.GetPriority();
}
TimeType Message::GetTime() const
{
    return m_buf->m_hdr.GetTime() * 1000.0;
}
uint8_t* Message::GetDataPointer() const
{
    return m_buf->m_hdr.m_data;
}
int Message::GetTotalLength() const
{
    return sizeof(m_buf->m_hdr)+GetDataLength();
}
bool Message::Exists()
{
    return m_buf != 0 && m_data != 0;
}
