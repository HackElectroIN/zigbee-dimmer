unsigned char AC_LOAD_PIN = 1;    // Output to Opto Triac pin
unsigned char SYNC_PIN = A1; // Need pin 2 or 3 for interrupts
unsigned char PWM_IN_PIN = A0; // Need pin 2 or 3 for interrupts
unsigned char ONOFF_IN_PIN = 4;

volatile unsigned char dimming = 10;  // Dimming level (0-100)
volatile boolean interrupted = false; // the dimmer interrupt sometimes messes up the PWM readings.

volatile int pwm_value = 0;
volatile int prev_time = 0;

volatile int prev_sync_stat = 0;
volatile int prev_pwm_stat = 0;

int on_off_value = 0;
int temp_pwm_value;
unsigned char temp_dimming;

void setup() {
  //Serial.begin(9600);

  // Set AC Load pin as output
  pinMode(AC_LOAD_PIN, OUTPUT);
  
  
  pinMode(ONOFF_IN_PIN, INPUT);

  InitialiseIO();
  InitialiseInterrupt();
}

void InitialiseIO(){
  pinMode(A0, INPUT);     // Pin A0 is input to which a switch is connected
  digitalWrite(A0, HIGH);   // Configure internal pull-up resistor  
  pinMode(A1, INPUT);     // Pin A0 is input to which a switch is connected
  digitalWrite(A1, HIGH);   // Configure internal pull-up resistor  
}

void loop() {  
  on_off_value = digitalRead(ONOFF_IN_PIN);
  if (on_off_value == LOW) {
    dimming = 0;
  } else {
    temp_pwm_value = pwm_value;
    //Serial.println(temp_pwm_value );
    
    // Get PWM and make sure we don't see any unexpected values
    temp_pwm_value = pwm_value;
    if ((temp_pwm_value < 0) || (temp_pwm_value > 1000)) {
      return; 
    }    
    temp_pwm_value = constrain(temp_pwm_value, 10, 800); 

    // Convert to values used by the dimmer circuit
    temp_dimming = map(temp_pwm_value, 10, 800, 10, 114);;
    temp_dimming = constrain(temp_dimming, 10, 110); 
    //Serial.println(temp_dimming);
    dimming = temp_dimming;//temp_dimming;
  }
}

void InitialiseInterrupt(){
  cli();    // switch interrupts off while messing with their settings  
  PCICR =0x02;          // Enable PCINT1 interrupt
  PCMSK1 = 0b00000011;
  sei();    // turn interrupts back on
}

ISR(PCINT1_vect) {    // Interrupt service routine. Every single PCINT8..14 (=ADC0..5) change
            // will generate an interrupt: but this will always be the same interrupt routine
  
  if ((digitalRead(A1) == 0) && (prev_sync_stat == 1)) {
    //Serial.println("A1-0");    
    prev_sync_stat = 0;
  }
  
  if ((digitalRead(A1) == 1) && (prev_sync_stat == 0)) {
    interrupted = true;
    //Serial.println("A1-1");    
    prev_sync_stat = 1;
    int dimtime = (65*dimming);    // For 60Hz =>65    
    delayMicroseconds(dimtime);    // Off cycle
    digitalWrite(AC_LOAD_PIN, HIGH);   // triac firing
    delayMicroseconds(8.33);         // triac On propogation delay (for 60Hz use 8.33)
    digitalWrite(AC_LOAD_PIN, LOW);    // triac Off
  }
  
  if ((digitalRead(A0) == 0) && (prev_pwm_stat == 1)) {
    //Serial.println("A0-0");
    if (!interrupted) {
      pwm_value = micros()-prev_time;
    }
    prev_pwm_stat = 0;
    interrupted = false;
  }
  
  if ((digitalRead(A0) == 1) && (prev_pwm_stat == 0)) {
    //Serial.println("A0-1");
    prev_time = micros();
    prev_pwm_stat = 1;
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
  int dimtime = (65*dimming);    // For 60Hz =>65    
  delayMicroseconds(dimtime);    // Off cycle
  digitalWrite(AC_LOAD_PIN, HIGH);   // triac firing
  delayMicroseconds(8.33);         // triac On propogation delay (for 60Hz use 8.33)
  digitalWrite(AC_LOAD_PIN, LOW);    // triac Off
}


