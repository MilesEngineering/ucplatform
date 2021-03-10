#ifndef SIMULINK_MESSAGE_INTERFACE_H__
#define SIMULINK_MESSAGE_INTERFACE_H__

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// For a given id, size, and other header fields, give back:
// 1) a pointer to the msgbuf, so the caller can call our send_msg(msgbuf) below.
// 2) a pointer to the message body, so the caller can set message fields from C code.
bool allocate_msg(uint32_t id, int size, int src, int dst, void** msgbuf, uint8_t** data);

// this takes the same pointer given by allocate_msg, sends the message and gives up ownership of it
void send_msg(void** msgbuf);

// when we get received message data, we just get a pointer to the data, possibly NULL
uint8_t* message_rx_data(uint32_t message_id, int src, int dst);

//# copied from /usr/local/MATLAB/R2020b/rtw/c/src/rt_logging.h:281 to disable
//# simulink logging.  Unclear why it's necessary to do it here and Simulink
//# doesn't take care of it.
#define rt_UpdateTXYLogVars(li, tPtr) NULL /* do nothing */

#ifdef __cplusplus
}
#endif

#endif
