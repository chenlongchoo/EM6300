
#define __GENERICTCPCLIENT_C

#include "TCPIPConfig.h"
#include "TCPIP Stack/TCPIP.h"


// Defines the server to be accessed for this application
static BYTE ServerName[] =	"192.168.1.111";

// Defines the port to be accessed for this application
static WORD ServerPort = 80;

// Defines the URL to be requested by this HTTP client
static ROM BYTE RemoteURL[] = "/protect/reboot.cgi";


BOOL CommRebootClient(IP_ADDR comm_ip)
{
	BOOL				result;	//2012-06-23 Liz: Reboot comm
	BYTE 				i;
	WORD				w;
	BYTE				vBuffer[30];
	static DWORD		Timer;
	static TCP_SOCKET	MySocket = INVALID_SOCKET;
	static enum _GenericTCPExampleState
	{
		SM_HOME = 0,
		SM_SOCKET_OBTAINED,
		SM_DISCONNECT
	} GenericTCPExampleState = SM_HOME; //SM_DONE;

	switch(GenericTCPExampleState)
	{
		case SM_HOME:
			// Connect a socket to the remote TCP server
			MySocket = TCPOpen(comm_ip.Val, TCP_OPEN_IP_ADDRESS, ServerPort, TCP_PURPOSE_WEB_REQUEST);
			
			// Abort operation if no TCP socket of type TCP_PURPOSE_GENERIC_TCP_CLIENT is available
			// If this ever happens, you need to go add one to TCPIPConfig.h
			if(MySocket == INVALID_SOCKET)
				break;
				
			GenericTCPExampleState++;
			Timer = TickGet();
			result = FALSE;
			break;

		case SM_SOCKET_OBTAINED:
			// Wait for the remote server to accept our connection request
			if(!TCPIsConnected(MySocket))
			{
				// Time out if too much time is spent in this state
				if(TickGet()-Timer > 5*TICK_SECOND)
				{
					// Close the socket so it can be used by other modules
					TCPDisconnect(MySocket);
					MySocket = INVALID_SOCKET;
					GenericTCPExampleState--;
					result = FALSE;
				}
				break;
			}

			Timer = TickGet();

			// Make certain the socket can be written to
			if(TCPIsPutReady(MySocket) < 125u)
			{
				result = FALSE;
				break;
			}
			// Place the application protocol data into the transmit buffer.  For this example, we are connected to an HTTP server, so we'll send an HTTP GET request.
			TCPPutROMString(MySocket, (ROM BYTE*)"GET ");
			TCPPutROMString(MySocket, RemoteURL);
			TCPPutROMString(MySocket, (ROM BYTE*)" HTTP/1.0\r\nHost: ");
			TCPPutString(MySocket, ServerName);
			TCPPutROMString(MySocket, (ROM BYTE*)"\r\nAuthorization: Basic YWRtaW46bWljcm9jaGlw");
			TCPPutROMString(MySocket, (ROM BYTE*)"\r\nConnection: close\r\n\r\n");

			// Send the packet
			TCPFlush(MySocket);
			
			{
				// Check to see if the remote node has disconnected from us.
				BYTE retry = 0;
				while(TCPIsConnected(MySocket) && retry++ > 50)
				{
					// Did not detect disconnection, meaning reboot was not successful.
					GenericTCPExampleState = SM_DISCONNECT;
					result = FALSE;
					//return FALSE;
					// Do not break;  We might still have data in the TCP RX FIFO waiting for us
				}
				
				GenericTCPExampleState = SM_DISCONNECT;
				result = TRUE;
				//return TRUE;
			}
		
			GenericTCPExampleState++;
			break;
	
		case SM_DISCONNECT:
			// Close the socket so it can be used by other modules
			// For this application, we wish to stay connected, but this state will still get entered if the remote server decides to disconnect
			TCPDisconnect(MySocket);
			MySocket = INVALID_SOCKET;
			GenericTCPExampleState = SM_HOME;
			break;
	}
	return result;
}
