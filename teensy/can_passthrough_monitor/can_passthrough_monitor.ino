// -------------------------------------------------------------
// can_passthrough_monitor for Teensy 3.1
// by ductapemaster
//
// CAN data is mirrored in both directions. Received frames are 
// printed to serial port for monitoring in either tab separated
// or comma separated values.  The character 'c' toggles between
// output formats.

#include <FlexCAN.h>
#include <mcp_can.h>
#include <SPI.h>

// LED for receive indication.  Can't use onboard one because it shares the SPI SCLK pin
int led = 14;

// FlexCAN
FlexCAN B_CAN(125000);
static CAN_message_t rxmsg;

/*
 * typedef struct CAN_message_t {
 * uint32_t id; // can identifier
 * uint8_t ext; // identifier is extended
 * uint8_t len; // length of data
 * uint16_t timeout; // milliseconds, zero will disable waiting
 * uint8_t buf[8];
 * } CAN_message_t;
 */

// MCP2515 CAN

// Interrupt is D2, from schematic
// Default SPI Pins on Teensy:
// CS:        10
// DOUT/MOSI: 11
// DIN/MISO   12
// SCLK       13 (also the LED!)

const int csPin = 10;     // MCP library requires CS pin as argument.  Use default
MCP_CAN HU_CAN(csPin);

// Serial output separator - default is tsv
static char sep = '\t';

static void printFrame(CAN_message_t f)
{
      Serial.printf("%li%c%08X%c%i", millis(), sep, f.id, sep, f.len);
      
      for (int i = 0; i < f.len; i++) 
      {
        Serial.printf("%c%02X", sep, f.buf[i]);
      }
      
      Serial.println();
}

static void printCSVHeader(CAN_message_t f)
{
  Serial.println(F("Time (ms),CAN Arbitration ID,Bytes,Byte[0],Byte[1],Byte[2],Byte[3],Byte[4],Byte[5],Byte[6],Byte[7]"));
}

// -------------------------------------------------------------
void setup(void)
{
  
  B_CAN.begin();
  Serial.begin(115200);
  
  pinMode(led, OUTPUT);
  digitalWrite(led, 1);

  delay(1000);
  
  // F() stores string in flash rather than RAM
  Serial.println(F("Teensy CAN Passthrough and Monitoring"));
  Serial.print(F("Initializing MCP2515 interface..."));

  // Attempt to initialize MCP2515 module
  while (CAN_OK != HU_CAN.begin(CAN_125KBPS))
  {
    // Loops forever - probably should implement some sort of timeout.  Watchdog?
    Serial.print(".");
    delay(100);
  }
  
  Serial.println(F("success"));
  digitalWrite(led, 0);

}


// -------------------------------------------------------------
void loop(void)
{
    // Check for message on car's B-CAN bus
    if( B_CAN.read(rxmsg) )
    {
      // Toggle our LED to show bus activity
      digitalWrite(led, 1);
      
      printFrame(rxmsg);
      
      // ID, CAN Mode: 1=extended, buffer length, buffer
      HU_CAN.sendMsgBuf(rxmsg.id, 1, rxmsg.len, rxmsg.buf);
    }

    // Check for message on Head Unit's CAN bus
    // This could be replaced with interrupt code and leverage the MCP2515 INT pin.
    if ( CAN_OK == HU_CAN.readMsgBufID( &(rxmsg.id), &(rxmsg.len), rxmsg.buf ) )
    {
      // Toggle LED for activity.  Use a different LED for head unit messages?
      digitalWrite(led, 1);

      printFrame(rxmsg);

      B_CAN.write(rxmsg);
    }

    // Activity LED toggle
    digitalWrite(led, 0);

    // Check for character to switch output format
    // Switches between tab- and comma-separated output
    if ( Serial.available() )
    {
      char c  = Serial.read();
      if ( c == 'c' )
      {
        sep = ( sep == ',' ) ? '\t' : ',';
      }
    }
    
}

