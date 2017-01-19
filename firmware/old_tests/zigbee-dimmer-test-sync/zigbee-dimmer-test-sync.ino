unsigned char AC_LOAD = 5;    // Output to Opto Triac pin
unsigned char dimming = 3;  // Dimming level (0-100)
//unsigned char LED_PIN = 13; 
unsigned char i;
volatile boolean int_called = false;

void zero_crosss_int()  // function to be fired at the zero crossing to dim the light
{
  // Firing angle calculation : 1 full 50Hz wave =1/50=20ms 
  // Every zerocrossing : (50Hz)-> 10ms (1/2 Cycle) For 60Hz (1/2 Cycle) => 8.33ms 
  // 10ms=10000us

  int_called = true;
  int dimtime = (65*dimming);    // For 60Hz =>65    
  delayMicroseconds(dimtime);    // Off cycle
  digitalWrite(AC_LOAD, HIGH);   // triac firing
  delayMicroseconds(8.33);         // triac On propogation delay (for 60Hz use 8.33)
  digitalWrite(AC_LOAD, LOW);    // triac Off
}


void setup() {
  // put your setup code here, to run once:
  pinMode(AC_LOAD, OUTPUT);// Set AC Load pin as output
  //pinMode(dimming, INPUT); # no need . pin 3 is already int1
  attachInterrupt(0, zero_crosss_int, RISING);

  //Serial.begin(9600);
}


void loop() {
          //Serial.println("here");
          //Serial.println("yo!");   

  dimming=115;;

/*
          for (i=99;i<115;i++)
          {
            dimming=i;
            delay(20);
          }
        /*
          for (i=85;i>5;i--)
          {
            dimming=i;
            delay(20);
          }
         */ 
}
