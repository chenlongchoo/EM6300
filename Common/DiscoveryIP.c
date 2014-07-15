
#define __DISCOVERYIP_C

#include "TCPIPConfig.h"

#if defined(STACK_USE_ANNOUNCE)

#include "TCPIP Stack/TCPIP.h"
#include "AppConfig.h"

// The announce port
#define ANNOUNCE_PORT	30303

//extern NODE_INFO remoteNode;

/*********************************************************************
 * Function:        void DiscoveryTask(void)
 *
 * Summary:         Announce callback task.
 *
 * PreCondition:    Stack is initialized()
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Recurring task used to listen for Discovery
 *                  messages on the specified ANNOUNCE_PORT.  These
 *                  messages can be sent using the Microchip Device
 *                  Discoverer tool. If one is received, this
 *                  function will transmit a reply.
 *
 * Note:            A UDP socket must be available before this 
 *					function is called.  It is freed at the end of 
 *					the function.  MAX_UDP_SOCKETS may need to be 
 *					increased if other modules use UDP sockets.
 ********************************************************************/
void DiscoveryIP(void)
{
	static enum {
		DISCOVERY_HOME = 0,
		DISCOVERY_LISTEN,
		DISCOVERY_DISABLED
	} DiscoverySM = DISCOVERY_HOME;

	static UDP_SOCKET	MySocket;
	BYTE 				i;
	
	switch(DiscoverySM)
	{
		case DISCOVERY_HOME:
		{
			// Open a UDP socket for inbound and outbound transmission
			// Since we expect to only receive broadcast packets and 
			// only send unicast packets directly to the node we last 
			// received from, the remote NodeInfo parameter can be anything
			//MySocket = UDPOpen(ANNOUNCE_PORT, NULL, ANNOUNCE_PORT);
			MySocket = UDPOpenEx(0,UDP_OPEN_SERVER,2860, 2860);

			if(MySocket == INVALID_UDP_SOCKET)
				return;
			else
				DiscoverySM++;
		}
			break;

		case DISCOVERY_LISTEN:
		{
			WORD w = 0;
			char s[50], is_new_meter = 0;
			IP_ADDR ip;

			// Check if UDPSocket is connected
			if(!UDPIsOpened(MySocket))
			{
				UDPClose(MySocket);
				UDPClose(MySocket);
				DiscoverySM = DISCOVERY_HOME;
				return;
			}

			// Do nothing if no data is waiting
			if(!UDPIsGetReady(MySocket))
				return;
			
			// Get data
			w = UDPIsGetReady(MySocket);
			if(w <= 50)
				UDPGetArray(&s[0], w);
			else
				UDPGetArray(&s[0], 50);
			
			UDPDiscard();

			// Change the destination to the unicast address of the last received packet
        	//memcpy((void*)&UDPSocketInfo[MySocket].remote.remoteNode, (const void*)&remoteNode, sizeof(remoteNode));
			
			// We received a discovery response, check for duplicate inside list before update iplist
				
			{
				BYTE m_i = 0;
				
				is_new_meter = 1;
				m_i = 0;
				do{
					NODE_INFO node = *((NODE_INFO*)&iplist.v[m_i*10]);
					if( UDPSocketInfo[MySocket].remote.remoteNode.MACAddr.v[5] != node.MACAddr.v[5] ||
		      			UDPSocketInfo[MySocket].remote.remoteNode.MACAddr.v[4] != node.MACAddr.v[4] ||
	  	      			UDPSocketInfo[MySocket].remote.remoteNode.MACAddr.v[3] != node.MACAddr.v[3] ||
						UDPSocketInfo[MySocket].remote.remoteNode.MACAddr.v[2] != node.MACAddr.v[2] ||
		      			UDPSocketInfo[MySocket].remote.remoteNode.MACAddr.v[1] != node.MACAddr.v[1] ||
	  	      			UDPSocketInfo[MySocket].remote.remoteNode.MACAddr.v[0] != node.MACAddr.v[0] )
							is_new_meter = 1;
					else
							is_new_meter = 0;
					m_i++;				
				}while(is_new_meter==1 && m_i<10);

				if(is_new_meter == 1)
				{
					for(m_i=0; m_i<6; m_i++)
						((NODE_INFO*)&iplist.v[iplist.Val.availableMeters*10])->MACAddr.v[m_i] = UDPSocketInfo[MySocket].remote.remoteNode.MACAddr.v[m_i];

					((NODE_INFO*)&iplist.v[iplist.Val.availableMeters*10])->IPAddr.Val = UDPSocketInfo[MySocket].remote.remoteNode.IPAddr.Val;
					
					// Update number of available meters and save IPlist into eeprom
					if(iplist.Val.availableMeters < 9)	iplist.Val.availableMeters++;
					iplist.Val.Flags.bIsModified = 1;
				}
			}
		}
			break;

		case DISCOVERY_DISABLED:
			break;
	}	

}

void ClearIPLIST(void)
{
	BYTE i, j;
	
	for(i=0; i<10; i++)
	{
		for(j=0; j<6; j++)
			((NODE_INFO*)&iplist.v[i*10])->MACAddr.v[j] = 0;

		((NODE_INFO*)&iplist.v[i*10])->IPAddr.Val = 0;
	}	
	
	iplist.Val.availableMeters = 0;			
}

// Liz 2012-02-01: Broadcast TCP/IP Discovery request
BOOL DiscoveryTool(void)
{
	UDP_SOCKET	MySocket;
	BYTE 		i;

	if(!MACIsLinked())  // Check for link before blindly opening and transmitting (similar to DHCP case)
		return 0;

	// Open a UDP socket for outbound broadcast transmission
	MySocket = UDPOpenEx(0,UDP_OPEN_SERVER,2860, 30303);

	// Abort operation if no UDP sockets are available
	// If this ever happens, incrementing MAX_UDP_SOCKETS in 
	// StackTsk.h may help (at the expense of more global memory 
	// resources).
	if(MySocket == INVALID_UDP_SOCKET)
		return -1;
	
	// Make certain the socket can be written to
	while(!UDPIsPutReady(MySocket));
	
	// Begin sending discovery message in human readable form. It also 
	// would be lost if this broadcast packet were forwarded by a
	// router to a different portion of the network (note that 
	// broadcasts are normally not forwarded by routers).
	//UDPPut('D');
	UDPPutROMString((ROM BYTE*)"Discovery: Who is out there?");

	// Send some other human readable information.
	//UDPPutROMString((ROM BYTE*)"\r\nDHCP/Power event occurred");

	// Send the packet
	UDPFlush();
	
	// Close the socket so it can be used by other modules
	UDPClose(MySocket);

	return 1;
}
//


#endif //#if defined(STACK_USE_ANNOUNCE)
