unsigned char AC_LOAD_PIN = 5;    // Output to Opto Triac pin
unsigned char SYNC_PIN = 2; // On trinket and Uno, external interrupt is pin #2

unsigned char PWM_IN_PIN = 3;
unsigned char ONOFF_IN_PIN = 6;

unsigned char LED_PIN = 13;

unsigned char dimming = 3;  // Dimming level (0-100)
unsigned char i;

volatile int pwm_value = 0;
volatile int prev_time = 0;

int on_off_value = 0;
int temp_pwm_value;
unsigned char temp_dimming;
int prev_dimmering = dimming;

// smoothing readings
const int numReadings = 10;
int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average


void setup() {
  //Serial.begin(9600);

  // Set AC Load pin as output
  pinMode(AC_LOAD_PIN, OUTPUT);
  
  // Dimmer sync. Pin 2 on Uno, Pin 2 on attiny
  attachInterrupt(0, zero_crosss_int, RISING);

  // PIN for reading PWM from CREE bulb circuit
  pinMode(PWM_IN_PIN, INPUT);  
  attachInterrupt(digitalPinToInterrupt(PWM_IN_PIN), falling, FALLING);

  pinMode(ONOFF_IN_PIN, INPUT); 
  
  // initialize the digital pin as an output.
  pinMode(LED_PIN, OUTPUT);  

  // Smoothing array
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
}


void loop() {  

  on_off_value = digitalRead(ONOFF_IN_PIN);
  if (on_off_value == LOW) {
    dimming = 0;
  } else {
      
    // Get PWM and make sure we don't see any unexpected values
    temp_pwm_value = pwm_value;
    temp_pwm_value = constrain(temp_pwm_value, 0, 890); 
    temp_pwm_value = add_reading(temp_pwm_value);

    // Convert to values used by the dimmer circuit
    temp_dimming = map(temp_pwm_value, 0, 890, 10, 114);;
  
    // Handle values that the specific LED bulb does not light
    temp_dimming = constrain(temp_dimming, 10, 114); 
    if (temp_dimming > 110) {
      temp_dimming = 114;
    }
    dimming = temp_dimming;
    // Serial prints will mess up the timing
    //Serial.print(temp_pwm_value);
    //Serial.print(" - ");
    //Serial.println(temp_dimming);
  }
}

/*******************************************************/
// Smooth
// https://www.arduino.cc/en/tutorial/smoothing
/*******************************************************/
int add_reading(int reading) {
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = reading;
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    readIndex = 0;
  }

  // calculate the average:
  average = total / numReadings;
  return average;
}

/*******************************************************/
// Dimmer
/*******************************************************/
void zero_crosss_int()  // function to be fired at the zero crossing to dim the light
{
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
}
 
void falling() {
  attachInterrupt(digitalPinToInterrupt(PWM_IN_PIN), rising, RISING);
  pwm_value = micros()-prev_time;
}
