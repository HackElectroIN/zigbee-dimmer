//unsigned char AC_LOAD = 3;    // Output to Opto Triac pin
// unsigned char SYNC_PIN = 2; // On trinket, external interrupt is pin #2
unsigned char PWM_IN_PIN = 2;
unsigned char LED_PIN = 1; 

unsigned char dimming = 3;  // Dimming level (0-100)

unsigned char i;

volatile boolean int_called = false;
volatile int pwm_value = 0;
volatile int prev_time = 0;
volatile boolean is_rising = false;
volatile boolean is_falling = false;


void zero_crosss_int()  // function to be fired at the zero crossing to dim the light
{
  int_called = true;
}


void setup() {
  // dimmer sync
  // On trinket, external interrupt 0 is pin #2
  //attachInterrupt(0, zero_crosss_int, RISING);


  // If using external interrupt. Pin 2 on Uno, Pin 2 on attiny
  attachInterrupt(0, rising, RISING);

  // If using regular digital interrupt
  //pinMode(PWM_IN_PIN, INPUT);  
  //attachInterrupt(digitalPinToInterrupt(PWM_IN_PIN), falling, CHANGE);
  
  // initialize the digital pin as an output.
  pinMode(LED_PIN, OUTPUT);  

  Serial.begin(9600);  
}


void loop() {
  //dimming=115;;
  /*
  if (int_called) {
    digitalWrite(LED_PIN, HIGH);   // triac firing
    delay(500);         // triac On propogation delay (for 60Hz use 8.33)
    digitalWrite(LED_PIN, LOW);    // triac Off
    delay(500);         // triac On propogation delay (for 60Hz use 8.33)
    int_called = false;
  }  
  */
  
  // almost full on = 50
  // mid-way - 500
  // low = 900
  int temp_pwm_value = pwm_value;
  //Serial.println(temp_pwm_value);
  int mapped = map(temp_pwm_value, 0, 900, 115, 0);
  Serial.println(mapped);
  if (is_falling) {
      is_falling = false;
      is_rising = false;
      Serial.println("YOYOYO!");
  }
  //Serial.println(mapped);
  //Serial.println(is_rising);
}


void rising() {
  attachInterrupt(0, falling, FALLING);
  prev_time = micros();
  is_falling = false;
  is_rising = true;
}
 
void falling() {
  attachInterrupt(0, rising, RISING);
  pwm_value = micros()-prev_time;
  is_falling = true;
  is_rising = false;
}
