/*             *
 *  Ludlum 2000 Rate Meter Telnet Server*



   
 // Base on the Original work of Steve Lentz
// stlentz[at]gmail[dot]com   *

 Quick Start Instructions:
 1) Set Ethernet address in code below.
 2) Compile and upload sketch.
 3) Connect Arduino to Ethernet.
    Make sure link light is on.
 4) Telnet to Arduino's IP.
 5) On some Telnet clients, hit return to wake up connection.
 6) When connected, type ? <cr> for help.
 7) Try a simple command such as 'ar'.

Other notes
Tested on Duemilanove with Ethernet Shield.
Should work on compatible boards.
Tested with Win XP, OS X, and Debian Telnet clients.
Compiles to about 9 KB, can be made smaller by removing
  unneeded commands, help message, etc.
I am an entirely self-taught C programmer; if you
  don't like my code, too bad ;-).  */

//Ludlum 2000 Rate Meter Telnet Server Available command




//http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1278686415

// Ethernet parameters
#include <SPI.h>
#include <Ethernet.h>
//#include <UIPEthernet.h>
#include "Timer.h"
Timer t;

byte mac[] =     { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//byte ip[]  =     {192, 168, 2, 100 };
//byte gateway[] = {192, 168, 2, 1 };
//byte subnet[]  = {255, 255, 255, 0 };

// Other global variables
#define textBuffSize 15 //length of longest command string plus two spaces for CR + LF
char textBuff[textBuffSize]; //someplace to put received text
int charsReceived = 0;

boolean connectFlag = false; //we'll use a flag separate from client.connected
         
         
         
         
         //so we can recognize when a new connection has been created
unsigned long timeOfLastActivity; //time in milliseconds of last activity
unsigned long allowedConnectTime = 500000; //five minutes
unsigned long parseCountTime1 = 0; // actual decimal value which represents the amount of time the ludlum will count, number displayed on the net is derived from this integer (Must Be declare as unsigned long variable)
 unsigned long parseCountTime2 = 0; //Variable to hold the value of parseCountTime1 multiply by 1000 Milisecond to second conversion (Must Be declare as unsigned long variable)
// The Function of eache Input is describe 
// The pin number on the Data Connector at the back of the ludlum 2000 is indicated
// the color of the electrical wire connected to th at Connector at the back of the ludlum 2000 is indicated
// the following document give more information: Ludlum 2000 on Atmega 1284.docx

const int StartCount    = 28;     //Start the Ludlum 2000 Count. Pin 6 on The Ludlum 2000 Ratemeter (Count) : 
                                  //(Output Pin: 36 From The Microcontroller) (Input To The Ludlum 2000 Ratemeter)       
                                  //Association Variable: StartCount to Pin: 33 From The Microcontroller
const int StopCount     = 13;     //Stop the Ludlum 2000 Count. Pin 7 on The Ludlum 2000 Ratemeter (Hold) : 
                                  //(Output Pin:19 From The Microcontroller) (Input To The Ludlum 2000 Ratemeter)  
                                  // Association Variable: StopCount  to Pin: 34 From The Microcontroller   


const int countcompletepin = 27; //Count complet Pin



#define INTERRUPT_INPUT 1

const int loadprinterPin = 30;      // Pin 3 (load Printer) goes Low Signaling Completion of Data Transfert
                                    //(Input Pin:37 From The Microcontroller)(Output To The Ludlum 2000 Ratemeter)
                                    // Association Variable: loadprinterPin  to Pin: 34 From The Microcontroller

const byte printerready = 29;       // Printer Ready Pin



//BDC Data Input

#define q1 14                         //BDC Data Output From the Ludlum 2000 Ratemeter (Bit 1 (B1)) : Pin 12 on the Ludlum 2000 Ratemeter
                                     //Input Pin:18 From The Microcontroller)(Output To The Ludlum 2000 Ratemeter). 
                                     //Association Variable: q1  to Pin: 18 From The Microcontroller    ( PCB Start Buttons 1)                              
#define q2 15                         //BDC Data Output From the Ludlum 2000 Ratemeter (Bit 2 (B2)) : Pin 13 on the Ludlum 2000 Ratemeter
                                    //Input Pin:19 From The Microcontroller)(Output To The Ludlum 2000 Ratemeter)
                                    //Association Variable: q2  to Pin: 19 From The Microcontroller ( PCB Start Buttons 2)   


#define q4 25                       //BDC Data Output From the Ludlum 2000 Ratemeter (Bit 3(B4)) : Pin 14 on the Ludlum 2000 Ratemeter
                                   //Input Pin:20 From The Microcontroller)(Output To The Ludlum 2000 Ratemeter)
                                   //Association Variable: q4  to Pin: 20 From The Microcontroller (PCB Stop Buttons)

                                   
#define q8 26                      //BDC Data Output From the Ludlum 2000 Ratemeter (Bit 4(B8)) : Pin 15 on the Ludlum 2000 Ratemeter
                                   //Input Pin:21 From The Microcontroller)(Output To The Ludlum 2000 Ratemeter)    
                                   //Association Variable: q8  to Pin: 21 From The Microcontroller (PCB Capsule Request Reset (Clear))

//Variable Declaration 
byte countcomplete = 0; //Variable used as a flag for when the countcomplete pulse was detected

unsigned long digits[6] = {0,0,0,0,0,0};//Array of values stored to represent the displayed digital values coming for the ludlum counter .... digits[0] is the MSbit and digits[5] is the LSbit (Must Be declare as unsigned long variable)
volatile int i = 0 ; //variable used to choose in which digits interger to store the values
unsigned long convertdigits = 0; //flag used to allow function "converstions_of_digits" to be used (Must Be declare as unsigned long variable)

unsigned long totalCount=0;//represents the whole number (the displayed number of counts on the ludlum counter) derived from combination of the digits gathered from ludlum counter data stream. (Must Be declare as unsigned long variable)



//Variable Declaration 

//For controlling The Srat and Stop Sequence of The Ludlum 2000 Ratemeter



boolean receiving = false; //To keep track of whether we are getting data.
int ModeSS = 0; //The SS Mode Start and then Stop the Ludlum 200 Counter when the SS Command is Send over the Telnet Console
int ModeSO = 0; //The SO Mode Start and then Stop the Ludlum 200 Counter when the Count Time is Over. Display the Result on The Client


EthernetServer server(23); // Telnet listens on port 23
EthernetClient client; // Client needs to have global scope so it can be called
       // from functions outside of loop, but we don't know
       // what client is yet, so creating an empty object

bool SOmodeSet = 0; // Set a flag to diasbled the SO Function When So is started


void setup()
{

pinMode(StopCount, OUTPUT);      //Stop The Count on the Ludlum 2000 Ratemeter unit. Set The Pin 34 on The Microcontroller as an Output
  pinMode(StartCount, OUTPUT);     //Start The Count on the Ludlum 2000 unit.   


pinMode(loadprinterPin, INPUT);   // Pin 3 (load Printer) goes Low Signaling Completion of Data Transfert
                                    // Set The Pin 37 on The Microcontroller as an Output
  
  pinMode (printerready, OUTPUT);   // Configuring printerready pin as an output                             
                                    
                                    
      // Initialize the Following pin as an input:
      //BDC Data Input
 pinMode(q1, INPUT);    //BDC Data Output From the Ludlum 2000 Ratemeter (Bit 1 (B1)) : Pin 12 on the Ludlum 2000 Ratemeter
                        //Input Pin:18 From The Microcontroller)(Output To The Ludlum 2000 Ratemeter).
 
 pinMode(q2, INPUT);   //BDC Data Output From the Ludlum 2000 Ratemeter (Bit 2 (B2)) : Pin 13 on the Ludlum 2000 Ratemeter
                       //Input Pin:19 From The Microcontroller)(Output To The Ludlum 2000 Ratemeter)
 pinMode(q4, INPUT);  //BDC Data Output From the Ludlum 2000 Ratemeter (Bit 3(B4)) : Pin 14 on the Ludlum 2000 Ratemeter
                      //Input Pin:20 From The Microcontroller)(Output To The Ludlum 2000 Ratemeter)
 pinMode(q8, INPUT); //BDC Data Output From the Ludlum 2000 Ratemeter (Bit 4(B8)) : Pin 15 on the Ludlum 2000 Ratemeter
                     //Input Pin:21 From The Microcontroller)(Output To The Ludlum 2000 Ratemeter)
  
 
 digitalWrite(printerready, HIGH); //keeping printerready output high as default
  // Printer Clock INTERRUPT





//attachInterrupt (INTERRUPT_INPUT - 1, PrinterClockINT, FALLING);  

digitalWrite(INTERRUPT_INPUT, HIGH);    // internal pull-up resistor
  attachInterrupt(INTERRUPT_INPUT - 0,  // attach interrupt handler*/ The Interrupt 1 is connected to pin: on the microcontroller 
                 PrinterClockINT,       // call this function when the Interrupt 1 is trigger by the Falling edge of the Printer Clock Signal
                  FALLING);


  Serial.begin(9600);
  Serial.println("Ludlum 2000 Ratemeter Telnet Server");
 // Ethernet.begin(mac, ip, gateway, subnet);
  
  
  Ethernet.begin(mac);

  Serial.print("localIP: ");
 Serial.println(Ethernet.localIP());
 Serial.print("subnetMask: ");
 Serial.println(Ethernet.subnetMask());
 Serial.print("gatewayIP: ");
 Serial.println(Ethernet.gatewayIP());
 Serial.print("dnsServerIP: ");
 Serial.println(Ethernet.dnsServerIP());
 
  server.begin();//begin listening for incoming connectings
}


void PrinterClockINT ()
//Printerclock pin inturrupt. Each clock falling edge (which is when the inturrupt is triggered) represents the validation of the incoming data bits. In other words, the 
//data bits are valid and can be read at each falling edge of the printerclock pulses.
//6 Digits will be sent through the data stream pins. There a 4 data pins (q1,q2,q4,q8) which represents the 4 bits for 1 digit. There are 6 digits in total. 
//Therefore, we will receive 4 bits at each clock pulse, 6 times. The first incoming 4 bits from the first clock pulse will represent the upper digit. The last incoming 
//4 bits from the 6th clock pulse will represent lower digit of the 6 digit number. 
{
digits[i]=0;// zeroing the designated digit array byte. "digit[]" represents the decimal number from the designated digits. Example: the first digit's 4 bits = 1001.That means digit[0]=9
if (digitalRead(q1)==HIGH) { digits[i]+=1; }//LSbit of the digit
if (digitalRead(q2)==HIGH) { digits[i]+=2; }
if (digitalRead(q4)==HIGH) { digits[i]+=4; }
if (digitalRead(q8)==HIGH) { digits[i]+=8; }//MSbit of the digit
i++;//increment the i variable so that at the next inturrupt the integer digit array used will be digit[n+1]
  if(i==6){//if we are the 6th clock inturrupt, we must exit the inturrupt and combine the digits gathered to a whole number
  i=0;
  convertdigits = 1;//flag to indicate to go ahead with converting the digits into a whole number
  }

}  // end of printerclock inturrupt



void loop()
{
  t.update();
  // look to see if a new connection is created,
  // print welcome message, set connected flag
  if (server.available() && !connectFlag) {  //evaluate if there is a client available, if no client is availalable it will return a false to the if statement
    connectFlag = 1;
    client = server.available();//get Client object
    Serial.println("Ludlum 2000 Ratemeter Telnet Server");
    client.println("Ludlum 2000 Ratemeter Telnet Server");
    client.println("All Commands Must be UPPERCASE");
    client.println("? for help");
    printPrompt();
  }

  // check to see if text received
  if (client.connected() && client.available()) getReceivedText();//if client is connected client.connected() will be true, else its false. client.available will return the number of bytes avaiable to be read (therefore if there at least 1 byte the the function a true)

  // check to see if connection has timed out
//  if(connectFlag) checkConnectionTimeout();

LudlumDataRemove();
if(digitalRead(countcompletepin)== HIGH)//statement if true if the countcomplete pin is high (detecting the rising edge of the count complete pulse)
  {
     while(digitalRead(countcompletepin)== HIGH);//waiting for the pin state to go low (falling edge of the pulse)
     countcomplete = 1;//set the countcomplete flag as true so as to send a printerready pulse 
  } 
  // code to do other things in loop would go here



}

//////////////////////////////////////////////////////////DIRECT COMMUNICATION WITH LUDLUM SECTION////////////////////
void printPrompt()
{
  timeOfLastActivity = millis();
  client.flush(); //wait until all outgoing characters in buffer have been sent
  charsReceived = 0; //count of characters received
  client.print("\n>");
}




void getReceivedText()
{
  char c;
  int charsWaiting;

  // copy waiting characters into textBuff
  //until textBuff full, CR received, or no more characters
  charsWaiting = client.available();//client.available will return the number of bytes avaiable to be read  
  do {
    c = client.read();//read incoming byte
    textBuff[charsReceived] = c;//store receive byte
    charsReceived++;//since the will be multiple bytes that will be read the charsReceived int will act as a guide to know in which int from the textBuff array the byte has to be stored. Increments each time a byte if received.
    charsWaiting--;//each time we read a byte we will decrement from the count of incoming bytes received by the clinent.available function. Basically charsWaiting represents the amount of bytes left to be read.
  }
  while(charsReceived <= textBuffSize && c != 0x0d && charsWaiting > 0);//stay in loop until all bytes have been read according to the count from charsWating. Also stay in loop as long as there is no more then 15 byte that have been read

  //if CR found go look at received text and execute command
  if(c == 0x0d) {
    parseReceivedText();//evaluate the first byte receive to know what to do next
    // after completing command, print a new prompt
    printPrompt();
  }

  // if textBuff full without reaching a CR, print an error message
  if(charsReceived >= textBuffSize) {
    client.println();//SEND EMPTY LINE
    printErrorMessage();
    printPrompt();
  }
  // if textBuff not full and no CR, do nothing else;
  // go back to loop until more characters are received

}


void parseReceivedText()
{
  // look at first character and decide what to do
  if(textBuff[0] == 'S' && textBuff[1] == 'C'&& SOmodeSet == true)
  
    {
client.println("Please Wait Until Count Is Finished...");
}
      else if (textBuff[0] == 'S' && textBuff[1] == 'C'&& SOmodeSet == false)
      {
    SetCountTime();
    }
  
  else if(textBuff[0] == 'S' && textBuff[1] == 'O'&& SOmodeSet == true)
  
      {
client.println("Please Wait Until Count Is Finished...");
    }
  
  
  else if(textBuff[0] == 'S' && textBuff[1] == 'O'&& SOmodeSet == false)
  {

    SOMode();
    
  }



  
  else if(textBuff[0] == 'C'&& SOmodeSet == 1)
  
   {
client.println("Please Wait Until Count Is Finished...");
    }
  else if(textBuff[0] == 'C'&& SOmodeSet == 0)
  
  
  {
   checkCloseConnection();
  }

  else if(textBuff[0] == '?'&& SOmodeSet == 1)
{
client.println("Please Wait Until Count Is Finished...");
    }

else if(textBuff[0] == '?'&& SOmodeSet == 0)
{
printHelpMessage();
  }

 

}



//////////////////////////////////////////////////////////////////////////////////////////////////START-STOP COUNT///////////////////////////////////////////////



void SOMode()
  // if we got here, textBuff[0] = 'S' and textBuff[1] = 'O'
  // SO Mode: Start A Scaler Count When the Count Time Is Elapsed Stop the Count.
{

// If the Count Time is not set by the end user . The Telnet Server Print The following.
// The SO Mode Cant Work if the Count Time is not set by the end user
if (parseCountTime1 == 0)

{
     Serial.println("Set Count Time"); 
     client.println("Set Count Time"); 
            
          }

          else{

 
//The "SO Mode" Start a count for the (parseCountTime1)Set By The End-User , Then Stop the Count.
// Print The Following To The Telnet Console And Serial Console ("SO Mode: Start/Stop A Scaler Count. ")

    SOmodeSet= true;
    Serial.println ("Start Scaler Count SO Mode");
    digitalWrite(StartCount, HIGH);        //Set The "StartCount" Pin34 to HIGH 
    delay(2);                            //Delay of 2 Miliseconde
    digitalWrite(StartCount, LOW);        //Set The "StartCount" Pin34 to LOW
                              

 t.after(parseCountTime2,voidstop);
   
                     
 }
 }

  void voidstop ()
  {
    
    Serial.println ("Stop Scaler Count");
    digitalWrite(StopCount, HIGH);        //Set The "StopCount" Pin34 to HIGH
    while(digitalRead(countcompletepin)== LOW);//wait for countcomplete pulse
    if(digitalRead(countcompletepin)== HIGH)//statement is true if the countcomplete pin is set high (rising edge of the pulse)
  {
     while(digitalRead(countcompletepin)== HIGH);//wait for countcomplete pulse to go low so as to trigger printerready pulse at the falling edge
     countcomplete = 1;//countcomplete flag to inform that a countcomplete pulse was received
  }   
    delay(2);                           //Delay of 2 Miliseconde
    digitalWrite(StopCount, LOW);        //Set The "StartCount" Pin34 to LOW
    
SOmodeSet= false;

}




//////////////////////////////////////////////////////////////////////////////////////////////////SET COUNT TIME///////////////////////////////////////////////




void SetCountTime()

  // if we got here, textBuff[0] = "S" and textBuff[1] = "C"
// SC: SC Command. Set count Time 
// Count time is adjustable from 1 to 65535. The "Minutes" button shall be set to "EXT" on the Ludlum 2000 Rate Meter


  // If we got here, textBuff[0] = 'S' and textBuff[1] = 'C'
  // If we got here textBuff[2] (The "=" caracter is found on the textBuff[2],   Then call the function: parseCountTime() 
  //The Function: parseCountTime(), This function parse the countime input by the user to a variable called:  parseCountTime1
{
   if (textBuff[2] == '=') 
   {
parseCountTime();
  }
  else
  {
  printErrorMessage();
  }
  
  }
  

int parseCountTime()
{

  client.println ("Set Count Time");
  Serial.println ("Set Count Time");
  
  if (parseCountTime1 > 0)

{

parseCountTime1 =0;
  
}
  
  int textPosition = 3;  //start at textBuff[3] 
  int digit;
  do {
    digit = parseDigit(textBuff[textPosition]); //look for a digit in textBuff
   if (digit >= 0 && digit <=9) {      //if digit found
  parseCountTime1 = parseCountTime1 * 10 + digit;     //shift previous result and add new digit
    }

    textPosition++;            //go to the next position in textBuff
  }
  //if not at end of textBuff and not found a CR and not had an error, keep going
 while(textPosition < 9 && textBuff[textPosition] != 0x0d && parseCountTime);//evaluate every digits (7 of them max) afterwards exit loop




client.println("Count Time Set TO"); //Print the Following on telnet console: ("Count Time Set TO")
Serial.println("Count Time Set TO"); //Print the Following on Serial console: ("Count Time Set TO")
client.println(parseCountTime1); //Print the Following on telnet console: The countain of variable ("parseCountTime1")
Serial.println(parseCountTime1);//Print the Following on Serial Console : The countain of variable ("parseCountTime1")
client.println("Second"); //Print the Following on telnet console: ("Second")
Serial.println("Second"); //Print the Following on Serial console: ("Second")

  parseCountTime2 = parseCountTime1*1000;
  Serial.println(parseCountTime2);
}

    

//////////////////////////////////////////////////////////////////////////////////////////////////SET COUNT TIME///////////////////////////////////////////////









int parseDigit(char c)//convert the char number to an actual digital integer number, then return digital value to the parseDigit int
{
  int digit = -1;
  digit = (int) c - 0x30; // subtracting 0x30 from ASCII code gives value (PURPUSE IS TO RECEIVE THE DIGITAL VALUE OF THE CHARACTER, SINCE THE NUMBERS THE CLIENT SENT WERE IN ASCII)
                          //EXAMPLE: CLIENT SENT THE CHARACTER 2 WHICH IS 0X32 IN ASCII. WE SUBSTRACT THE 0X3 TO BE LEFT WITH ONLY THE DIGITAL VALUE 2
  if(digit < 0 || digit > 9) digit = -1;
  return digit;
}


void printErrorMessage()
{
  client.println("Unrecognized command.  ? for help.");
  Serial.println("Unrecognized command.  ? for help.");
}


void checkCloseConnection()
  // if we got here, textBuff[0] = 'C', check the next two
  // characters to make sure the command is valid
{
  if (textBuff[1] == 'L' && textBuff[2] == 0x0d)
    closeConnection();
  else
    printErrorMessage();
}


void closeConnection()
{
  client.println("\nBye.\n");
  Serial.println("\nBye.\n");
  client.stop();
  connectFlag = 0;
}


void printHelpMessage()
{
  client.println("\nExamples of supported commands:\n");
  client.println("All Commands Must be UPPERCASE");
  client.println("  SO     -Start A Scaler Count When The Count Time Is Elapsed Stop The Scaler Count");
  client.println("Automatically Sent To The Computer");
  client.println("  SC=20  -Set Count Time:   Set Count Time to 20 Second");
  client.println("          Count time From 1 Seconds to 9999 Seconds");
  client.println("  RC     -Return the Count Display On The Front Of The Ludlum Unit");
  client.println("  CL     -Close Connection");
  client.println("  ?      -Print This Help Message");
  
}

//////////////////////////////////////////////////////////TELNET SERVER SECTION////////////////////


//////////////////////////////////////////////////////////DIRECT COMMUNICATION WITH LUDLUM SECTION////////////////////

void LudlumDataRemove (void)
{


 if(countcomplete==1) // if statement is true when the countcomplete flag from the the countcompleteINT is true
 {
  countcomplete = 0; // bring variable back to a value of as default
  i=0;
   digitalWrite (printerready, LOW);
delay (3); // printerready pulse of 3ms
    digitalWrite (printerready, HIGH);  
 }

  if(convertdigits == 1)//statemenmt is true if the convertdigits flag set at the printclock inturrupt is true
{
 conversion_of_digits();// function to convert the digits received and combine them into a while number
 convertdigits = 0;
 }

}

void conversion_of_digits(void)
//Convert the 6 digits received from the ludlum and combine them into a readable whole number to display it. DEC Digit Multiplication 
//According to the position on the LED Display on the Rate Meter

{
totalCount=0;
totalCount+=(digits[0]*100000);//Most significant digit
totalCount+=(digits[1]*10000);
totalCount+=(digits[2]*1000);
totalCount+=(digits[3]*100);
totalCount+=(digits[4]*10);
totalCount+=digits[5];//Least significant digit

Serial.println ("Return the Count Display On The Front Of The Ludlum Unit");
Serial.println(totalCount, DEC); // Print of Serial Console the number on the LED Display on the Rate Meter
client.println (totalCount);
}

//////////////////////////////////////////////////////////DIRECT COMMUNICATION WITH LUDLUM SECTION////////////////////

