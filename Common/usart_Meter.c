#include <usart.h>
#include "usart_Meter.h"

// Initilize USART
// supported input baud rate: 9600, 10417, 19200, 57600, 115200
#if defined (SINGLE_PHASE_BOTTOM_BOARD) || defined (THREE_PHASE_BOTTOM_BOARD)
int InitUSART(int baud)
{
	if(9600 == baud)
	{
		// error rate: 0.16%
		OpenUSART(
				USART_TX_INT_OFF
				& USART_RX_INT_ON
				& USART_ASYNCH_MODE
				& USART_EIGHT_BIT
				& USART_CONT_RX
				& USART_BRGH_HIGH,
				207);
	}
	else if(10417 == baud)
	{
		// error rate: -0.003%
		OpenUSART(
				USART_TX_INT_OFF
				& USART_RX_INT_ON
				& USART_ASYNCH_MODE
				& USART_EIGHT_BIT
				& USART_CONT_RX
				& USART_BRGH_HIGH,
				191);
	}
	else if(19200 == baud)
	{
		// error rate: 0.16%
		OpenUSART(
				USART_TX_INT_OFF
				& USART_RX_INT_ON
				& USART_ASYNCH_MODE
				& USART_EIGHT_BIT
				& USART_CONT_RX
				& USART_BRGH_HIGH,
				103);
	}
	else if(57600 == baud)
	{
		// error rate: -0.79%
		OpenUSART(
				USART_TX_INT_OFF
				& USART_RX_INT_ON
				& USART_ASYNCH_MODE
				& USART_EIGHT_BIT
				& USART_CONT_RX
				& USART_BRGH_HIGH,
				34);
	}
	else if(115200 == baud)
	{
		// error rate: 0.64%
		OpenUSART(
				USART_TX_INT_OFF
				& USART_RX_INT_ON
				& USART_ASYNCH_MODE
				& USART_EIGHT_BIT
				& USART_CONT_RX
				& USART_BRGH_HIGH,
				68);
		baudUSART(BAUD_16_BIT_RATE);
	}
	else
	{
		// input baud rate is not supported
		return -1;
	}
	return 1;
}

void ClearOverrunError(void)
{
	if( RCSTAbits.OERR )
	{
		RCSTAbits.CREN = 0;
		RCSTAbits.CREN = 1;
	}
}

#elif defined METER_TOP_BOARD

int InitUSART(int baud)
{
	if(9600 == baud)
	{
		// error rate: 0.16%
		Open1USART(
				USART_TX_INT_OFF
				& USART_RX_INT_ON
				& USART_ASYNCH_MODE
				& USART_EIGHT_BIT
				& USART_CONT_RX
				& USART_BRGH_LOW,
				77);
	}
	else if(10417 == baud)
	{
		// error rate: -0.003%
		Open1USART(
				USART_TX_INT_OFF
				& USART_RX_INT_ON
				& USART_ASYNCH_MODE
				& USART_EIGHT_BIT
				& USART_CONT_RX
				& USART_BRGH_LOW,
				71);
	}
	else if(19200 == baud)
	{
		// error rate: 0.16%
		Open1USART(
				USART_TX_INT_OFF
				& USART_RX_INT_ON
				& USART_ASYNCH_MODE
				& USART_EIGHT_BIT
				& USART_CONT_RX
				& USART_BRGH_LOW,
				38);
	}
	else if(57600 == baud)
	{
		// error rate: 0.16%
		Open1USART(
				USART_TX_INT_OFF
				& USART_RX_INT_ON
				& USART_ASYNCH_MODE
				& USART_EIGHT_BIT
				& USART_CONT_RX
				& USART_BRGH_LOW,
				12);
	}
	else if(115200 == baud)
	{	
		// error rate: 0.16%
		Open1USART(
				USART_TX_INT_OFF
				& USART_RX_INT_ON
				& USART_ASYNCH_MODE
				& USART_EIGHT_BIT
				& USART_CONT_RX
				& USART_BRGH_HIGH,
				25);
	}
	else
	{
		// input baud rate is not supported
		return -1;
	}
	return 1;
}

void ClearOverrunError(void)
{
	if( RCSTA1bits.OERR )
	{
		RCSTA1bits.CREN = 0;
		RCSTA1bits.CREN = 1;
	}
}	

#endif
