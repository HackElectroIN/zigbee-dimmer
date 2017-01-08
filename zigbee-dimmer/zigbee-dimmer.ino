unsigned char AC_LOAD_PIN = 1;    // Output to Opto Triac pin
unsigned char SYNC_PIN = 2; // Need pin 2 or 3 for interrupts

unsigned char PWM_IN_PIN = 3; // Need pin 2 or 3 for interrupts
unsigned char ONOFF_IN_PIN = 4;

volatile unsigned char dimming = 10;  // Dimming level (0-100)
volatile boolean interrupted = false; // the dimmer interrupt sometimes messes up the PWM readings.
unsigned char i;

volatile int pwm_value = 0;
volatile int prev_time = 0;

int on_off_value = 0;
int temp_pwm_value;
unsigned char temp_dimming;
int prev_dimmering = dimming;

void setup() {
  //Serial.begin(9600);

  // Set AC Load pin as output
  pinMode(AC_LOAD_PIN, OUTPUT);
  
  // Dimmer sync. Pin 2 on Uno, Pin 2 on attiny
  pinMode(SYNC_PIN, INPUT);  
  attachInterrupt(digitalPinToInterrupt(SYNC_PIN), zero_crosss_int, RISING);

  // PIN for reading PWM from CREE bulb circuit
  pinMode(PWM_IN_PIN, INPUT);  
  attachInterrupt(digitalPinToInterrupt(PWM_IN_PIN), falling, FALLING);

  pinMode(ONOFF_IN_PIN, INPUT);   
}


void loop() {  
  on_off_value = digitalRead(ONOFF_IN_PIN);
  if (on_off_value == LOW) {
    dimming = 0;
  } else {
      
    // Get PWM and make sure we don't see any unexpected values
    temp_pwm_value = pwm_value;
    temp_pwm_value = constrain(temp_pwm_value, 0, 890); 

    // Convert to values used by the dimmer circuit
    temp_dimming = map(temp_pwm_value, 0, 890, 10, 114);;
    temp_dimming = constrain(temp_dimming, 10, 114); 

    dimming = temp_dimming;
    // Serial prints will mess up the timing
    //Serial.print(temp_pwm_value);
    //Serial.print(" - ");
    //Serial.println(temp_dimming);
  }
}


/*******************************************************/
// Dimmer
/*******************************************************/
void zero_crosss_int()  // function to be fired at the zero crossing to dim the light
{
  interrupted = true;
  // Firing angle calculation : 1 full 50Hz wave =1/50=20ms 
  // Every zerocrossing : (50Hz)-> 10ms (1/2 Cycle) For 60Hz (1/2 Cycle) => 8.33ms 
  // 10ms=10000us
  int dimtime = (65*dimming);    // For 60Hz =>65    
  delayMicroseconds(dimtime);    // Off cycle
  digitalWrite(AC_LOAD_PIN, HIGH);   // triac firing
  delayMicroseconds(8.33);         // triac On propogation delay (for 60Hz use 8.33)
  digitalWrite(AC_LOAD_PIN, LOW);    // triac Off
}

/*******************************************************/
// PWM-in functions
/*******************************************************/
void rising() {
  attachInterrupt(digitalPinToInterrupt(PWM_IN_PIN), falling, FALLING);
  prev_time = micros();
  interrupted = false;
}
 
void falling() {
  attachInterrupt(digitalPinToInterrupt(PWM_IN_PIN), rising, RISING);
  if (! interrupted) {
    pwm_value = micros()-prev_time;
  }
}

