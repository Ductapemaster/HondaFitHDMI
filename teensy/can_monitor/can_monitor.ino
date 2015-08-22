// -------------------------------------------------------------
// can_monitor for Teensy 3.1
// by ductapemaster
//
// Receives CAN frames on the internal CAN interface and prints
// them in human readable format to the serial port.
//
// To Do: should make print function more like the socketcan candump output
//

#include <FlexCAN.h>

int led = 13;
FlexCAN CANbus(125000);
static CAN_message_t rxmsg;

// -------------------------------------------------------------
static void printCanFrame(CAN_message_t f)
{
      Serial.print(millis());
      Serial.print("ms");
      Serial.print("\t0x");
      Serial.print(f.id, HEX);
      Serial.print("\t[");
      Serial.print(f.len);
      Serial.print("]\t");
      for (int i = 0; i < f.len; i++) 
      {
        Serial.print(f.buf[i], HEX);
        Serial.print("  ");
      }
      Serial.write('\r');
      Serial.write('\n');
}

// -------------------------------------------------------------
void setup(void)
{
  
  CANbus.begin();
  Serial.begin(115200);
  
  pinMode(led, OUTPUT);
  digitalWrite(led, 1);

  delay(1000);
  
  // F() stores string in flash rather than RAM
  Serial.println(F("Teensy CAN Bus Monitor"));

}


// -------------------------------------------------------------
void loop(void)
{
    while ( CANbus.read(rxmsg) ) {

      // Toggle our LED to show bus activity
      digitalWrite(led, 1);
      printCanFrame(rxmsg);
      
    }
    
    digitalWrite(led, 0);

}

