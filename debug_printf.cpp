#include "debug_printf.h"
#include "msg/message_client.h"
#include "msg/message_bus.h"

// auto-generated header files for messages to output debug info
#include "Printf.h"
#include "PrintfID.h"

// uncomment to output strings inside messages, instead of format string ID and arg list
//#define OUTPUT_DEBUG_STRINGS

#ifdef OUTPUT_DEBUG_STRINGS
#define PRIORITY_TYPE PrintfMessage::Priorities
#else
#include <stdarg.h>
#define PRIORITY_TYPE PrintfIDMessage::Priorities
#endif

void vLogStatement(PRIORITY_TYPE priority, int stream_id, int format_id, const char *fmt, int argc, va_list argp)
{
#ifdef OUTPUT_DEBUG_STRINGS
    UNUSED(format_id);
    PrintfMessage msg;
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
    PrintfIDMessage msg;
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
    int stream_id = MessageClient::CurrentClientID();
	va_list argp;
	va_start(argp, fmt);
	vLogStatement(PRIORITY_TYPE::Info, stream_id, format_id, fmt, count, argp);
	va_end(argp);
}
void _debug_warn(int count, int format_id, const char *fmt, ...)
{
    int stream_id = MessageClient::CurrentClientID();
	va_list argp;
	va_start(argp, fmt);
	vLogStatement(PRIORITY_TYPE::Warning, stream_id, format_id, fmt, count, argp);
	va_end(argp);
}
void _debug_error(int count, int format_id, const char *fmt, ...)
{
    int stream_id = MessageClient::CurrentClientID();
	va_list argp;
	va_start(argp, fmt);
	vLogStatement(PRIORITY_TYPE::Error, stream_id, format_id, fmt, count, argp);
	va_end(argp);
}
