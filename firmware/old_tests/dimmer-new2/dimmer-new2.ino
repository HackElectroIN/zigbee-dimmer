#include  <TimerOne.h>          // Avaiable from <a href="http://www.arduino.cc/playground/Code/Timer1" rel="nofollow"> <a href="http://www.arduino.cc/playground/Code/Timer1" rel="nofollow"> http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1...</a>
volatile int i=0;               // Variable to use as a counter
volatile boolean zero_cross=0;  // Boolean to store a "switch" to tell us if we have crossed zero
int AC_pin = 6;                // Output to Opto Triac
unsigned char SYNC_PIN = 2; // Need pin 2 or 3 for interrupts

volatile int dimming = 67; 

void setup() {                                      // Begin setup
  pinMode(AC_pin, OUTPUT);                          // Set the Triac pin as output
  pinMode(SYNC_PIN, INPUT);
}

void zero_crosss_int()  
{
  int dimtime = 65*dimming;     //65?  
  delayMicroseconds(dimtime);    // Off cycle
  digitalWrite(AC_pin, HIGH);   // triac firing
  delayMicroseconds(8.33);         // triac On propogation delay
  digitalWrite(AC_pin, LOW);    // triac Off
}                              

long prevMills = 0;

void loop() {                        
 dimming = 114;
 if(digitalRead(SYNC_PIN)){
      zero_crosss_int();
 }
}
