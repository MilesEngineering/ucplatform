#ifndef MESSAGE_KEY_H__
#define MESSAGE_KEY_H__

struct MessageKey
{
    union {
        struct {
            unsigned id : 16;
            unsigned src : 8;
            unsigned dst : 8;
        } fields;
        uint32_t value;
    };
    MessageKey(unsigned id, unsigned src=0, unsigned dst=0)
    {
        fields.id = id;
        fields.src = src;
        fields.dst = dst;
    }
    MessageKey(Message& msg)
    : MessageKey(msg.GetMessageID(), msg.GetSource(), msg.GetDestination())
    {
    }
    bool operator <(const MessageKey& rhs) const
    {
        return value < rhs.value;
    }
};

#endif
