// This demo does web requests to a fixed IP address, using a fixed gateway.
// 2010-11-27 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

#include <EtherCard.h>

#define REQUEST_RATE 5000 // milliseconds

// ethernet interface mac address
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
// ethernet interface ip address
static byte myip[] = { 192,168,3,203 }; // nanode ip
// gateway ip address
static byte gwip[] = { 192,168,3,1 }; // gateway (router) ip
// remote website ip address and port
static byte hisip[] = { 192,168,3,1 }; // ip of the site you request
// remote website name
char website[] PROGMEM = "192.168.3.1"; // the hostname of the url of the site you request

// ~guidocalvano/
static BufferFiller bfill;  // used as cursor while filling the buffer
byte Ethernet::buffer[500];   // a very small tcp/ip buffer is enough here
static long timer;

// 272 is for monochrome 17 * 16 screen
// 816 is for colour 17 * 16 screen

#define TOTAL_LENGTH 272  // width * height * #of colours
byte pwmVal[  TOTAL_LENGTH ] ;

unsigned long startFrame       = millis() ; // all units are milliseconds
unsigned long frameDuration    = 0 ;

#define BRIGHTNESS_LEVEL_COUNT 7 


byte dataPin    = 0 ;

byte clockPin   = 1 ;
byte clockState = LOW ;

byte latchPin   = 2 ;

void outputPwm()
  {
   int newTime   = millis()             ;
   frameDuration = startFrame - newTime ;
   startFrame    = newTime              ;
   
   byte shiftChunk = 0 ;
   byte bitPhase = 0 ;
   for( int brightnessLevel = 0 ; brightnessLevel < BRIGHTNESS_LEVEL_COUNT ; brightnessLevel++ )
      {
       digitalWrite( latchPin, LOW ) ;
        
       byte brightnessTime = ( millis() % frameDuration ) / 256;
       

       for( int i = 0 ; i < TOTAL_LENGTH ; i++ )
         {
          bitPhase = i % 8 ; 
           
          shiftChunk &=  ~( ( brightnessTime < pwmVal[ i ] ? 0 : 1 ) << bitPhase ) ;
          if( bitPhase == 7  )
            shiftOut( dataPin, clockPin, MSBFIRST, shiftChunk ) ;
          /*
          digitalWrite( dataPin, brightnessTime < pwmVal[ i ] ? HIGH : LOW ) ;
          
                // I don't know yet how exactly the clock pin should be set, assuming worst case
          clockState = !clockState ;
          digitalWrite( clockPin, clockState ) ; 
          digitalWrite( clockPin, HIGH ) ;*/
         } 
             
           // I don't know yet how exactly the latch pin should be set, assuming worst case
       digitalWrite( latchPin, HIGH ) ;
      } 
  } 

void setupPwmShiftRegisterInterface()
  {
   for( int i = 0 ; i < TOTAL_LENGTH ; i++ )
     { 
      pwmVal[ i ] = 0 ;
     }
    
   pinMode( dataPin,  OUTPUT ) ;      
   pinMode( clockPin, OUTPUT ) ;
   pinMode( latchPin, OUTPUT ) ;
  } 


char page[] PROGMEM = "HTTP/1.0 200 OK\r\n"
"Content-Type: text/html\r\n"
"\r\n"
"<html>"
  "<head><title>"
    "No worries"
  "</title></head>"
  "<body>"
    "<h3>This service is currently fine</h3>"
    "<p><em>"
      "The main server is currently on-line.<br />"
    "</em></p>"
  "</body>"
"</html>"
;

byte* nextLine( byte* bufferOffset )
  {
   while( *bufferOffset != '\n' )
     bufferOffset++ ;
   
   bufferOffset++ ;
   
   return bufferOffset ; 
  } ;


void processUpdate()
  {
   word len = ether.packetReceive() ;
   word pos = ether.packetLoop(len) ;

   if( pos )
      {
        
       Serial.println( "start of packet\n" );
       bfill = ether.tcpOffset();
       byte* dataOffset = (byte*) Ethernet::buffer + pos ;
    
       Serial.println( (char*) dataOffset ) ;
    
       byte* next = nextLine( nextLine( nextLine( nextLine( nextLine( nextLine( dataOffset ) ) ) ) ) ) ;
       
       Serial.println( "payload" ) ;
       Serial.println( (char*) next ) ;
    
       word offset = *next ;
       next++ ;
       word length = *next ;
       next++ ;
    
       for( int i = 0 ; i < length ; i++ )
          {
           pwmVal[ offset + i ] =  *next ;
           next++ ;
          } 

       Serial.println( "end of packet\n" );
    
       memcpy_P(ether.tcpOffset(), page, sizeof page);
       ether.httpServerReply(sizeof page - 1);
     }    
    } ;

void setupEtherCard()
    {
     if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) 
         Serial.println( "Failed to access Ethernet controller");

     ether.staticSetup(myip, gwip);

     ether.copyIp(ether.hisip, hisip);
     ether.printIp("Server: ", ether.hisip);

     while (ether.clientWaitingGw())
        ether.packetLoop(ether.packetReceive());
    
     Serial.println("Gateway found");        
    }




void setup () {
  Serial.begin(57600);
  Serial.println("\nSTART OF EXECUTION");

  setupEtherCard() ;
  
  setupPwmShiftRegisterInterface() ;
  

}

void loop () 
  {

   processUpdate() ;
  
   int s = millis() ;
   for( int i = 0 ; i < 5 ; i++ )
     outputPwm() ;
  
   int d = millis() - s ;
   Serial.println( "TIME" ) ;
      Serial.println( d/5 ) ;
  }
