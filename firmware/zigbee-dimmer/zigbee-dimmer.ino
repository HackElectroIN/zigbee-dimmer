#include  <TimerOne.h> 

#include "RunningAverage.h"
RunningAverage smoother(15);

// pins
unsigned char LED_PIN = 13;    // 

unsigned char AC_LOAD_PIN = 6;    // Output to Opto Triac pin
unsigned char SYNC_PIN = 2; // Need pin 2 or 3 for interrupts

unsigned char PWM_IN_PIN = 3; // Need pin 2 or 3 for interrupts
unsigned char ONOFF_IN_PIN = 8;

volatile unsigned char DIM_DARKEST = 108;
volatile unsigned char DIM_BRIGHTEST = 10;

// Dimmer stuff
volatile boolean zero_cross=0;  // Boolean to store a "switch" to tell us if we have crossed zero
volatile unsigned char dimming = DIM_BRIGHTEST;  // Dimming level (0-~100). Set to "BRIGHT" so the
                                                 // lights are on by default

//PWM reading stuff
volatile boolean pwm_seen = false; // when the cree module is turned on, sometimes no PWM
                                   // values are available until the HUE system sends a new value
volatile boolean interrupted = false; // the dimmer interrupt sometimes messes up the PWM readings.
volatile int pwm_value = DIM_BRIGHTEST;
volatile unsigned long prev_time = 0; // used for measuring PWM

// variables used in the loop
int on_off_value = 0;
int temp_pwm_value;
int prev_temp_pwm_value = -1;
unsigned char temp_dimming;
int led_value = 0;

void setup() {
  //Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  
  // Set AC Load pin as output
  pinMode(AC_LOAD_PIN, OUTPUT);
  
  // Dimmer sync. Pin 2 on Uno, Pin 2 on attiny
  pinMode(SYNC_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(SYNC_PIN), zero_cross_called, RISING);

  // PIN for reading PWM from CREE bulb circuit
  pinMode(PWM_IN_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(PWM_IN_PIN), falling, FALLING);

  pinMode(ONOFF_IN_PIN, INPUT); 
  delay(500);
  //Serial.println("Started");
}

// dimming = 10 is brightest
// dimming = 114 is darkest, but flickers with some bulbs. Might need a different value.
void loop() {  
  //TODO: replace with interrupt (mode:change)
  on_off_value = digitalRead(ONOFF_IN_PIN);
  //Serial.println(on_off_value );
  if (on_off_value == LOW) {
    dimming = 128;
  } else {
    // LED
    led_value = map(dimming, DIM_BRIGHTEST, DIM_DARKEST, 255, 10);;
    analogWrite(LED_PIN,led_value);

    // Check PWM value from Cree, calculate dim level if needed
    temp_pwm_value = pwm_value;
      //Serial.print(pwm_seen);
      //Serial.print(",");
      //Serial.println(temp_pwm_value );
    
    if (prev_temp_pwm_value != temp_pwm_value) {      
      prev_temp_pwm_value = temp_pwm_value;
      //Serial.println(temp_pwm_value );
      
      // Get PWM and ignore any unexpected values
      if ((temp_pwm_value < 0) || (temp_pwm_value > 900)) {
        return; 
      }
  
      temp_pwm_value = constrain(temp_pwm_value, 10, 800); 
  
      // Convert to values used by the dimmer circuit
      temp_dimming = map(temp_pwm_value, 10, 800, DIM_BRIGHTEST, DIM_DARKEST);;
      smoother.addValue(temp_dimming );
      temp_dimming  = smoother.getAverage();
      dimming = temp_dimming;
      //Serial.print("DIMMING:");
      //Serial.println(dimming);
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
  pwm_seen = true;
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

