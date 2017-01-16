#include <AStar32U4.h>
#include  <TimerOne.h> 

#include "RunningAverage.h"
RunningAverage smoother(15);

// pins
unsigned char AC_LOAD_PIN = 6;    // Output to Opto Triac pin
unsigned char SYNC_PIN = 2; // Need pin 2 or 3 for interrupts

unsigned char PWM_IN_PIN = 3; // Need pin 2 or 3 for interrupts
unsigned char ONOFF_IN_PIN = 8;

// Dimmer stuff
unsigned char SHUTDOWN_VALUE = 60; // when to turn off the bulb (because of flickering..)
volatile boolean zero_cross=0;  // Boolean to store a "switch" to tell us if we have crossed zero
int freqStep = 65;    // This is the delay-per-brightness step in microseconds.
                      // For 60 Hz it should be 65

volatile int i=0;               // Variable to use as a counter
unsigned char dimming = 114;  // Dimming level (0-100)

//PWM reading stuff
volatile boolean interrupted = false; // the dimmer interrupt sometimes messes up the PWM readings.
volatile int pwm_value = 0;
volatile unsigned long prev_time = 0;

// variables used in the loop
int on_off_value = 0;
int temp_pwm_value;
int prev_temp_pwm_value = -1;
int prev_dimming = -1;
unsigned char temp_dimming;

void setup() {
  Serial.begin(9600);
  
  // Set AC Load pin as output
  pinMode(AC_LOAD_PIN, OUTPUT);
  
  // Dimmer sync. Pin 2 on Uno, Pin 2 on attiny
  //pinMode(SYNC_PIN, INPUT);  
  attachInterrupt(digitalPinToInterrupt(SYNC_PIN), zero_cross_detect, RISING);

  // PIN for reading PWM from CREE bulb circuit
  //pinMode(PWM_IN_PIN, INPUT);  
  attachInterrupt(digitalPinToInterrupt(PWM_IN_PIN), falling, FALLING);

  pinMode(ONOFF_IN_PIN, INPUT); 

  // Use the TimerOne Library to attach an interrupt
  // to the function we use to check to see if it is 
  // the right time to fire the triac.  This function 
  // will now run every freqStep in microseconds.     
  Timer1.initialize(freqStep);                      // Initialize TimerOne library for the freq we need
  Timer1.attachInterrupt(dim_check, freqStep);      

}

// dimming = 10 is brightest
// dimming = 114 is darkest
void loop() {  
  on_off_value = digitalRead(ONOFF_IN_PIN);
  //Serial.println(on_off_value );
  if (on_off_value == LOW) {
    dimming = 128;
  } else {
    temp_pwm_value = pwm_value;
    if (prev_temp_pwm_value != temp_pwm_value) {
      prev_temp_pwm_value = temp_pwm_value;
      //Serial.println(temp_pwm_value );
      
      // Get PWM and make sure we don't see any unexpected values
      if ((temp_pwm_value < 0) || (temp_pwm_value > 900)) {
        return; 
      }
  
      temp_pwm_value = constrain(temp_pwm_value, 10, 800); 
  
      // Convert to values used by the dimmer circuit
      temp_dimming = map(temp_pwm_value, 10, 800, 10, 114);;
      smoother.addValue(temp_dimming );
      temp_dimming  = smoother.getAverage();

      dimming = temp_dimming;
      //Serial.println(temp_dimming);
    }
  }
}


/*******************************************************/
// Dimmer
/*******************************************************/
void zero_cross_detect() {    
  zero_cross = true;               // set the boolean to true to tell our dimming function that a zero cross has occured
  i=0;
  digitalWrite(AC_LOAD_PIN, LOW);       // turn off TRIAC (and AC)
}   

// Turn on the TRIAC at the appropriate time
void dim_check() {                   
  if(zero_cross == true) {              
    if(i>=dimming) {                     
      digitalWrite(AC_LOAD_PIN, HIGH); // turn on light       
      i=0;  // reset time step counter                         
      zero_cross = false; //reset zero cross detection
    } 
    else {
      i++; // increment time step counter                     
    }                                
  }                                  
}                 
/*******************************************************/
// PWM-in functions
/*******************************************************/
void rising() {
  attachInterrupt(digitalPinToInterrupt(PWM_IN_PIN), falling, FALLING);
  prev_time = micros();
}
 
void falling() {
  attachInterrupt(digitalPinToInterrupt(PWM_IN_PIN), rising, RISING);
  if (! interrupted) {
    pwm_value = micros()-prev_time;
  }
  interrupted = false;
}

