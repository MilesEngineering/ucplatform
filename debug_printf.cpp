#include "debug_printf.h"
#include "msg/message_client.h"
#include "msg/message_bus.h"
#include "debug_server.h"
#include <string.h>

// auto-generated header files for messages to output debug info
#include "Debug.h"

#ifdef ENABLE_DEBUG_MSG_STRINGS
#define PRIORITY_TYPE PrintfMessage::Priorities
#else
#include <stdarg.h>
#define PRIORITY_TYPE PrintfIDMessage::Priorities
#endif

#if defined(ENABLE_DEBUG_SEGGER_RTT) || defined (ENABLE_DEBUG_SEGGER_RTT_STRINGS)
#include "rtt/RTT/SEGGER_RTT.h"
#define SEGGER_RTT_BUFFER_INDEX_FOR_PRINT 0
#endif

#ifdef ENABLE_DEBUG_SEGGER_RTT
#include "tick.h"
#endif

int global_debug_threshold = (int)PRIORITY_TYPE::Warning;

#ifdef ENABLE_DEBUG_SEGGER_RTT
struct SeggerRttStruct
{
    uint8_t stream_id;
    uint8_t priority;
    uint16_t format_id;
    uint32_t time;
    uint32_t args[4];
};
#endif

void vLogStatement(PRIORITY_TYPE priority, int format_id, const char *fmt, int argc, va_list argp)
{
#ifdef ENABLE_DEBUG_STDOUT
    {
        vprintf(fmt, argp);
    }
#endif
#ifdef ENABLE_DEBUG_SEGGER_RTT_STRINGS
    {
        //# Don't do string formatting here.  Instead call SEGGER_RTT_Write()
        //# or SEGGER_RTT_WriteNoLock/SEGGER_RTT_WriteSkipNoLock/
        //# SEGGER_RTT_ASM_WriteSkipNoLock/SEGGER_RTT_WriteWithOverwriteNoLock
        //# to write binary data, and then update MsgDebug on PC to read from
        //# RTT (either directly via Segger's API, or by opening telnet on port
        //# 19021 to a Segger RTT server.
        va_list copy;
        va_copy(copy, argp);
        SEGGER_RTT_vprintf(SEGGER_RTT_BUFFER_INDEX_FOR_PRINT, fmt, &copy);
        va_end(copy);
    }
#endif
#if defined(ENABLE_DEBUG_MSGS) || defined(ENABLE_DEBUG_MSG_STRINGS) || defined(ENABLE_DEBUG_SEGGER_RTT)
    int stream_id = 0;
    int required_priority = global_debug_threshold;
    MessageClient* c = MessageClient::CurrentClient();
    if(c)
    {
        stream_id = c->ID();
        required_priority = c->DebugThreshold();
    }
#endif
#ifdef ENABLE_DEBUG_SEGGER_RTT
    {
        SeggerRttStruct segger_struct;
        segger_struct.stream_id = stream_id;
        segger_struct.priority = (int)priority;
        segger_struct.format_id = format_id;
        segger_struct.time = GetTickCount();
        for (int i=0; i<argc; i++)
        {
            segger_struct.args[i] = va_arg(argp, uint32_t);
        }
        SEGGER_RTT_Write(SEGGER_RTT_BUFFER_INDEX_FOR_PRINT, &segger_struct, offsetof(SeggerRttStruct, args) + argc*sizeof(segger_struct.args[0]));
    }
#endif
#if defined(ENABLE_DEBUG_MSGS) || defined(ENABLE_DEBUG_MSG_STRINGS) || defined(ENABLE_DEBUG_SEGGER_RTT)
    if((int)priority < required_priority)
    {
        return;
    }
#endif
#if defined(ENABLE_DEBUG_MSG_STRINGS)
    UNUSED(format_id);
    UNUSED(argc);
    PrintfMessage msg;
    if(!msg.Exists())
    {
        printf("!");
        vprintf(fmt, argp);
        return;
    }
    else
    {
        msg.SetStreamID(stream_id);
        msg.SetPriority(priority);
        int dataLen = vsnprintf((char*)msg.Buffer(), msg.GetDataLength(), fmt, argp);
        msg.Buffer()[msg.GetDataLength()-1] = '\0';
        msg.SetDataLength(dataLen);
        MessageBus::SendMessage(msg, 0);
    }
#elif defined(ENABLE_DEBUG_MSGS)
    UNUSED(fmt);
    PrintfIDMessage msg;
    if(!msg.Exists())
    {
        printf("!");
        vprintf(fmt, argp);
        return;
    }
    else
    {
        msg.SetStreamID(stream_id);
        msg.SetPriority(priority);
        msg.SetFormatStringID(format_id);
        for (int i=0; i<argc; i++)
        {
            msg.SetParameters(va_arg(argp, uint32_t), i);
        }
        msg.SetDataLength(PrintfIDMessage::ParametersFieldInfo::loc + sizeof(PrintfIDMessage::ParametersFieldInfo::min) * argc);
        MessageBus::SendMessage(msg, 0);
    }
#endif
}

void _debug_printf(int count, int format_id, const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    vLogStatement(PRIORITY_TYPE::Info, format_id, fmt, count, argp);
    va_end(argp);
}
void _debug_warn(int count, int format_id, const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    vLogStatement(PRIORITY_TYPE::Warning, format_id, fmt, count, argp);
    va_end(argp);
}
void _debug_error(int count, int format_id, const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    vLogStatement(PRIORITY_TYPE::Error, format_id, fmt, count, argp);
    va_end(argp);
}
