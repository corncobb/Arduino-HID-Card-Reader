/*
 * HID RFID Reader Wiegand Interface for Arduino Uno
 * Originally by Daniel Smith, 2012.01.30 -- http://www.pagemac.com/projects/rfid/arduino_wiegand
 * 
 * Most recent update 2018-10-20 by Cameron Cobb.
 * Visit https://www.cameroncobbconsulting.com for details, instructions, video, and contact.
 * Works for more advanced card reader like the Software House SWH 5100 and Deister Electronic PRx5
 * NOTE: SWH 5100 is the same card reader as Deister PRx5 just with a different cover...
*/
 
 
#define MAX_BITS 100                 // max number of bits 
#define WEIGAND_WAIT_TIME  3000      // time to wait for another weigand pulse.  
 
unsigned char databits[MAX_BITS];    // stores all of the data bits
unsigned char bitCount;              // number of bits currently captured
unsigned char flagDone;              // goes low when data is currently being captured
unsigned int weigand_counter;        // countdown until we assume there are no more bits
 
unsigned long facilityCode=0;        // decoded facility code
unsigned long cardCode=0;            // decoded card code

int LED_GREEN = 11;
int LED_RED = 12;
int BEEP_BEEP = 10;

// interrupt that happens when INTO goes low (0 bit)
void ISR_INT0() {
  bitCount++;
  flagDone = 0;
  weigand_counter = WEIGAND_WAIT_TIME;  
 
}
 
// interrupt that happens when INT1 goes low (1 bit)
void ISR_INT1() {
  databits[bitCount] = 1;
  bitCount++;
  flagDone = 0;
  weigand_counter = WEIGAND_WAIT_TIME;  
}
 
void setup() {
  pinMode(LED_RED, OUTPUT);  
  pinMode(LED_GREEN, OUTPUT);  
  pinMode(BEEP_BEEP, OUTPUT);  
  digitalWrite(LED_RED, HIGH); // High = Off
  digitalWrite(BEEP_BEEP, HIGH); // High = off
  digitalWrite(LED_GREEN, LOW);  // Low = On

  //Make sure the mode is INPUT_PULLUP for Software House SWH 5100 Model.
  pinMode(2, INPUT_PULLUP);     // DATA0 (INT0) 
  pinMode(3, INPUT_PULLUP);     // DATA1 (INT1)
 
  Serial.begin(9600);
  Serial.println("RFID Readers");
 
  // binds the ISR functions to the falling edge of INTO and INT1
  attachInterrupt(0, ISR_INT0, FALLING);  
  attachInterrupt(1, ISR_INT1, FALLING);
 
 
  weigand_counter = WEIGAND_WAIT_TIME;
}
 
void loop()
{ 
  // This waits to make sure that there have been no more data pulses before processing data
  if (!flagDone) {
    if (--weigand_counter == 0)
      flagDone = 1; 
  }
 
  // if we have bits and we the weigand counter went out
  if (bitCount > 0 && flagDone) {
    unsigned char i;
    Serial.print("Raw Binary = ");
    for (i=0; i<bitCount; i++) 
     {
       Serial.print(databits[i]);
     }
    Serial.print("\nBits = ");
    Serial.print(bitCount);

    
    if (bitCount == 35) {
      Serial.print("\nCard format: HID Corporate 1000");
      // facility code = bits 2 to 14
      for (i=2; i<14; i++) {
         facilityCode <<=1;
         facilityCode |= databits[i];
      }
 
      // card code = bits 15 to 34
      for (i=14; i<34; i++) {
         cardCode <<=1;
         cardCode |= databits[i];
      }
 
      printBits();
    }
    else if (bitCount == 26) {
      Serial.print("\nCard format: Standard 26 bit format");
      // facility code = bits 2 to 9
      for (i=1; i<9; i++) {
         facilityCode <<=1;
         facilityCode |= databits[i];
      }
 
      // card code = bits 10 to 23
      for (i=9; i<25; i++) {
         cardCode <<=1;
         cardCode |= databits[i];
      }
      printBits();
    }
    else if (bitCount == 40) {
      Serial.print("\nCard format: Casi-Rusco 40 Bit");
      Serial.print("\n\n**************************\n\n"); 
      
    }else{

     // you can add other formats if you want!
     Serial.print("\nUnable to decode.");
     Serial.print("\n\n**************************\n\n"); 
    }
     // cleanup and get ready for the next card
     bitCount = 0;
     facilityCode = 0;
     cardCode = 0;
     for (i=0; i<MAX_BITS; i++) 
     {
       databits[i] = 0;
     }
  }
}
 
void printBits(){

      Serial.print("\nFacility Code = ");
      Serial.print(facilityCode);
      Serial.print("\nCard Code = ");
      Serial.print(cardCode);
      Serial.print("\n\n**************************\n\n");

      // Now lets play with some LED's for fun:
      digitalWrite(LED_RED, LOW); // Red
      if(cardCode == 432279){
        // If this one "bad" card, turn off green
        // so it's just red. Otherwise you get orange-ish
        digitalWrite(LED_GREEN, HIGH); 
      }else {
      digitalWrite(LED_GREEN, LOW); 
      digitalWrite(LED_RED, HIGH); 
      delay(100);
      digitalWrite(LED_RED, LOW); 
      delay(100); 
      digitalWrite(LED_RED, HIGH); 
      delay(100);
      digitalWrite(LED_RED, LOW);
      delay(100);// Red Off
      digitalWrite(LED_RED, HIGH);
      delay(300);// Red Off  
 
      // Lets be annoying and beep more
      digitalWrite(BEEP_BEEP, LOW);
      delay(500);
      digitalWrite(BEEP_BEEP, HIGH);
      delay(500);
      digitalWrite(BEEP_BEEP, LOW);
      delay(500);
      digitalWrite(BEEP_BEEP, HIGH);
      }
}
