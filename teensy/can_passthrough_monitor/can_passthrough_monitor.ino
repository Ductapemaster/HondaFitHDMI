// -------------------------------------------------------------
// can_passthrough_monitor for Teensy 3.1
// by ductapemaster
//
// Receives on internal CAN interface and copies all data to 
// MCP2515 interface.  Works only in one direction.
//
// Received frames are printed to serial port for monitoring

#include <FlexCAN.h>
#include <mcp_can.h>
#include <SPI.h>

// LED for receive indication.  Can't use onboard one because it shares the SPI SCLK pin
int led = 14;

//FlexCAN
FlexCAN CANIn(125000);
static CAN_message_t rxmsg;

//MCP2515 CAN
//Interrupt is D2, from schematic
//Default SPI Pins on Teensy:
// CS:        10
// DOUT/MOSI: 11
// DIN/MISO   12
// SCLK       13 (also the LED!)

//MCP library requires CS pin as argument.  Use default
const int csPin = 10;
MCP_CAN CANOut(csPin);

// -------------------------------------------------------------
static void printCanFrame(CAN_message_t f)
{
      Serial.print(millis());
      Serial.print("ms");
      Serial.print("\t");
      Serial.print(f.id, HEX);
      Serial.print("[\t");
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
  
  CANIn.begin();
  Serial.begin(115200);
  
  pinMode(led, OUTPUT);
  digitalWrite(led, 1);

  delay(1000);
  
  // F() stores string in flash rather than RAM
  Serial.println(F("Teensy CAN Passthrough and Monitoring"));

  Serial.print(F("Initializing MCP2515 interface..."));

  // Attempt to initialize module
  while (CAN_OK != CANOut.begin(CAN_125KBPS))
  {
    // Loops forever - probably should implement some sort of timeout.  Watchdog?
    Serial.print(".");
    delay(100);
  }
  
  Serial.println("success\r\n");

}


// -------------------------------------------------------------
void loop(void)
{
    while ( CANIn.read(rxmsg) ) {

      // Toggle our LED to show bus activity
      digitalWrite(led, 1);
      
      printCanFrame(rxmsg);
      
      // ID, CAN Mode: 1=extended, buffer length, buffer
      CANOut.sendMsgBuf(rxmsg.id, 1, rxmsg.len, rxmsg.buf);
      
    }

    digitalWrite(led, 0);

}

