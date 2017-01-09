unsigned char AC_LOAD_PIN = 1;    // Output to Opto Triac pin
volatile int prev_sync_stat = 0;
volatile int prev_pwm_stat = 0;

volatile boolean interrupted = false; // the dimmer interrupt sometimes messes up the PWM readings.

volatile int pwm_value = 0;
volatile int prev_time = 0;

void setup()
{
  Serial.begin(9600);
  Serial.println("Boe");
// Set AC Load pin as output
  pinMode(AC_LOAD_PIN, OUTPUT);
  InitialiseIO();
  InitialiseInterrupt();
}

void loop() {
  /* Nothing to do: the program jumps automatically to Interrupt Service Routine "blink"
   in case of a hardware interrupt  */
   Serial.println(pwm_value);        

}  

void InitialiseIO(){
  pinMode(A0, INPUT);     // Pin A0 is input to which a switch is connected
  digitalWrite(A0, HIGH);   // Configure internal pull-up resistor  
  pinMode(A1, INPUT);     // Pin A0 is input to which a switch is connected
  digitalWrite(A1, HIGH);   // Configure internal pull-up resistor  

}

void InitialiseInterrupt(){
  cli();    // switch interrupts off while messing with their settings  
  PCICR =0x02;          // Enable PCINT1 interrupt
  PCMSK1 = 0b00000111;
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
    int dimming = 114;
    int dimtime = (65*dimming);    // For 60Hz =>65    
    delayMicroseconds(dimtime);    // Off cycle
    digitalWrite(AC_LOAD_PIN, HIGH);   // triac firing
    delayMicroseconds(8.33);         // triac On propogation delay (for 60Hz use 8.33)
    digitalWrite(AC_LOAD_PIN, LOW);    // triac Off
  }
  
  if ((digitalRead(A0) == 0) && (prev_pwm_stat == 1)) {
    //Serial.println("A0-0");
    if (! interrupted) {
      pwm_value = micros()-prev_time;
    }
    prev_pwm_stat = 0;
  }
  
  if ((digitalRead(A0) == 1) && (prev_pwm_stat == 0)) {
    //Serial.println("A0-1");
    interrupted = false;
    prev_time = micros();
    prev_pwm_stat = 1;
  }
}
