#include <gsmSMS.h> //You just need to import the gsmSMS.h file if you are just doing SMS message

//*******PIN DEFS***********
#define turnOnPin 40        //used to turn on off Telit
//**************************

//******FUNC DEFS***********

bool readDeleteMessages();       // Used to read any received messages and then deletes them
bool checkNetworkSendMessage();  // Used to check if your on the network and then sends SMS
void talkReply();                // Used to send commands directly to the Telit and trigger the above functions

//**************************

//********make a object of Gsm class********************
/*First you have to make the gsmSMS object, the arguments are in order
*GsmSMS (#1 Name of the serial port connected to GSM (your choice),#2 the address of millis function(just copy whats there) )*/
gsmSMS  myGsmSMS(Serial3,&millis,&Serial);        //gsmSMS TELIT SMS
//******************************************************

void setup(){
  Serial.begin(9600);                // used for debugging
  Serial3.begin(9600);               // used for talking to telit 
  Serial.println("hello");           // say hello to check serial line 
 
 /*If you are using a Telit module with a on/off pin you can
 *use the turnOn function. It is used with the supplied circuit
 *from Wiki.Grounglab.cc
 
 //******************************
 //myGsmSMS.turnOn(turnOnPin);  //only needed if you have supporing hardware to turn on the Telit
 //******************************
 
 
 /*Then call the init function which sets the band to use.
 *More info found at:
 *http://en.wikipedia.org/wiki/GSM_frequency_bands
 *
 *0 - GSM 900MHz + DCS 1800MHz (eroupe,africa,some asia/south america)
 *1 - GSM 900MHz + PCS 1900MHz
 *2 - GMS 850MHz + DCS 1800MHz 
 *3 - GMS 850MHz + PCS 1900MHz  (north america,some south america)*/
 
 //******************************
  myGsmSMS.init(3);               // set for New York NY
  Serial.println("done intit");
 //****************************** 

}

void loop(){
  /*shows a basic example of how to send a text message
  *The first agrument is the number you wish to dial as a const char,
  *the second is the message you want to send also as a const char.
  *So if you wanted to send 1-800-POODLES a message you can say
  *EXAMPLE:*/
  
  //char myNumber[]={"18007663537"};               //The number you want to dial
  //char myMessage[]={"Hello Poodles"};            //The message
  //GsmSMS.sendNoSaveCMGS(myNumber,myMessage);     //Send it
  
  /* OR you can just say */ 
  
  //GsmSMS.sendNoSaveCMGS("18007663537","Hello Poodles");
  
  /*Use the talkReply function to get a good overview of the functionality (see below)*/
 
  talkReply();   
}

//type '%' to send a text message ***REPLACE tel# AND MESSAGE FIRST! in checkNetworkSendMessage()***
//type '~' to read any messages that have been sent to your phone and then delete them.
//type '^' to shutdown telit (if your using a telit)
//otherwise when you type you are sending commands striaght to the telit like "AT"
void talkReply(){
  
        //listens for your typing
        if (Serial.available()){
          while (Serial.available()>0){
            char c = Serial.read();
                  switch (c){
                    case '%': checkNetworkSendMessage();
                    break;
                    case '~': readDeleteMessages(); 
                    break;
                    case '^': myGsmSMS.turnOff();
                    break;
                    default:
                    Serial3.write(c);
                    break;
                    }
                  delay(50);    //to signal the end of TX
          }
          Serial3.println();    //send <CR>
        }
        //listen for reply, print it
        if (Serial3.available()>0){
            while (Serial3.available()>0){
                  Serial.write(Serial3.read());               
            }
       
        }
}


/*Below is an example of checking to see if you 
 *have received messages, if so read them
 *and delete the read messages.*/

bool readDeleteMessages(){
   char myMessages[500];                                            //used to hold messages
   if (strcpy(myMessages, myGsmSMS.readAllCMGL("REC UNREAD",500))){   //gets messages, 500 sets max size of message
     Serial.write(myMessages);                                      //display messages
    /*ERASE all read messages*/
    myGsmSMS.deletMessagesCMGD("1,2");
    return 1;
   }
return 0;
}

/*A Example of how to send a message, it is a good 
idea to check network before completeing a send.*/

bool checkNetworkSendMessage(){
  if (myGsmSMS.checkCREG()){              //Check if your registered with network, Returns true if you are.
      if(myGsmSMS.checkCSQ() > 15){         //Check the signal quality 20 for GPRS, 15 is ok for SMS
        /* If everything is fine go ahaed with the send
        ****Replace tel # and message FIRST!**********/
        if (myGsmSMS.sendNoSaveCMGS("18007663537","Hello Poodles")) return true;        
      }
  }
  return false;
}

/*Examples of more functions in the SMS class*/

void gsmSMSTester(){
//CHECK CMGD gives a numerical list of all messages which are stored seperated by ','
      Serial.write( myGsmSMS.checkCMGDList() );
//ERASE CMGD (with arg) erases message at id(EG 1,2,3,4) arg
//      GsmSMS.deletMessagesCMGD("2");
//READ CMGR read message with specified ID     
//      GsmSMS.readMessageCMGR("4");
//SAVE CMGW saves a message to memory and CMSS sends saved message.
//      GsmSMS.sendSavedMessageCMSS(GsmSMS.saveMessageCMGW("18007663537","I remember!"));        
//CHECK CPMS gets number of messages or total space availible in sim memory selected by arg.
//      Serial2.write( GsmSMS.getNumMesInMemCPMS(1) );

}

