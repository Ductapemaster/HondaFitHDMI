// -------------------------------------------------------------
// hdmi_enable for Teensy 3.1
// by ductapemaster
//
// This code successfully disables HDMI lockout and lockout of specific features at speed.
//
// Two frames are edited before passing through:
// 1. Gear shifter value set permanently to "park"
// 2. Vehicle speed value set permanently to '0'
//
// Only passes CAN data from car to head unit.  Head unit output frames are currently unhandled.

#include <FlexCAN.h>
#include <teensy_mcp_can.h>
#include <spi4teensy3.h>

//#define debug

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

//MCP library requires CS pin as argument.  Use default.
// this actually doesn't matter since we are overwriting the calls to the Arduino SPI lib with calls to spi4teensy3
const int csPin = 10;
MCP_CAN CANOut(csPin);
bool filter_out = false;

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
  
  CANIn.begin();
  Serial.begin(115200);
  
  pinMode(led, OUTPUT);
  digitalWrite(led, 1);

  delay(1000);
  
  // F() stores string in flash rather than RAM
  Serial.println(F("Teensy CAN Passthrough Test"));

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
    // Poll CAN buffer for new data
    while ( CANIn.read(rxmsg) ) {

      filter_out = false;

      // Toggle our LED to show bus activity
      digitalWrite(led, 1);
      
      #ifdef debug
      printCanFrame(rxmsg);
      #endif

      // Modify multiple frames based on conditional data
      switch (rxmsg.id) {

        // Gear Shifter/Parking Brake Position
        case 0x12F85150:
          
          Serial.println("changing gear to park!");
          //Serial.print("before: ");
          //printCanFrame(rxmsg);
          rxmsg.buf[0] = 0x40;
          rxmsg.buf[1] = 0x03;
          //Serial.print("after: ");
          //printCanFrame(rxmsg);
          break;

        // Vehicle Speed/Engine RPM - verified!
        case 0x12F85050:
          rxmsg.buf[0] = 0x00;
          //filter_out = true;
          break;        
          
        default:
          break;
        
      }

      // Write buffered data to second CAN bus
      // ID, CAN Mode: 1=extended, buffer length, buffer     
      // There's got to be a better way to do this - can we use the canframe object?
      if (!filter_out)
      {
        CANOut.sendMsgBuf(rxmsg.id, 1, rxmsg.len, rxmsg.buf);
      }
      
    }

    digitalWrite(led, 0);

}

