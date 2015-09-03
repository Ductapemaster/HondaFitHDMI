// -------------------------------------------------------------
// can_passthrough_monitor for Teensy 3.1
// by ductapemaster
//
// CAN data is mirrored in both directions. Received frames are 
// printed to serial port for monitoring in either tab separated
// or comma separated values.  The character 'c' toggles between
// output formats.

#include <FlexCAN.h>
#include <teensy_mcp_can.h>
#include <spi4teensy3.h>

#define SERIAL_BAUD       115200
#define CANBUS_RATE       125000  //BCAN: 125kHz, FCAN: 500kHz

#define CANID_GEAR_POS    0x12F85150
#define CANID_SPEED_RPM   0x12F85050

/* States */
// Serial output - run/stop, CSV/TSV
// CAN Bridge - mirror/modify
// Monitor Filter - on/off
//  - Need to be able to set filter values

/* Mode Variables */
bool serialEnable = false;
bool decimalFormat = false;   // False = hex
bool lockoutDisable = true;   // Modifies CAN data to disable speed lockout and enable HDMI on Honda head unit

/* Reuseable Strings */
// F() stores string in flash rather than RAM
String strCSVHeader =   F("\n\rTime (ms),CAN Arbitration ID,Bytes,Byte[0],Byte[1],Byte[2],Byte[3],Byte[4],Byte[5],Byte[6],Byte[7]");
String strSuccess =     F("success");
String strGreeting =    F("2015 Honda Fit Head Unit CAN Bus Interrupter\n\r============================================\n\r");
                           
/* LED for rx indication */
int led = 14;  // Can't use onboard one because it shares the SPI SCLK pin

/* FlexCAN */
FlexCAN B_CAN(CANBUS_RATE);
static CAN_message_t rxmsg;

/* MCP2515 CAN */
// Interrupt is D2, from schematic
const int intPin = 2;
const int csPin = 10;     // MCP library requires CS pin as argument.  Use default
MCP_CAN HU_CAN(csPin);

/* Message Printing Functions */
// Serial output separator - default is tsv
char sep = '\t';

static void printFrame(CAN_message_t f)
{
      Serial.printf("%li%c%08X%c%i", millis(), sep, f.id, sep, f.len);
      
      for (int i = 0; i < f.len; i++) 
      {
        if (decimalFormat)
          Serial.printf("%c%03u", sep, f.buf[i]);
        else
          Serial.printf("%c%02X", sep, f.buf[i]);
      }
      
      Serial.println();
}

/* User Interface */
static void printMenu(void)
{
  Serial.println();
  Serial.println(strGreeting);
  Serial.println(F("Menu"));
  Serial.println(F("'s': Starts and stops serial monitoring of CAN data"));
  Serial.println(F("'c': Ouput CSV formatted data.  Also prints header string."));
  Serial.println(F("'t': Output human-readable data (tab-separated)"));
  Serial.println(F("'x': Display CAN payload data in hex"));
  Serial.println(F("'d': Display CAN payload data in decimal"));
  //Serial.println(F("'b': Display CAN payload data in binary"));   // Need to write binary converter for this to work
  //Serial.println(F("'f<CAN ID>': Filter data displayed on one input address. Example: f12F85150"));  // Not implemented yet!
  Serial.println(F("'m': Toggle modification of CAN data to disable speed lockout functions of Honda head unit (incl. HDMI)"));
  Serial.println(F("'h': Prints this menu\n\r\n\r"));
}

void processSerial(void)
{
    char c  = Serial.read();

    switch( c )
    {

      // CSV output
      case 'c':
        if (sep != ',')
        {
          sep = ',';
          Serial.println(strCSVHeader);
        }
        break;

      // TSV output
      case 't':
        if (sep != '\t')
          sep = '\t';
        break;

      // Hex payload
      case 'x':
        decimalFormat = false;
        break;

      // Decimal payload
      case 'd':
        decimalFormat = true;
        break;

      case 'm':
        lockoutDisable = !lockoutDisable;
        break;

      // Start/stop
      case 's':
        serialEnable = !serialEnable;
        break;

      // Print menu
      case 'h':
        printMenu();
        serialEnable = false;
        break;

      case 'b':
      
      case 'f':
      
      default:
        Serial.printf(F("Invalid Command: %c"), c);
        break;

    }
}

/* CAN Functions */
void lockoutDisableCAN(void) 
{
    // Modify data in buffer based on conditional data
    switch (rxmsg.id) {
  
      // gear = park; parking brake = on
      case CANID_GEAR_POS:
        rxmsg.buf[0] = 0x40;
        rxmsg.buf[1] = 0x03;
        //Serial.println(F("Gear payload set to 'park'"));
        break;
  
      // speed = 0
      case CANID_SPEED_RPM:
        rxmsg.buf[0] = 0x00;
        //Serial.println(F("Speed payload set to 0"));
        break;        
        
      default:
        break;
      
    }
  
}

// -------------------------------------------------------------
void setup(void)
{

  // Initialize LED and show we're booting
  pinMode(led, OUTPUT);
  digitalWrite(led, 1);

  pinMode(intPin, INPUT);

  Serial.begin(SERIAL_BAUD);
  Serial.println(strGreeting);

  // FlexCAN interface initialization
  // FlexCAN.begin() blocks with while loops during initialization.  No return value for success.
  Serial.println(F("Initializing FlexCAN Interface..."));
  B_CAN.begin();
  Serial.println(strSuccess);

  // Attempt to initialize MCP2515 module.  Tries 3 times before failing out.
  Serial.print(F("Initializing MCP2515 interface..."));
  int init_ctr = 0;
  
  while ( ( CAN_OK != HU_CAN.begin(CAN_125KBPS) ) & ( init_ctr < 3) )
  {
    // Try to initialize three times
    Serial.print(".");
    init_ctr++;
    delay(100);
  }

  if (init_ctr < 3)
  {
      Serial.println(strSuccess);
      Serial.println(F("Begin!"));
      digitalWrite(led, 0);
  } else {
      Serial.println(F("fail"));
      Serial.println(F("Operation halting."));
      while(true);
  }

  printMenu();

}


// -------------------------------------------------------------
void loop(void)
{
    // Check for message on car's B-CAN bus
    if( B_CAN.read(rxmsg) )
    {
      // Toggle our LED to show bus activity
      digitalWrite(led, 1);

      if (lockoutDisable)
        lockoutDisableCAN();

      if (serialEnable) 
        printFrame(rxmsg);

      // ID, CAN Mode: 1=extended, buffer length, buffer
      HU_CAN.sendMsgBuf(rxmsg.id, 1, rxmsg.len, rxmsg.buf);
    }

    // Check for message on Head Unit's CAN bus
    if ( digitalRead(intPin) == LOW )
    {

      HU_CAN.readMsgBufID( &(rxmsg.id), &(rxmsg.len), rxmsg.buf );
      
      // Toggle LED for activity.  Use a different LED for head unit messages?
      digitalWrite(led, 1);

      if (serialEnable) 
        printFrame(rxmsg);

      B_CAN.write(rxmsg);
    }

    // Activity LED toggle
    digitalWrite(led, 0);

    if ( Serial.available() )
      processSerial();
    
}

