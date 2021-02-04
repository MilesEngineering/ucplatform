#include "debug_printf.h"
#include "msg/message_client.h"
#include "msg/message_bus.h"
#include "debug_server.h"
#include <string.h>

// auto-generated header files for messages to output debug info
#include "Debug.h"

// uncomment to output strings inside messages, instead of format string ID and arg list
//#define OUTPUT_DEBUG_STRINGS

// uncomment to output strings to Segger J-Link RTT
#define USE_SEGGER_RTT

#ifdef OUTPUT_DEBUG_STRINGS
#define PRIORITY_TYPE PrintfMessage::Priorities
#else
#include <stdarg.h>
#define PRIORITY_TYPE PrintfIDMessage::Priorities
#endif

#ifdef USE_SEGGER_RTT
#include "rtt/RTT/SEGGER_RTT.h"
#define SEGGER_RTT_BUFFER_INDEX_FOR_PRINT 0
#endif

int global_debug_threshold = (int)PRIORITY_TYPE::Warning;

void vLogStatement(PRIORITY_TYPE priority, int format_id, const char *fmt, int argc, va_list argp)
{
    int stream_id = 0;
    int required_priority = global_debug_threshold;
    MessageClient* c = MessageClient::CurrentClient();
    if(c)
    {
        stream_id = c->ID();
        required_priority = c->DebugThreshold();
    }
#ifdef USE_SEGGER_RTT
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
    if((int)priority < required_priority)
    {
        return;
    }
#ifdef OUTPUT_DEBUG_STRINGS
    UNUSED(format_id);
    MessageBuffer* buf = DebugServer::GetBuffer(PrintfIDMessage::MSG_SIZE);
    if(buf == 0)
    {
        printf("!");
        vprintf(fmt, argp);
        return;
    }
    PrintfMessage msg(buf);
    if(msg.Exists())
    {
        msg.SetStreamID(stream_id);
        msg.SetPriority(priority);
        int dataLen = vsnprintf((char*)msg.Buffer(), msg.GetDataLength(), fmt, argp);
        msg.Buffer()[msg.GetDataLength()-1] = '\0';
        msg.SetDataLength(dataLen);
    }
#else
    UNUSED(fmt);
    MessageBuffer* buf = DebugServer::GetBuffer(PrintfMessage::MSG_SIZE);
    if(buf == 0)
    {
        printf("!");
        vprintf(fmt, argp);
        return;
    }
    PrintfIDMessage msg(buf);
    if(msg.Exists())
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
