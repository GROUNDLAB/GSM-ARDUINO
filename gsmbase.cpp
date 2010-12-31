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

#include "gsmbase.h"

//with debug
GSMbase::GSMbase(Serial& _telit ,
uint32_t (*_millis)(),Serial* _debug) 
:telitPort(_telit),millis(_millis),DebugPort(_debug)
,fullData(NULL),parsedData(NULL)
{}


//////////////////////////////////////////////////////////////////////SEND FUNCS
//Sends the command clear (use with catchTelitData)
void GSMbase::sendATCommand(const char* theMessageMelleMel){
	telitPort.write(theMessageMelleMel);
	telitPort.write("\r");
	
}
//Sends AT command and checks for OK or ERROR good for commands that reply OK or ERROR
const char* const GSMbase::sendRecQuickATCommand(const char* theMessageMelleMel){
	telitPort.write(theMessageMelleMel);
	telitPort.write("\r");
return	(catchTelitData(2000,true));				// 2000 milli time out with a quickCheck return data

}
//Sends AT command and stores reply with the catch data function
const char* const GSMbase::sendRecATCommand(const char* theMessageMelleMel){
	telitPort.write(theMessageMelleMel);
	telitPort.write("\r");
return	(catchTelitData());								//return data

}
//Sends AT command parses reply, makes string from the passed in strings
//EG. 
//input string: "the quick brown blah blah"
//_start string: "the"
//_end string: "blah"
//returns a pointer to "quick brown"
const char* const GSMbase::sendRecATCommandParse(const char* theMessageMelleMel, 
const char* _start,const char* _end){
	telitPort.write(theMessageMelleMel);
	telitPort.write("\r");
return (parseData(const_cast<char*>(catchTelitData()),_start,_end));			//return data

}
//Sends AT command splits data according to delimeter
//EG
//input string: "the, quick: brown, blah: blah,"
//_delimiters string ",:"
//field: 2
//returns pointer to "brown"
const char* const GSMbase::sendRecATCommandSplit(const char* theMessageMelleMel, 
const char* _delimiters, uint16_t _field){
	telitPort.write(theMessageMelleMel);
	telitPort.write("\r");
return	(parseSplit(const_cast<char*>(catchTelitData()),_delimiters,_field));		//return data

}
//////////////////////////////////////////////////////////////////////SEND FUNCS*




//////////////////////////////////////////////////////////////////////PARSE FUNCS
//finds the objectOfDesire string in theString if it is ! a NULL pointer
bool GSMbase::parseFind(const char* const theString
,const char* objectOfDesire){
	if (!theString) return 0;                       // If we get a NULL pointer bail	
return strstr(theString,objectOfDesire);
}


//Main function which retrives data from serial buffer and puts it into 
//fullData, which has class scope.
const char* const GSMbase::catchTelitData(uint32_t _timeout, 
bool quickCheck,uint16_t dataSize,uint32_t baudDelay){
	
	//memory allocation issue with realloc writes over itself and malloc fragments the SRAM with
	//too many calls I tried for a happy medium but unless i know your program flow this is the 
	//best I can do.
	bool badMessage = false;
	free(fullData);
	fullData=NULL;
	if (quickCheck) dataSize=20; //If it is just a quick check max size is "/n/nERROR/n/n/0"
	
	char* storeData = (char*) 
	malloc(sizeof(char) * (dataSize));
	
	if (storeData == NULL){		//if we get bad memory
		return 0;
		}

	// block wait for reply
	uint64_t startTimeGlobal = millis();		//9600/1000= 9.6bits per milli 6 milles ~6 bytes
	while (telitPort.available() < 1){ 		// smallest message "<CR><LF>OK<CR><LF>"    
		if((millis() - startTimeGlobal) > 
		_timeout){ return 0;} 			// timed out bad message
	}

	uint16_t dataPos=0;
	uint64_t startTimeBaud;
	//if no data in 60 milli (baudDelay default) sendings done
	while (1){
		if ((dataPos+1) >= dataSize){
			dataPos = 0;				//don't overflow buffer overwrite to clear buffer
			badMessage = true;			//after return NULL to mark bad message
		}
		storeData[dataPos++] = telitPort.read();	//Read out serial register
//DebugPort->write(storeData[dataPos-1]);
		startTimeBaud = millis();			
		while (telitPort.available() < 1){ 						
			if((millis() - startTimeBaud) > baudDelay){	//if no more data is coming
				storeData[dataPos]= '\0';		//Add NULL for a string
				//If it is a small amount of data 
				//we can afford to cut it down.		
				if (dataPos < 500){	//500 seems to be the threshold with nothing else running	
					free(fullData);
					fullData=NULL;
					
					fullData = (char*) 
					malloc(sizeof(char) * (dataPos+1));
					
					if (fullData == NULL) {
						return 0;
					}
					memcpy(fullData,storeData,dataPos+1);
					free(storeData);
					storeData=NULL;
				}else fullData=storeData; //ELSE we just copy over the whole thing
			goto doneReceive;
			}	
			
		} //No data in x time goto doneReceive, based on baud delay
	}


	
	doneReceive:
DebugPort->write(fullData);
	if(badMessage) return 0;
	if(quickCheck){
	if (parseFind(fullData, "\r\nOK\r\n")) return fullData; 	//return fullData
	else if (parseFind(fullData,"ERROR")) return 0;   		//return NULL
	else return 0;		
	}

return fullData;

}

//Sends AT command parses reply, makes string from the passed in strings
//EG. 
//input string: "the quick brown blah blah"
//_start string: "the"
//_end string: "blah"
//returns a pointer to "quick brown"
const char* const GSMbase::parseData(const char* const theString,const char* start,
const char* end){
	if (!theString) return 0;                       // If we get a NULL pointer bail	

	size_t startSize = strlen(start);		// get size of string 
	char* startP = strstr (theString,start);        // looks for string gives pointer including look
	if(!startP) return 0;				// If we didn't find begining of string *MEM LEAK IF TAKEN OUT*
	startP+=startSize;                              // offset (gets rid of delim)
	char* endP = strstr ((startP),end);             // starts at startP looks for END string
	if(!endP) return 0;				// We didn't find end 			*MEM LEAK IF TAKEN OUT*
	free(parsedData);          			// if it has been allocated, Free the memory
	parsedData=NULL;
	parsedData = (char*) malloc((size_t)(sizeof(char)*(endP-startP)+1));      // get memory 
	if (parsedData == NULL) {
	return 0;
	}            					// if we couldn't get mem
	
	uint16_t dataPos=0;
	while ( startP != endP ){			// grab between starP and endP
		parsedData[dataPos++]= *startP++;
	}
	parsedData[dataPos]= '\0';                      // NULL to make a proper string
return parsedData;					// gives back what it can. parsData has class scope.
}

//Splits data according to delimeter
//EG
//(THE FIELDS     0     1      2     3     4  )
//input string: "the, quick: brown, blah: blah,"
//_delimiters string ",:"
//field: 2
//returns pointer to "brown"
const char* const GSMbase::parseSplit(const char* const theString,
const char* delimiters,uint16_t field){
	if (!theString) return 0;  			// if not a NULL pointer 

	char * temp;					// you have to use a local scope char array,	
	char deadStr[strlen(theString)+1];		// or you get a bad memory leak.
	strcpy(deadStr, theString);			// don't change this unless you are careful.
	temp = strtok (deadStr,delimiters);		// split here first
	for(uint16_t i=0; i<field;++i){
	temp = strtok (NULL,delimiters); 		// use NULL to keep spliting same string
     	}

	free(parsedData);          			// if it has been allocated Free the memory
	parsedData=NULL;
	if(!temp)return 0;				// if we didn't find anything return NULL
	parsedData =  (char*) malloc(sizeof(char)* (strlen(temp)+1));   // get mem +'\0'
	if (parsedData == NULL) return 0;    				// If we get a NULL pointer 
	strcpy(parsedData,temp);                                    	// copy to parsedData, it has class scope


return parsedData;
}
//////////////////////////////////////////////////////////////////////PARSE FUNCS*
 

//////////////////////////////////////////////////////////////////////HARDWARE FUNCS
//This is the only function to be re written for arduino
//you would need to include the wiring.h and binary.h 
//in header file and #define _cplusplus
//#define OnOffPin PA0
bool GSMbase::turnOn(uint16_t OnOffPin){
	telitPort.write("AT\r");
	if(catchTelitData(1000,true,20,2000)) return 1;		// the power is already on

DebugPort->write("toggling power");	
	uint64_t startTime; 
	while(1){
		digitalWrite(OnOffPin, HIGH);
	 	startTime = millis();		
		while ((millis() - startTime) < 3000); 		// block 3 seconds
		digitalWrite(OnOffPin, LOW);
		
		startTime = millis();		
		while ((millis() - startTime) < 10000);		// block 10 seconds
		telitPort.write("AT\r");
		if(catchTelitData(1000,true,20,2000)) return 1;		// the power is already on
DebugPort->write("stuck in ON");
		}
return 1;							//should never get here
}


//Turns off Telit
bool GSMbase::turnOff(){
	uint64_t startTime;
	while(1){
		
		if(sendRecQuickATCommand("AT#SHDN")){			//send off command
			startTime = millis();
			while (((millis() - startTime) < 10000)); 	//block 10 second
			if(!sendRecQuickATCommand("AT"))return 1;
//DebugPort->write("stuck in OFF");
		}else if(!sendRecQuickATCommand("AT"))return 1;		//double check
	}		
//DebugPort->write("stuck in OFF outside");


return 1;
}
//Used to init Telit to right settings, code doesn't work if not used.
bool GSMbase::init(uint16_t _band){
//DebugPort->write("initalizing");

	telitPort.write("ATE0\r");		//set no echo
	catchTelitData(500,true,20,2000);	//needs a long baud delay to catch ok after echo	
	
	if(!sendRecQuickATCommand("ATV1"))return 0;		//set verbose mode
	if(!sendRecQuickATCommand("AT&K0"))return 0;		//set flow control off
	if(!sendRecQuickATCommand("AT+IPR=0"))return 0;		//set autoBaud (default not really needed)
	if(!sendRecQuickATCommand("AT+CMEE=2"))return 0;	//set exended error report
	switch(_band){
	case 0: if(!sendRecQuickATCommand("AT#BND=0"))return 0;	//0 - GSM 900MHz + DCS 1800MHz
	case 1: if(!sendRecQuickATCommand("AT#BND=1"))return 0;	//1 - GSM 900MHz + PCS 1900MHz
	case 2: if(!sendRecQuickATCommand("AT#BND=2"))return 0; //2 - GMS 850MHz + DCS 1800MHz 
	case 3: if(!sendRecQuickATCommand("AT#BND=3"))return 0; //3 - GMS 850MHz + PCS 1900MHz
	}

return 1;
}

//AT+TEMPMON=1 returns temp of mod in C
const char* const GSMbase::getTemperatureTEMPMON(){
//RETURNS: #TEMPMEAS: 0,25 
return  sendRecATCommandSplit("AT+TEMPMON=1",",",1);
}

//////////////////////////////////////////////////////////////////////HARDWARE FUNCS*


