#ifndef MESSAGE_H
#define MESSAGE_H

#define MessageIdType uint32_t
#define TimeType      uint32_t

#include "headers/NetworkHeader.h"
//#include "MsgInfo.h"
#define MSG_REFERENCE_COUNTING

#ifdef MSG_REFERENCE_COUNTING
#include <atomic>
#endif

class MessagePool;

// This holds the contents of a message, including:
// 1) the header
// 2) any house-keeping data useful for allocating/deallocating/sending messages
// 3) the body of the message
//# Note that the body of the message is NOT aligned, and relies on access to
//# it via FieldAccess.h working for unaligned fields, which it currently does.
//# If we optimized FieldAccess.h to work faster for aligned fields, and/or
//# only allowed messages to contained aligned fields, we would want to change
//# this class so the body of the message is aligned.  Note that if we insert
//# padding between the header and body, that can break code that relies on
//# them being contiguous.
class MessageBuffer
{
    public:
        int increment_refcount();
        int decrement_refcount();
    private:
        MessagePool*    m_owner;
#ifdef MSG_REFERENCE_COUNTING
        std::atomic_int m_referenceCount;
#endif
        uint16_t        m_bufferSize;
        NetworkHeader   m_hdr;
        uint8_t         m_data[1];
    friend class Message;
    friend class MessagePool;
    friend class NetworkClient;
};

// this is similar to smart pointer for a Message, that uses an underlying
// pool allocator to create and destroy messages.
class Message
{
    public:
        Message();
        Message(MessageBuffer* buf);
        Message(int size);
        void InitializeTime();
        bool Allocate(int size);
        // Copy constructor
        Message(const Message &rhs);
        ~Message();
        MessageBuffer* TakeBuffer();
        void Deallocate();
        bool Exists();
        void      SetMessageID(MessageIdType id);
        void      SetDataLength(uint16_t len);
        void      SetSource(int source);
        void      SetDestination(int dest);
        void      SetPriority(int prio);
        void      SetTime(TimeType time);

        MessageIdType GetMessageID() const;
        int       GetDataLength() const;
        uint8_t*  GetDataPointer() const;
        int       GetSource() const;
        int       GetDestination() const;
        int       GetPriority() const;
        TimeType  GetTime() const;
    protected:
        MessageBuffer* m_buf;
        // auto-generated code wants 'm_data' to exist!
        uint8_t*       m_data;
    private:
        //# make this private for now.  It might be dangerous/confusing to allow
        //# people to copy messages with the assignment operator.
        void operator=(Message& rhs);
    friend class MessageQueue;
    friend class NetworkClient;
};

#endif
