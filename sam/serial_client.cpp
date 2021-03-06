#include "serial_client.h"
#include <string.h> // for memmove

#include "status_codes.h"
#include "services/serial/sam_uart/uart_serial.h"
#include "services/ioport/ioport.h"
#include "services/usb/class/cdc/device/udi_cdc.h"
#include "services/usb/udc/udc.h"
#include <inttypes.h>

#include "debug_printf.h"

#define ESCAPED_FILE_PATH ___ucplatform_sam_serial_client

static uint16_t crc16(uint8_t* data, int len)
{
    uint16_t crc = 0;
    for(int i=0; i<len; i++)
    {
        crc = (crc >> 8) | (crc << 8);
        crc ^= data[i];
        crc ^= (crc & 0xff) >> 4;
        crc ^= crc << 12;
        crc = 0xFFFF & crc;
        crc ^= (crc & 0xff) << 5;
        crc = 0xFFFF & crc;
    }
    return crc;
}

SerialHeaderWithHelpers::SerialHeaderWithHelpers()
: SerialHeader()
{
}
void SerialHeaderWithHelpers::InitializeFromMessage(const Message& msg)
{
    Init();

    SetID(msg.GetMessageID());
    SetSource(msg.GetSource());
    SetDestination(msg.GetDestination());
    SetPriority(msg.GetPriority());
    SetDataLength(msg.GetDataLength());
    SetTime(msg.GetTime());
    SetHeaderChecksum(crc16(m_data, SerialHeader::HeaderChecksumFieldInfo::loc));
    SetBodyChecksum(crc16(msg.GetDataPointer(), msg.GetDataLength()));
}
void SerialHeaderWithHelpers::SetMessage(Message& msg) const
{
    msg.SetMessageID(GetID());
    msg.SetSource(GetSource());
    msg.SetDestination(GetDestination());
    msg.SetPriority(GetPriority());
    msg.SetDataLength(GetDataLength());
    msg.SetTime(GetTime());
}

bool SerialHeaderWithHelpers::StartSequenceValid()
{
    if(GetStartSequence() != SerialHeader::StartSequenceFieldInfo::defaultValue)
    {
        //increment statistics
        return false;
    }
    return true;
}

bool SerialHeaderWithHelpers::HeaderValid()
{
    if(!StartSequenceValid())
    {
        //increment statistics
        return false;
    }
    if(crc16(m_data, SerialHeader::HeaderChecksumFieldInfo::loc) != GetHeaderChecksum())
    {
        //increment statistics
        return false;
    }
    return true;
}
bool SerialHeaderWithHelpers::BodyValid(const Message& msg)
{
    if(!HeaderValid())
    {
        //increment statistics
        return false;
    }
    uint16_t calculatedCrc = crc16(msg.GetDataPointer(), GetDataLength());
    if(calculatedCrc != GetBodyChecksum())
    {
        debugWarn("crc16[%d] 0x%02X != 0x%02X\n", GetDataLength(), calculatedCrc, GetBodyChecksum());
        //increment statistics
        return false;
    }
    return true;
}

SerialClient::SerialClient(const char* name, MessagePool& pool, int period)
: MessageClient(name, &pool, period)
{
}

void SerialClient::receiveByte(uint8_t byte)
{
    if (rxProgress < SerialHeader::SIZE)
    {
        rxInProgressHdr.m_data[rxProgress] = byte;
        rxProgress++;
        if(rxProgress == SerialHeader::SIZE)
        {
            if(rxInProgressHdr.HeaderValid())
            {
                rxInProgressMsg.Allocate(rxInProgressHdr.GetDataLength());
                if(rxInProgressMsg.Exists())
                {
                    rxInProgressHdr.SetMessage(rxInProgressMsg);
                }
                else
                {
                    //# Since the message was valid but we couldn't use it, it's correct
                    //# to throw the entire header away and resynchronize on valid header
                    //# after the end of it.  We'll of course have to throw the body away
                    //# one byte at a time until we find the next valid header.
                    printf("%s:%d, Couldn't allocate, throwing away %d bytes rx msg\n", __FILE__, __LINE__, rxInProgressHdr.GetDataLength());
                    rxProgress = 0;
                }
            }
            else
            {
                const int len_for_start_sequence = sizeof(SerialHeader::StartSequenceFieldInfo::defaultValue);
                //#printf("\nBAD HDR ");
                do
                {
                    //#printf("%02X ", rxInProgressHdr.m_data[0]);
                    memmove(rxInProgressHdr.m_data, &rxInProgressHdr.m_data[1], SerialHeader::SIZE-1);
                    rxProgress--;
                } while(rxProgress >= len_for_start_sequence && !rxInProgressHdr.StartSequenceValid());
                //#printf("\n");
            }
        }
    }
    else
    {
        rxInProgressMsg.GetDataPointer()[rxProgress - SerialHeader::SIZE] = byte;
        rxProgress++;
    }
    // This is a separate check, and not just inside the 'else' above, to
    // handle receiving messages with zero-length body.
    if(rxProgress >= SerialHeader::SIZE &&
       rxProgress == SerialHeader::SIZE + rxInProgressHdr.GetDataLength())
    {
        if(rxInProgressHdr.BodyValid(rxInProgressMsg))
        {
            //# header was already set up above, this should have no effect.
            rxInProgressHdr.SetMessage(rxInProgressMsg);
            SendMessage(rxInProgressMsg);
            printf("serial Rx %" PRId32 "\n", rxInProgressMsg.GetMessageID());
            rxInProgressMsg.Deallocate();
            rxProgress = 0;
        }
        else
        {
            // throw away one byte from header and decrement progress
            memmove(rxInProgressHdr.m_data, &rxInProgressHdr.m_data[1], SerialHeader::SIZE-1);
            rxProgress--;
            // if we still have some body left, shift it into the header
            if(rxProgress > SerialHeader::SIZE)
            {
                // put first byte of body (if exists) on end of header
                rxInProgressHdr.m_data[SerialHeader::SIZE] = rxInProgressMsg.GetDataPointer()[0];
                // shift body down one
                memmove(rxInProgressMsg.GetDataPointer(), &rxInProgressMsg.GetDataPointer()[1], rxProgress-SerialHeader::SIZE-1);
                rxProgress--;
            }
            printf("serial INVALID body\n");
        }
    }
}

UsartClient::UsartClient(const char* name, Usart* uart, MessagePool& pool)
: SerialClient(name, pool, 1000),
  m_uart(uart)
{
}
void UsartClient::Initialize()
{
    int peripheral_id = 0;
    if(m_uart == USART0)
    {
        peripheral_id = ID_USART0;
    }
    else if(m_uart == USART1)
    {
        peripheral_id = ID_USART1;
    }
    else
    {
        printf("Unrecognized USART %p\n", (void*)m_uart);
    }    
    
	const usart_serial_options_t uart_serial_options = {
		/*.baudrate =*/     115200UL,
		/*.charlength =*/   US_MR_CHRL_8_BIT,
		/*.paritytype =*/   US_MR_PAR_NO,
		/*.stopbits =*/     US_MR_NBSTOP_1_BIT,
	};
	sysclk_enable_peripheral_clock(peripheral_id);

    if(m_uart == USART0)
    {
        ioport_set_pin_peripheral_mode(USART0_RXD_GPIO, USART0_RXD_FLAGS);
        ioport_set_pin_peripheral_mode(USART0_TXD_GPIO, USART0_TXD_FLAGS);
        irq_register_handler(USART0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+1);
    }
    else if(m_uart == USART1)
    {
        ioport_set_pin_peripheral_mode(USART1_RXD_GPIO, USART1_RXD_FLAGS);
        MATRIX->CCFG_SYSIO |= CCFG_SYSIO_SYSIO4;
        ioport_set_pin_peripheral_mode(USART1_TXD_GPIO, USART1_TXD_FLAGS);
        irq_register_handler(USART1_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+1);
    }
    else
    {
        /* other options we might want to support
          UART0_IRQn
          UART1_IRQn
          UART2_IRQn
          UART3_IRQn
          UART4_IRQn
          USART2_IRQn
        */
        printf("What I/O pins for UART %d?\n", peripheral_id);
    }

    MessageBus::SubscribeAll(this);
}
void UsartClient::HandleReceivedMessage(Message& msg)
{
    m_txMsgs.put(msg);
    usart_enable_interrupt(m_uart, US_IER_TXRDY);
}
bool UsartClient::sendNextByte(uint8_t& byte)
{
    if(txProgress > 0)
    {
        if(txProgress < SerialHeader::SIZE)
        {
            byte = txInProgressHdr.m_data[txProgress];
            txProgress++;
            return true;
        }
        else if(txProgress < SerialHeader::SIZE + txInProgressMsg.GetDataLength())
        {
            byte = txInProgressMsg.GetDataPointer()[txProgress - SerialHeader::SIZE];
            txProgress++;
            return true;
        }
        else
        {
            // close out the message we finished sending
            txInProgressMsg.Deallocate();
            txProgress = 0;
        }
    }
    MessageBuffer* msgbuf = m_txMsgs.get(0);
    if(msgbuf)
    {
        // re-construct message using placement new.
        new (&txInProgressMsg) Message(msgbuf);

        txInProgressHdr.InitializeFromMessage(txInProgressMsg);
        byte = txInProgressHdr.m_data[0];
        txProgress++;
        return true;
    }
    return false;
}
void UsartClient::HandleInterrupt()
{
	uint32_t status = usart_get_status(m_uart);
    // if no interrupts for us occurred, return quickly without setting s_isrCurrentClient
    if(!status)
    {
        return;
    }

    s_isrCurrentClient = this;

    if(status & US_CSR_RXRDY)
    {
        uint32_t received_byte;
        usart_read(m_uart, &received_byte);
        receiveByte(received_byte);
    }
    if(status & US_CSR_TXEMPTY)
    {
        uint8_t byte_to_transmit;
        if(sendNextByte(byte_to_transmit))
        {
	        usart_write(m_uart, byte_to_transmit);
        }
        else
        {
            usart_disable_interrupt(m_uart, US_IER_TXRDY);
        }
    }

    s_isrCurrentClient = nullptr;
}

UsbCdcClient::UsbCdcClient(MessagePool& pool)
: SerialClient("USB_CDC", pool, 1000/*ms*/),
  m_usb(USBHS)
{
}
void UsbCdcClient::Woken()
{
    //# Note: Ubuntu Linux ModemManager will attempt to probe serial ports by
    //# AT commands which seem to consist of 7e 00 78 f0 7e 7e 00 78 f0 7e.
    //# So don't be surprised if you get some junk when the PC connects, even
    //# if your PC application isn't sending it.
    if(udi_cdc_is_rx_ready())
    {
        //#printf("USB read ");
        int bytes=0;
        while(udi_cdc_is_rx_ready())
        {
            uint8_t byte = udi_cdc_getc();
            //#printf("%02X ", byte);
            receiveByte(byte);
            bytes++;
        }
        //#printf("%d\n", bytes);
    }
}

void UsbCdcClient::PeriodicTask()
{
    //# Extra polling, just in case we didn't get correctly woken up.
    Woken();
}
void UsbCdcClient::HandleReceivedMessage(Message& msg)
{
    if(!m_connected)
        return;

    //# Only send the message if there's room for the whole thing.
    if(udi_cdc_get_free_tx_buffer() > SerialHeader::SIZE + (unsigned)msg.GetDataLength())
    {
        SerialHeaderWithHelpers serialHdr;
        serialHdr.InitializeFromMessage(msg);
        udi_cdc_write_buf(serialHdr.m_data, SerialHeader::SIZE);
        udi_cdc_write_buf(msg.GetDataPointer(), msg.GetDataLength());
        //#printf("USBC CD Tx[%d:%d]\n", SerialHeader::SIZE, msg.GetDataLength());
    }
    else
    {
        //# this happens a lot.  both because the UDI_CDC_DISABLE_EXT callback
        //# for disconnect is broken in ASF, and also because it seems like the
        //# USB connection will sometimes hang and need be closed and then
        //# reopened in msgserver.
        //#printf("ERROR!  USB CDC has no room for more messages!\n");
    }
}

void UsbCdcClient::Initialize()
{
	/*
	 * Start and attach USB CDC device interface for devices with
	 * integrated USB interfaces.  Assume the VBUS is present if
	 * VBUS monitoring is not available.
	 */
	udc_start ();

    MessageBus::SubscribeAll(this);
}

extern "C" void USART0_Handler(void)
{
    UsartClient::Usart0()->HandleInterrupt();
}

//# Called by UDI interface when CDC is enabled/disabled
//  by connection/disconnection with host.
extern "C" bool serial_console_cdc_enable(void)
{
    UsbCdcClient::Instance()->m_connected = true;
    printf("UDI_CDC_ENABLE_EXT()\n");
    return true;
}
//# UDI_CDC_DISABLE_EXT callback in ASF3 is broken!
//# See https://www.avrfreaks.net/forum/usb-cdc-disable-callback
extern "C" void serial_console_cdc_disable(void)
{
    UsbCdcClient::Instance()->m_connected = false;
    printf("UDI_CDC_DISABLE_EXT()\n");
}

extern "C" void serial_console_rx_notify(void)
{
    UsbCdcClient::Instance()->Wake();
    //#printf("UDI_CDC_RX_NOTIFY()\n");
}