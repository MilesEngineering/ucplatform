#ifdef BUILD_SPEC_sam
#include "services/ioport/ioport.h"
#include "status_codes.h"
#include "services/serial/sam_uart/uart_serial.h"
#include "utils/stdio/stdio_serial/stdio_serial.h"

// from MsgToolsExample/embedded/samv71/asf/sam/applications/same70_xplained_demo/main.c
/* on schematic edbg uart = pa21 pb4 = usart1
From MsgToolsExample/embedded/samv71/asf/sam/boards/same70_xplained/same70_xplained.h:85:#define CONSOLE_UART               USART1
*/

//# Add use of Segger RTT, and output via Messages.
//# All debug output should go to segger RTT because it's highest performance
//# with lowest overhead.  Because that only works when JLink is connected,
//# also use runtime filtering to select some subset to go out via Messages.
//# Perhaps segger RTT can be put in memory that isn't cleared on software reset,
//# and segger RTT memory can be polled on startup to re-transmit messages that
//# were related to a crash that may not have been output before the crash occurred.
void configure_console(int peripheral_id)
{
	const usart_serial_options_t uart_serial_options = {
		/*.baudrate =*/		115200UL,
		/*.charlength =*/	US_MR_CHRL_8_BIT,
		/*.paritytype =*/	US_MR_PAR_NO,
		/*.stopbits =*/		US_MR_NBSTOP_1_BIT,
	};

    sysclk_enable_peripheral_clock(peripheral_id);

    if(peripheral_id == ID_USART0)
    {
        ioport_set_pin_peripheral_mode(USART0_RXD_GPIO, USART0_RXD_FLAGS);
        ioport_set_pin_peripheral_mode(USART0_TXD_GPIO, USART0_TXD_FLAGS);
        stdio_serial_init(USART0, &uart_serial_options);
    }
    else if(peripheral_id == ID_USART1)
    {
        ioport_set_pin_peripheral_mode(USART1_RXD_GPIO, USART1_RXD_FLAGS);
        MATRIX->CCFG_SYSIO |= CCFG_SYSIO_SYSIO4;
        ioport_set_pin_peripheral_mode(USART1_TXD_GPIO, USART1_TXD_FLAGS);
        stdio_serial_init(USART1, &uart_serial_options);
    }
    else
    {
        //# If we've failed above, printf won't work...
        printf("What I/O pins for Console on peripheral %d?\n", peripheral_id);
    }

}
#else
void configure_console()
{
}
#endif
