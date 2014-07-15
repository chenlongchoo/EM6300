/*********************************************************************
 *
 *                  Headers for TCPIP Demo App
 *
 *********************************************************************
 * FileName:        MainDemo.h
 * Dependencies:    Compiler.h

 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 *
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * E. Wood				4/26/08 Copied from MainDemo.c
 ********************************************************************/
#ifndef _MAINDEMO_H
#define _MAINDEMO_H


#define BAUD_RATE       (19200)		// bps

extern METER_STATUS_FLAG meter_flag; //2012-10-02 Liz added

void SaveAppConfig(void);
void SMTPDemo(void);
void PingDemo(void);
void SNMPTrapDemo(void);
void GenericTCPClient(void);
void GenericTCPServer(void);
void BerkeleyTCPClientDemo(void);
void BerkeleyTCPServerDemo(void);
void BerkeleyUDPClientDemo(void);

void CustomClearWdt(void);

#endif // _MAINDEMO_H
