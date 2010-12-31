#include <gsmGPRS.h>

//*******PIN DEFS***********
#define turnOnPin 40        //used to turn on off Telit
//**************************

//******FUNC DEFS***********
//connects to network and gets IP from gatway
bool setNetworkandGetIP(char* APN, char* userName, char* password);
//used to place a HTTP GET Request
bool placeGetRequest(char* hostName, char* resourcePath, uint16_t dataSize);       
// Used to send a  HTTP POST
bool placePostRequest(char* hostName, char* resourcePath, char* postString, uint16_t dataSize); 
//used to diconnect from network and free resources
void disconnectGiveBackIP(char* userName, char* password);
// Used to send commands directly to the Telit and trigger the above functions *used for testing*
void talkReply();                

//**************************

//********make a object of Gsm class********************
/*First you have to make the gsmSMS object, the arguments are in order
*GsmSMS (#1 Name of the serial port connected to GSM (your choice),#2 the address of millis function(just copy whats there) )*/
gsmGPRS  myGsmGPRS(Serial3,&millis,&Serial);        //gsmSMS TELIT SMS
//******************************************************

void setup(){
  Serial.begin(9600);                // used for debugging
  Serial3.begin(9600);               // used for talking to telit 
  Serial.println("hello");           // say hello to check serial line 
 
 /*If you are using a Telit module with a on/off pin you can
 *use the turnOn function. It is used with the supplied circuit
 *from www.GROUNDLAB/whatever*/
 
 //******************************
  myGsmGPRS.turnOn(turnOnPin);
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
  myGsmGPRS.init(3);               // set for New York NY
  Serial.println("done init");
 //****************************** 

}

bool isConnected = false;
void loop(){
   talkReply();  
}


//type '%' to send a text message ***REPLACE tel# AND MESSAGE FIRST!***
//type '~' to read any messages that have been sent to your phone and then delete them.
//type '^' to shutdown telit (if your using a telit)
//otherwise when you type you are sending commands straight to the telit like "AT"
void talkReply(){
  
        //listens for your typing
        if (Serial.available()){
          while (Serial.available()>0){
            char c = Serial.read();
                  switch (c){ 
                   case '!':  //get IP adress do this once
                    setNetworkandGetIP("wap.cingular","WAP@CINGULARGPRS.COM", "CINGULAR1");
                    break;
                    
                    case '%':  //place a get request
                                    //HOST                      //Resource Path        //received data size
                    placeGetRequest("www.johnhenryshammer.com", "/cTest/testGet.html", 500);
                    break;
                    
                    case '~':  //place Post request
                                     //HOST                      //resource path    //POST STRING          //received data size
                    placePostRequest("www.johnhenryshammer.com","/cTest/myPing.php","testPing=helloworld", 500);
                    break;
                    
                    case '^':  //disconnect from network and turn off telit
                    disconnectGiveBackIP("WAP@CINGULARGPRS.COM","CINGULAR1"); //username password for ATT 
                    isConnected = false;                                      //we are not connected             
                    myGsmGPRS.turnOff();
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


// ATT settings        "wap.cingular""WAP@CINGULARGPRS.COM","CINGULAR1"
bool setNetworkandGetIP(char* APN, char* userName, char* password){
//SET NETWORK CONNECTION
//SETS the context number associated with a PDP protocal "IP"/"PPP" and APN number.
        //context "2" now has these settings (NOTE don't use "0" it is reserved for SMS)
        //*********************************
        Serial.write("CGDCONT");
        myGsmGPRS.setApnCGDCONT("2","IP",APN);
        //*********************************
        //SETS the TCP/IP stack
        //after command socket conection ID 1 is now linked to context ID 2 data, with default timeouts TCP/IP
        //*********************************
        Serial.write("SCFG");
        myGsmGPRS.setTcpIpStackSCFG("1","2");
        //*********************************      
        //REGISTERS with the network, receives IP address and network resources.
        //connect the specified context ID ("2") to the network. 
        //1 gets network resources 0 disconnects from network and frees resources.
        //*****************************************************************
        Serial.write("SGACT");
        if(myGsmGPRS.setContextSGACT("2","1",userName,password)) return true;
        //*****************************************************************
  
 return false;  
}

//Test off my server, please change "www.johnhenryshammer.com", "/cTest/testGet.html" 500
bool placeGetRequest(char* hostName, char* resourcePath, uint16_t dataSize){
         char myGet[dataSize];                                         //used to hold GET reply
         if( myGsmGPRS.socketDialSD("1","0","80",hostName)){           //dial the socket
              //Constructs and sends a get request on open socket,
              //copy reply into your array.
              strcpy(myGet, myGsmGPRS.getHTTP(dataSize,hostName,resourcePath,"HTTP/1.1",true) );   
                         
              //Suspends listing to socket, socket can still 
              //receive data till a SH command is issued to 
              //shut the socket, SEE BELOW
              myGsmGPRS.suspendSocket();
         }else return false;                                            //if we didn't get CONNECT

        //OPTIONAL*********
        //AT#SI can be implemented to view the info of a socket
        Serial.write(myGsmGPRS.socketInfoSI("1") );                     //see the bytes transfered
        //*****************

        //AT#SH closes the socket connection, no data in or out
        myGsmGPRS.closeSocketSH("1");
  
        Serial.write("GET Request: \n");
        Serial.write(myGet);                                      //display GEt reply 
  
}

//"www.johnhenryshammer.com" "/cTest/myPing.php" "testPing=helloworld" 500
bool placePostRequest(char* hostName, char* resourcePath, char* postString, uint16_t dataSize){
          char myPostReply[dataSize];                                       //used to hold Post reply
          if( myGsmGPRS.socketDialSD("1","0","80",hostName)){
                //Johny 5 is the agent if you want to change it
                //copy reply into your array
                strcpy(myPostReply,myGsmGPRS.postHTTP(dataSize,hostName,resourcePath,
                "Johnny 5", "HTTP/1.1",true, postString));
               
                //Suspends listing to socket, socket can still 
                //receive data till a SH command is issued to 
                //shut the socket.
                        myGsmGPRS.suspendSocket();                          
                }else return false;                                         //if we didn't get CONNECT

          //OPTIONAL*********
          //AT#SI can be implemented to view the info of a socket
          Serial.write(myGsmGPRS.socketInfoSI("1") );                       //see the bytes transfered
          //*****************

          //AT#SH closes the socket connection, no data in or out
          myGsmGPRS.closeSocketSH("1");
        
          Serial.write("POST Reply: \n");
          Serial.write(myPostReply);                                        //display GEt reply       
}

//ATT NYC -> "WAP@CINGULARGPRS.COM" "CINGULAR1"
void disconnectGiveBackIP(char* userName, char* password){
        // Disconect and Give back the IP to the network
        myGsmGPRS.setContextSGACT("2","0",userName,password);  
}



