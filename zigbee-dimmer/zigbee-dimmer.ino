unsigned char AC_LOAD_PIN = 5;    // Output to Opto Triac pin
unsigned char SYNC_PIN = 2; // On trinket and Uno, external interrupt is pin #2

unsigned char PWM_IN_PIN = 3;
unsigned char ONOFF_IN_PIN = 6;

unsigned char LED_PIN = 13;

unsigned char dimming = 3;  // Dimming level (0-100)
unsigned char i;

volatile boolean int_called = false;
volatile int pwm_value = 0;
volatile int prev_time = 0;
int on_off_value = 0;

void setup() {
  pinMode(AC_LOAD_PIN, OUTPUT);// Set AC Load pin as output
  
  // Dimmer sync. Pin 2 on Uno, Pin 2 on attiny
  attachInterrupt(0, zero_crosss_int, RISING);

  // PIN for reading PWM from CREE bulb circuit
  pinMode(PWM_IN_PIN, INPUT);  
  attachInterrupt(digitalPinToInterrupt(PWM_IN_PIN), falling, FALLING);

  pinMode(ONOFF_IN_PIN, INPUT); 
  
  // initialize the digital pin as an output.
  pinMode(LED_PIN, OUTPUT);  

  //Serial.begin(9600);
}


void loop() {
  if (int_called) {
    int_called = false;
  }  

  on_off_value = digitalRead(ONOFF_IN_PIN);
  if (on_off_value == LOW) {
    dimming = 0;
  } else {
    // almost full on = 50
    // mid-way - 500
    // low = 900
    int temp_pwm_value = pwm_value;
  
    // Make sure we don't see any unexpected values
    if (temp_pwm_value > 890) {
      temp_pwm_value = 890;
    } else if (temp_pwm_value < 0) {
      temp_pwm_value = 0;
    }
  
    int temp_dimming = map(temp_pwm_value, 0, 900, 10, 114);;
  
    // Handle values that the specific LED bulb does not light
    if (temp_dimming > 110) {
      temp_dimming = 110;
    }
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
  // Firing angle calculation : 1 full 50Hz wave =1/50=20ms 
  // Every zerocrossing : (50Hz)-> 10ms (1/2 Cycle) For 60Hz (1/2 Cycle) => 8.33ms 
  // 10ms=10000us

  int_called = true;
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
}
 
void falling() {
  attachInterrupt(digitalPinToInterrupt(PWM_IN_PIN), rising, RISING);
  pwm_value = micros()-prev_time;
}
