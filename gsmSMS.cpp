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

#include "gsmSMS.h"
//CONSTRUCT THIS
gsmSMS::gsmSMS(Serial& _telit, uint32_t (*_millis)(), Serial* _debug)
: GSMbase(_telit ,_millis, _debug), messageList(NULL)
{}


//////////////////////////////////////////////////////////////////////INIT FUNC
bool gsmSMS::init(uint16_t _band){
	if(GSMbase::init(_band) && smsInit()) return 1;
return 0;
}

bool gsmSMS::smsInit(){

	if(!sendRecQuickATCommand("AT+CMGF=1")) return 0;             	// set tesxt mode not PDU	
	//Configuration for receive/send SMS
	if(!sendRecQuickATCommand("AT+CNMI=0,0,0,0,0")) return 0;	// set receive action, all quite on the western..
	//SET to no buffer no notification on recive using CNMI
	//USE AT+CNMI=2,2,2,1,0 to have messages send to DTE on reception.	
	//<buffer notification>, <no notification sent to DTE>, 
	//<no brodcast notification>,<no status notification>, 
	//<when buffering switches state flushes all stored notification to DTE >
	sendRecQuickATCommand("AT#SMSMODE=0");			// set extended smsmode off error
									// means not supported anyway	
return 1;
}
//////////////////////////////////////////////////////////////////////INIT FUNC*



//////////////////////////////////////////////////////////////////////SEND FUNCS
//AT+CMGS //sends SMS without storing
bool gsmSMS::sendNoSaveCMGS(const char* theNumber,const char* sendString){
//RETURNS > 
	telitPort.write("AT+CMGS=\"");
	telitPort.write(theNumber);
	telitPort.write("\"\r");
	if( parseFind(catchTelitData(),">")){
	telitPort.write(sendString);
	telitPort.write(0x1A);
	return 1;					// good send
	}else telitPort.write(0x1B);
return 0;						// if we got here it failed
}

//AT+CMGW //sends message to storage
const char* const  gsmSMS::saveMessageCMGW(
const char* theNumber,const char* sendString){
//RETURNS: +CMGW: 1    (which is address of stored message)
	telitPort.write("AT+CMGW=\"");
	telitPort.write(theNumber);
	telitPort.write("\"\r");
	if( parseFind(catchTelitData(),">")){
	telitPort.write(sendString);
	telitPort.write(0x1A);			//close CTR-Z
	}else telitPort.write(0x1B);		//bail ESC

return parseSplit(catchTelitData(),":",1);	//returns the message id number
}

//AT+CMSS //sends stored message
bool gsmSMS::sendSavedMessageCMSS(const char * const theMesNum){
//RETURNS: OK
	telitPort.write("AT+CMSS=");
	telitPort.write(theMesNum);
	telitPort.write("\r");
return	catchTelitData(10000,1);		//10000 is time out 1 means quickcheck: true

}
//////////////////////////////////////////////////////////////////////SEND FUNCS*



//////////////////////////////////////////////////////////////////////LIST MESSAGES FUNCS

//AT+CPMS="SM" // Gets number of messages in specified mem. 
//whichMemspace = 1 (gives number of SMs stored into <memr>)
//whichMemspace = 3 (gives number of SMs stored into <memw>)
//whichMemspace = 5 (gives number of SMs stored into <mems>)
//whichMemspace = 2 (gives number of SMs allowed in <mems>)
//whichMemspace = 4 (gives number of SMs allowed in <mems>)
//whichMemspace = 6 (gives number of SMs allowed in <mems>)
const char* const gsmSMS::getNumMesInMemCPMS(uint16_t whichMemSpace){
//RETURNS: +CPMS: 11,30,11,30,11,30
return sendRecATCommandSplit(" AT+CPMS=\"SM\" ",":,",whichMemSpace);
}

//Gets a numerical list of all messages seperated by ','
const char*const  gsmSMS::checkCMGDList(){
//RETURNS: +CMGD: (1,2,3 ect....),(0-4)OK  // take off last two numbers for the 0-4
return sendRecATCommandParse("AT+CMGD=?","(",")");	// get list of messages
}
//////////////////////////////////////////////////////////////////////LIST MESSAGES FUNCS*



//////////////////////////////////////////////////////////////////////READ FUNCS
//AT+CMGR=1  //reads the message 1
const char* const gsmSMS::readMessageCMGR(const char* const whichMessage ){
//RETURNS: A lot of stuff
	telitPort.write("AT+CMGR=");
	telitPort.write(whichMessage);
	telitPort.write("\r");
return	parseData(catchTelitData(),"\r\n","\r\n");
}

//AT+CMGL="REC UNREAD" returns all messages with specific listing, gives full message
//"REC UNREAD" - new message
//"REC READ" - read message
//"STO UNSENT" - stored message not yet sent
//"STO SENT" - stored message already sent
//"ALL" - all messages.
const char* const gsmSMS::readAllCMGL(const char* const messageType, uint16_t dataSize){

	telitPort.write("AT+CMGL=\"");
	telitPort.write(messageType);
	telitPort.write("\"\r");
return	catchTelitData(10000,false,dataSize);		//set timeout and datasize
}
//////////////////////////////////////////////////////////////////////READ FUNCS



//////////////////////////////////////////////////////////////////////DELETE FUNCS

//AT+CMGD=1 // searches and deletes a specific message or all messages of a type
//If passed 1 arg EG: "AT+CMGD=1" it erases that message.
//If passed 2 args EG: "AT+CMGD=1,1" the second arg chooses all messages of that kind to erase.
//**********************
//1 - delete all read messages from <memr> storage, leaving unread
//messages and stored mobile originated messages (whether sent or not)
//untouched
//2 - delete all read messages from <memr> storage and sent mobile
//originated messages, leaving unread messages and unsent mobile
//originated messages untouched
//3 - delete all read messages from <memr> storage, sent and unsent
//mobile originated messages, leaving unread messages untouched
//4 - delete all messages from <memr> storage.
bool gsmSMS::deletMessagesCMGD(const char* const whichMessages){
//RETURNS: OK
	telitPort.write("AT+CMGD=");
	telitPort.write(whichMessages);
	telitPort.write("\r");
return	catchTelitData();
}
//////////////////////////////////////////////////////////////////////DELETE FUNCS




//**********************************************************************************
/*
// *OPTIONAL* USE IF YOU NEED TO SPECIFY SERVICE CENTER NUMBER
//Checks the service center number to use.
const char* const checkCSCA(){
//RETURNS: +CSCA: "+13123149810",145
 if ( parsedData = sendRecATCommandSplit("AT+CSCA?",":,",1) ) return parsedData;
return 0;
}

//Sets the service center number to use.
bool setCSCA(const char* theNumber){
//Returns: OK
	char str[30]{"AT+CSCA=+"};
	strcat(str,theNumber);
	strcat(str,",145");
	if(!sendRecQuickATCommand(str))return 0;             //set number mode not PDU
return 1;
}
*/
//**********************************************************************************

