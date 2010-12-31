/*AVR SOURCE FILES FOR GSM,SERIAL FUNCTIONALITY
*                                                                     1%*
* Copyright (C) 2010  Justin Downs of GRounND LAB   			*
* www.GroundLab.cc                                  			*
*                                                                       *
* This program is free software: you can redistribute it and/or modify  *
* it under the terms of the GNU General Public License as published by  *
* the Free Software Foundation, either version 3 of the License, or     *
* at your option) any later version.                                    *
*                                                                       *
* This program is distributed in the hope that it will be useful,       *
* but WITHOUT ANY WARRANTY; without even the implied warranty of        *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
* GNU General Public License for more details.                          *
*                                                                       *
* You should have received a copy of the GNU General Public License     *
* with this program.  If not, see <http://www.gnu.org/licenses/>.       *
+***********************************************************************/

#ifndef GSMSMS
#define GSMSMS
#include "gsmbase.h"

class gsmSMS : virtual public GSMbase
{

				//**FULL FUNC DESCRIPTION IN SOURCE**\\

	private:
	char* messageList;
	public:
	gsmSMS(Serial&, uint32_t(*FP)(), Serial* = NULL);
	inline const char* const getMessageList(){return messageList;}
	////////////////////////////INIT FUNC
	virtual bool init(uint16_t);					//INITS, CALLS BASE INIT
	bool smsInit();							//called in init, for derived class
	////////////////////////////SEND FUNC
	bool sendNoSaveCMGS(const char*,const char*);			//Send message no save
	const char* const saveMessageCMGW(const char*,const char*);	//Send to memory
	bool sendSavedMessageCMSS(const char * const);			//Send from memory
	///////////////////////////LIST MESSAGE FUNC
	const char* const getNumMesInMemCPMS(uint16_t);			//Gets # mes 
	const char* const checkCMGDList();				//Gets a # list of all avail mes
	///////////////////////////READ FUNCS
	const char* const readMessageCMGR(const char* const);		//Reads specified mes
	const char* const readAllCMGL(const char* const, uint16_t _dataSize=300);	//Reads all mes of type
	///////////////////////////DELETE FUNCS
	bool deletMessagesCMGD(const char* const);			//Deletes mes 
	
	////****OPTIONAL SEE BELOW****************
	//const char* const checkCSCA();
	//bool setCSCA(const char* theNumber);
	//****************************************

};

#endif
