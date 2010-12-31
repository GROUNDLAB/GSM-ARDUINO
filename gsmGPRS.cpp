/*AVR SOURCE FILES FOR GSM,SERIAL FUNCTIONALITY                         *
*                     Copyright (C) 2010  Justin Downs of GRounND LAB	*
*                     www.GroundLab.cc     1%                          	*
*                                                                 	*
* This program is free software: you can redistribute it and/or modify 	*
* it under the terms of the GNU General Public License as published by 	*
* the Free Software Foundation, either version 3 of the License, or    	*
* at your option) any later version.                                   	*
*                                                                      	*
* This program is distributed in the hope that it will be useful,      	*
* but WITHOUT ANY WARRANTY; without even the implied warranty of       	*
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        	*
* GNU General Public License for more details.                         	*
*                                                                      	*
* You should have received a copy of the GNU General Public License    	*
* with this program.  If not, see <http://www.gnu.org/licenses/>.      	* 
************************************************************************/


#include "gsmGPRS.h"

gsmGPRS::gsmGPRS(Serial& telit, uint32_t(*millis)(), Serial* debug):
GSMbase(telit, millis, debug), getData(NULL)
{
	memset(ipAddress1,'\0',IPsize);
	memset(ipAddress2,'\0',IPsize);
}


//////////////////////////////////////////////////////////////////////INIT FUNC
bool gsmGPRS::init(uint16_t _band){
	GSMbase::init(_band);
return 1;
}
//////////////////////////////////////////////////////////////////////INIT FUNC*

//////////////////////////////////////////////////////////////////////////////////////SOCKET CONFIG
//AT+CGDCONT sets up ISP information for the context 
//example string would be 2,"IP","wap.cingular","0.0.0.0",0,0
//You would say: 
//setApnCGDCONT("2","IP","wap.cingular","0.0.0.0",0,0); 
//if static ip=0.0.0.0 dynamic assignment from gateway. 
//**context can only be numbered 1-5, 0 is always for SMS context.
bool gsmGPRS::setApnCGDCONT(const char * const userSetContextID ,const char* const PDPtype,
const char* const APN ,const char* const requestedStaticIP, const char* const dataCompression,
const char* const headerCompression){
//RETURNS: OK
	
	telitPort.write("AT+CGDCONT=");
	telitPort.write(userSetContextID);
	telitPort.write(",\"");
	telitPort.write(PDPtype);
	telitPort.write("\",\"");
	telitPort.write(APN);
	telitPort.write("\",\"");
	telitPort.write(requestedStaticIP);
	telitPort.write("\",");
	telitPort.write(dataCompression);
	telitPort.write(",");
	telitPort.write(headerCompression);
	telitPort.write("\r");


return catchTelitData(2000,1);		//make it a quick one 2 sec timeout
}


//AT+SCFG is used to set the socket connection settings
bool gsmGPRS::setTcpIpStackSCFG(const char * const userSetConnectionID ,
const char* const userSetContextID,const char* const minPacketSize ,
const char* const globalTimeout, const char* const connectionTimeout,
const char* const txTimeout){
//RETURNS: OK
	
	telitPort.write("AT#SCFG=");
	telitPort.write(userSetConnectionID);
	telitPort.write(",");
	telitPort.write(userSetContextID);
	telitPort.write(",");			//telit defaults
	telitPort.write(minPacketSize);		//min number of bytes before sending 300
	telitPort.write(",");
	telitPort.write(globalTimeout);		//90 seconds
	telitPort.write(",");
	telitPort.write(connectionTimeout);	//60 seconds
	telitPort.write(",");
	telitPort.write(txTimeout);		//5 seconds
	telitPort.write("\r");

return catchTelitData(2000,1);		//make it a quick one 2 sec timeout
}


//AT+SGACT activates/closes (statusOfConnection=1/0) context, gets IP from gateway
const char* const gsmGPRS::setContextSGACT(const char * const userSetConnectionID ,
const char* const statusOfConnection,const char* const username,
const char* const password){
//RETURNS: OK
	
	telitPort.write("AT#SGACT=");
	telitPort.write(userSetConnectionID);
	telitPort.write(",");
	telitPort.write(statusOfConnection);		//"1" is connect "0" disconnect
	if(username != NULL){
		telitPort.write(",");		
		telitPort.write(username);		//if username is needed
	}
	if(password != NULL){
		telitPort.write(",");
		telitPort.write(password);		//if password needed
	}
	telitPort.write("\r");
return catchTelitData();
//return parseData(catchTelitData(),"\"","\"");		//returns IP address
}


///////////////////////////////////OPTIONAL config
//
//AT#TCPMAXDAT : tells server the max data a packet can hold, might be good if
//there are buffer issues.
//
//AT+CGQMIN defines a min quality of service for the telit to send.
//Telit recomends to always set to network default "00000".
bool gsmGPRS::setQualityCGQMIN(const char * const userSetConnectionID ,
const char* const precedence,const char* const delay,
const char* const reliability, const char* const peak,
const char* const mean){
//RETURNS: OK
	
	telitPort.write("AT+CGQMIN=");
	telitPort.write(userSetConnectionID);
	telitPort.write(",");
	telitPort.write(precedence);
	telitPort.write(",");		
	telitPort.write(delay);		
	telitPort.write(",");
	telitPort.write(reliability);	
	telitPort.write(",");
	telitPort.write(peak);	
	telitPort.write(",");
	telitPort.write(mean);	
	telitPort.write("\r");
return catchTelitData(2000,1);	//make it a quick one 2 sec timeout
}

//AT+CGQREQ asks for a specific quality of service from network	
bool gsmGPRS::requestQualityCGQREQ(const char * const userSetConnectionID ,
const char* const precedence,const char* const delay,
const char* const reliability, const char* const peak,
const char* const mean){
//RETURNS: OK
	
	telitPort.write("AT+CGQREQ=");
	telitPort.write(userSetConnectionID);
	telitPort.write(",");
	telitPort.write(precedence);
	telitPort.write(",");		
	telitPort.write(delay);		
	telitPort.write(",");
	telitPort.write(reliability);	
	telitPort.write(",");
	telitPort.write(peak);	
	telitPort.write(",");
	telitPort.write(mean);	
	telitPort.write("\r");

return catchTelitData(2000,1);	//make it a quick one 2 sec timeout
}


//AT+SGACTAUTH sets security protocal used with network
//0 - no authentication
//1 - PAP authentication (factory default)
//2 - CHAP authentication
bool gsmGPRS::setSecuritySGACTAUTH(const char* const securitySetting){
//RETURNS OK
	telitPort.write("AT+SGACTAUTH=");
	telitPort.write(securitySetting);	
	telitPort.write("\r");	
return  catchTelitData(2000,1);
}
/////////////////////////////////////////////////END OPTIONAL

//////////////////////////////////////////////////////////////////////////////////////SOCKET CONFIG*


//////////////////////////////////////////////////////////////////////////////////////SOCKET CONTROL
//AT#SD socket dial opens a socket to remote server.
bool gsmGPRS::socketDialSD(const char * const userSetConnectionID ,
const char* const protocol,const char* const remotePort,
const char* const ipAddress){
//RETURNS: CONNECT
	
	telitPort.write("AT#SD=");
	telitPort.write(userSetConnectionID);
	telitPort.write(",");
	telitPort.write(protocol);
	telitPort.write(",");		
	telitPort.write(remotePort);		
	telitPort.write(",\"");
	telitPort.write(ipAddress);	
	telitPort.write("\"\r");
	if(parseFind(catchTelitData(),"CONNECT"))return 1;
return 0;	
}

//look at #PADFWD AND #PADCMD, used to trigger a close on the socket
//using a delim char. EG. set '*' as send char, then when you put
//a '*' on the serial port it flushes all info out the socket and closes.

//SUSPENDSOCKET(): suspends listing to socket,socket can still receive data till
//a SH command is issued to shut the socket
bool gsmGPRS::suspendSocket(){
	telitPort.write("+++");				// escape sequence
	uint64_t startTime = millis();
        while ((millis() - startTime) < 2000);          // block 2 seconds SET WITH "gaurd time/S12"
return catchTelitData(2000,1);
}

//AT#SO reopens a suspended connection (eg suspended with +++ or timed out)
bool gsmGPRS::resumeSocketSO(const char* const whichSocket){
	telitPort.write("AT#SO=");
	telitPort.write(whichSocket);
	telitPort.write("\r");
	if(parseFind(catchTelitData(),"CONNECT"))return 1;

return 0; 
}

//AT#SH closes the socket connection, no data in or out
bool gsmGPRS::closeSocketSH(const char* const whichSocket){
	telitPort.write("AT#SH=");
	telitPort.write(whichSocket);
	telitPort.write("\r");
return catchTelitData(2000,1);				// look for a OK
}

//AT#SS can be implemented to view the status of a socket
//0 - Socket Closed.
//1 - Socket with an active data transfer connection.
//2 - Socket suspended.
//3 - Socket suspended with pending data.
//4 - Socket listening.
//5 - Socket with an incoming connection. Waiting for the user accept or
//  shutdown command.
const char* const gsmGPRS::socketStatusSS(){
//RETURNS:2,4,91.80.90.162,1000 OK
	telitPort.write("AT#SS");
//	telitPort.write(connectionID); //AT#SS=1 is not supported
	telitPort.write("\r");

return catchTelitData();
}

//AT#SI Execution command is used to get information about socket data traffic.
const char* const gsmGPRS::socketInfoSI(const char* const connectionID){
//RETURNS:2,4,91.80.90.162,1000 OK
	telitPort.write("AT#SI=");
	telitPort.write(connectionID); 
	telitPort.write("\r");

return catchTelitData();		//returns soket info like bytes sent
}


//////////////////////////////////////////////////////////////////////////SOCKET CONTROL*


//////////////////////////////////////////////////////////////////////////SOCKET LISTEN
//At#SL starts listening on a socket, you can set to automatically
//accept data with the <ListenAutoRsp> parameter set using
//AT#SCFGEXT command. Otherwise you must wait for SRING indication
//and use the AT#SA command to accept, AT#SH to reject.
bool gsmGPRS::socketListenSL(const char* const connectionID, const char* const listenState,
const char* const listenPort){
//RETURNS: OK	
	telitPort.write("AT#SL= ");
	telitPort.write(connectionID);
	telitPort.write(",");
	telitPort.write(listenState);
	telitPort.write(",");
	telitPort.write(listenPort);
	telitPort.write("\r");

return catchTelitData(2000,1);
}


//At#SA except the connection
bool gsmGPRS::socketAcceptSA(const char* const connectionID){
//RETURNS: CONNECT	
	telitPort.write("AT#SA= ");
	telitPort.write(connectionID);
	telitPort.write("\r");
if(parseFind(catchTelitData(),"CONNECT"))return 1;
return 0;
}
///////////////////////////////////////////////////////////////////////////SOCKET LISTEN*


///////////////////////////////////////////////////////////////////////////HTTP REQUESTS

//Constructs and send a GET request on opened socket
const char* const gsmGPRS::getHTTP(uint16_t dataSize, const char* const host, 
const char* const resource, const char* const httpVersion,
bool keepAlive){
		
	telitPort.write("GET ");
	telitPort.write(resource);
	telitPort.write(" HTTP/");
	telitPort.write(httpVersion);
	telitPort.write("\r\n");
	telitPort.write("HOST: ");
	telitPort.write(host);
	telitPort.write("\r\n");
	if(keepAlive)telitPort.write("Connection: keep-alive\r\n");
	telitPort.write("\r\n\r\n");

	 
	
return  catchTelitData(180000,false,dataSize,3000); 	//Set with two min(180000) general time out, 
}


//Constructs and send a POST request on opened socket
const char* const gsmGPRS::postHTTP(uint16_t dataSize,const char* const host, 
const char* const resource,const char* const secretAgent, const char* const httpVersion,
bool keepAlive, const char* const reqStr){
//RETURN:
	size_t length =strlen(reqStr);	//Convert strlength into ascii for contentLength
	char asciiLength[7+1];		//1,000,000 + 1'\0'
	itoa(length,asciiLength,10);

	telitPort.write("POST ");
	telitPort.write(resource);
	telitPort.write(" HTTP/");
	telitPort.write(httpVersion);
	telitPort.write("\r\n");
	telitPort.write("HOST: ");
	telitPort.write(host);
	telitPort.write("\r\n");
	telitPort.write("USER-AGENT: ");
	telitPort.write(secretAgent);
	telitPort.write("\r\n");
	telitPort.write("Content-Type: application/x-www-form-urlencoded\r\n"); // media type
	if(keepAlive)telitPort.write("Connection: keep-alive\r\n");
	telitPort.write("Content-Length: ");
	telitPort.write(asciiLength);
	telitPort.write("\r\n\r\n");
	telitPort.write(reqStr);
	telitPort.write("\r\n\r\n");

return catchTelitData(180000,false,dataSize,3000);	//global time out,quickCheck,datasize,baudDelay for HTTP server
}

///////////////////////////////////////////////////////////////////////////////////////HTTP REQUESTS



///////////////////////////////////////////////////////////////////////////////////////FTP
//AT#FTPO sets FTP timeouts
bool gsmGPRS::ftpTimeOutFTPO(const char* const timeOut){ 
//RETURNS: OK
	telitPort.write("AT#FTPO=");
	telitPort.write(timeOut);
	telitPort.write("\r");

return catchTelitData(2000,true);
}



//AT#FTPOPEN opens a ftp connection
//you can leave port off and it defaults to 21, mode defaults to active Telit
bool gsmGPRS::FTPOPEN(const char* const serverPort, const char* const username, 
const char* password,const char* const mode){
//RETURNS: OK
	telitPort.write("AT#FTPOPEN=\"");
	telitPort.write(serverPort);
	telitPort.write("\",\"");
	telitPort.write(username);
	telitPort.write("\",\"");
	telitPort.write(password);
	telitPort.write("\",");
	telitPort.write(mode);
	telitPort.write("\r");

return catchTelitData(2000,true);
}


//AT#FTPTYPE sets ftp transfer type 
//1 ASCII
//0 BINARY
bool gsmGPRS::ftpDataTypeFTPTYPE(const char* const binaryAscii){
//RETURN: OK
	telitPort.write("AT#FTPTYPE=");
	telitPort.write(binaryAscii);
	telitPort.write("\r");

return catchTelitData(2000,true);
}



//AT#FTPCLOSE closes FTP connection
bool gsmGPRS::FTPCLOSE(){
//RETURNS: OK
	telitPort.write("AT#FTPCLOSE");
	telitPort.write("\r");

return catchTelitData();
}


//AT#FTPPUT uploads file into selected file name
bool gsmGPRS::FTPPUT(const char* const fileWriteName, const char* const data){
//RETURNS: OK

  
	telitPort.write("AT#FTPPUT=\"");
	telitPort.write(fileWriteName);
	telitPort.write("\"\r");					 //starts connection
  	if ( !parseFind(catchTelitData(),"CONNECTED") )return 0; 
  	telitPort.write(data);
return suspendSocket(); //closes data transfer
}

//AT#FTPGET gets a specific file
const char* const gsmGPRS::FTPGET(const char* const fileName,uint16_t dataSize){
//RETURNS: FILE
	
	telitPort.write("AT#FTPGET=\"");
	telitPort.write(fileName);
	telitPort.write("\"\r");			
	if( !parseFind(catchTelitData(), "CONNECTED") )return 0;	//if its connected continue
 	const char* const getData = catchTelitData(180000,false,dataSize,3000);
	//global time out,quickCheck,datasize,baudDelay for FTP server
	suspendSocket(); //closes GET TRANSFER
return getData;
}	


//AT#FTPCWD changes directory
bool gsmGPRS::changeDirFTPCWD(const char* const directory ){
//RETURNS: OK
	telitPort.write("AT#FTPCWD=\"");
	telitPort.write(directory);
	telitPort.write("\"\r");
return catchTelitData(2000,1);
}



//AT#FTPLS  lists directory contents




