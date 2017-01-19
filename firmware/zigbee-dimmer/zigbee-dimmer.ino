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
  //Serial.begin(9600);
  
  // Set AC Load pin as output
  pinMode(AC_LOAD_PIN, OUTPUT);
  
  // Dimmer sync. Pin 2 on Uno, Pin 2 on attiny
  pinMode(SYNC_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(SYNC_PIN), zero_cross_called, RISING);

  // PIN for reading PWM from CREE bulb circuit
  pinMode(PWM_IN_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(PWM_IN_PIN), falling, FALLING);

  pinMode(ONOFF_IN_PIN, INPUT); 
}

// dimming = 10 is brightest
// dimming = 114 is darkest
void loop() {  
  //TODO: replace with interrupt (mode:change)
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
    }
  }
}


/*******************************************************/
// Dimmer
/*******************************************************/
void zero_cross_called() {
  interrupted = true;
  detachInterrupt(digitalPinToInterrupt(PWM_IN_PIN));
  int dimtime = 65*dimming;     //65?  
  Timer1.initialize(dimtime);                      // Initialize TimerOne library for the freq we need
  Timer1.attachInterrupt(zero_cross_step2);      
}

void zero_cross_step2() {
  digitalWrite(AC_LOAD_PIN, HIGH);   // triac firing
  delayMicroseconds(8.33);         // triac On propogation delay
  digitalWrite(AC_LOAD_PIN, LOW);    // triac Off
  Timer1.stop();
  attachInterrupt(digitalPinToInterrupt(PWM_IN_PIN), falling, FALLING);
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

