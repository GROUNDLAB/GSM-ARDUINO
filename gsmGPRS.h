           /*AVR SOURCE FILES FOR GSM,SERIAL FUNCTIONALITY*\
 * * 1%                                                                    *
 * *                    Copyright (C) 2010  Justin Downs of GRounND LAB    *
 * *                     www.GroundLab.cc                                  *
 * *                                                                       *
 * * This program is free software: you can redistribute it and/or modify  *
 * * it under the terms of the GNU General Public License as published by  *
 * * the Free Software Foundation, either version 3 of the License, or     *
 * * at your option) any later version.                                    *
 * *                                                                       *
 * * This program is distributed in the hope that it will be useful,       *
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * * GNU General Public License for more details.                          *
 * *                                                                       *
 * * You should have received a copy of the GNU General Public License     *
 * * with this program.  If not, see <http://www.gnu.org/licenses/>.       * 
 **************************************************************************/


#ifndef GSMGPRS
#define GSMGPRS
#include "gsmbase.h"

class gsmGPRS : virtual public GSMbase
{

//**FULL FUNC DESCRIPTION IN SOURCE**\\

	private:
	//you can set up diffrent contexts with diffrent IPs
	#define IPsize 16
	char ipAddress1[IPsize]; 	//IP 000.000.000.000 + "/0" for first context
	char ipAddress2[IPsize]; 	//IP 000.000.000.000 + "/0" for second context
	char * getData;			//Use this to store persistent GET data
	public:
	gsmGPRS(Serial&, uint32_t(*)(), Serial* = NULL);
	inline const char* const getIpAddress(){return ipAddress1;}
	////////////////////////////INIT FUNC
	virtual bool init(uint16_t);                                    //INITS, CALLS BASE INIT

	////////////////////////////////////////////////////////////////////////////////SET SOCKET
	//AT+CGREG checks GPRS registration
	bool checkCGREG();
	//AT+CGDCONT sets up ISP settings
	bool setApnCGDCONT(const char * const,const char* const,
	const char* const,const char* const requestedStaticIP ="0.0.0.0", 
	const char* const dataCompression = "0",const char* const headerCompression = "0");
	//AT+SCFG sets up TCP/IP socket behavior
	bool setTcpIpStackSCFG(const char * const,
	const char* const,const char* const minPacketSize = "300",
	const char* const globalTimeout = "90", const char* const connectionTimeout = "600",
	const char* const txTimeout = "50"); //(conTime and TXTime  are in .1 seconds)	
	//AT+SGACT activates context gets IP from gateway
	const char* const setContextSGACT(const char * const,
	const char* const,const char* const username =NULL,
	const char* const password= NULL);	
	/////OPTIONAL SOCKET SETTINGS
	//AT+CGQMIN defines a min quality of service for the telit to send
	bool setQualityCGQMIN(const char * const,
	const char* const precedence = "0",const char* const delay = "0",
	const char* const reliability = "0", const char* const peak = "0",
	const char* const mean = "0");
	//AT+CGQREQ asks for a specific quality of service from network	
	bool requestQualityCGQREQ(const char * const,
	const char* const precedence = "0",const char* const delay = "0",
	const char* const reliability = "3", const char* const peak = "0",
	const char* const mean = "0");
	//AT+SGACTAUTH sets security protocal used with network
	bool setSecuritySGACTAUTH(const char* const);
	////////////////////////////////////////////////////////////////////////////////SOCKET CONTROL
	//AT#SD opens a socket connection to a host at Port/IP address
	bool socketDialSD(const char * const,const char* const ,const char* const,
	const char* const);							
	bool suspendSocket();		//suspends with +++ sequence
	bool resumeSocketSO(const char* const); 	// resumes a suspended socket
	bool closeSocketSH(const char* const);	// closes a socket
	const char* const socketStatusSS(); 	// returns status of a socket.
	const char* const socketInfoSI(const char* const); //socket info
	bool socketListenSL(const char* const, const char* const,	//sets a socket to listen for data
	const char* const);					
	bool socketAcceptSA(const char* const);		//accepts socket connection after AT#SL
	const char* const getHTTP(uint16_t, const char* const,		//forms and writes a HTTP GET request 
	const char* const, const char* const httpVersion="1.1",
	bool keepAlive=false);
	const char* const postHTTP(uint16_t, const char* const, 		//forms and wtites post command
	const char* const, const char* const, const char* const httpVersion="1.1",
	bool keepAlive=false,const char* const reqStr=NULL);

	/////////////////////////////////////////////////////////////////////////////FTP CONNECTION
	bool ftpTimeOutFTPO(const char* const); 
	bool FTPOPEN(const char* const serverPort, const char* const username, 
	const char* password,const char* const mode);
	bool ftpDataTypeFTPTYPE(const char* const binaryAscii);
	bool FTPCLOSE();
	bool FTPPUT(const char* const fileWriteName, const char* const data);
	const char* const FTPGET(const char* const fileName, uint16_t dataSize);
	bool changeDirFTPCWD(const char* const directory);

};

 //Gets registration status (true REGISTERED, false NOT)
inline bool gsmGPRS::checkCGREG(){
//RETURNS: +CREG: 0,1  OK
	if ( (sendRecATCommandSplit("AT+CGREG?",":,",2)[0]) == '1') return true;
return false;
}



#endif 
