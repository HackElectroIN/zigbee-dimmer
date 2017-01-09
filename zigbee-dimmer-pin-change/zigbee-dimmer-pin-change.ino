void setup()
{
  Serial.begin(9600);
  Serial.println("Boe");
  InitialiseIO();
  InitialiseInterrupt();
}

void loop() {
  /* Nothing to do: the program jumps automatically to Interrupt Service Routine "blink"
   in case of a hardware interrupt  */
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
  if (digitalRead(A0)==0)  Serial.println("A0-0");
  if (digitalRead(A0)==1)  Serial.println("A0-1");  
  if (digitalRead(A1)==1)  Serial.println("A1-1");    
}
