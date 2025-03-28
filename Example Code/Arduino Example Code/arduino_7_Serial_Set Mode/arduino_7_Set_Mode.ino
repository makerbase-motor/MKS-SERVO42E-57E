/**************** MKS SERVOxxE Close Loop Step Motor ***************
******************** arduino Example 7 ********************
**	Example Name：Set Mode
**  Example Purpose：Set motor's work mode through serial port
**  Example Phenomenon：After programe
** 1. The LED light is on, that is, the serial port sends a setting subdivision command
** 2. If the setting is successful, the LED light will flash slowly, and You can check the set working mode through the host computer "Read all parameters" button or serial port debugging assistant 47 command;
** 3. If the setting fails, the LED light will flash quickly
**  Precautions：
** 1. The serial port and the USB download port share the serial port (0,1). When uploading the program via USB, unplug the serial cable first to avoid program upload failure
** 2. Motor working mode is set to Pulse+Direction Closed-loop FOC mode or Pulse+Pulse Closed-loop FOC mode
** 3. If the program upload fails, you can try: press and hold the RESET button of UNO, and then click upload, and when the arduino displays "uploading", quickly release the button
* ** Source：
**  Github：https://github.com/makerbase-motor/MKS-SERVO42E-57E
**  Youtube Vedio：https://www.youtube.com/playlist?list=PLc2RScfrSFED5SgB08mZ3cSFRqxsY6OsM
**********************************************************/

uint8_t txBuffer[20];      //send data array
uint8_t rxBuffer[20];      //Receive data array
uint8_t rxCnt=0;          //Receive data count

uint8_t getCheckSum(uint8_t *buffer,uint8_t len);
void setMode(uint8_t slaveAddr,uint8_t Mode);
uint8_t waitingForACK(uint8_t len);


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);// Set the LED light port as output
  digitalWrite(LED_BUILTIN, HIGH); //Lights off

  // Start the serial port, set the rate to 38400
  Serial.begin(38400);
  //Wait for the serial port initialization to complete
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

// The power-on delay is 3000 milliseconds, waiting for the motor to be initialized
  delay(3000);
  }

void loop() {

  bool ackStatus;
  
  digitalWrite(LED_BUILTIN, HIGH); //light up
  setMode(1,5); //Slave address=1, mode=5 (RS485 bus Closed-loop FOC mode)

  ackStatus = waitingForACK(5);      //Wait for the motor to answer

  if(ackStatus == 1)        //successfully set
  {
    while(1)    //Slow flashing, indicating that the setting is successful (You can check the set working mode through the host computer "Read all parameters" button or serial port debugging assistant 47 command;)
    {
      digitalWrite(LED_BUILTIN, HIGH); delay(1000);    //Delay 1000 milliseconds
      digitalWrite(LED_BUILTIN, LOW);  delay(1000);    //Delay 1000 milliseconds
    }
  }
  else          //The setting failed (1. Check the connection of the serial cable; 2. Check whether the motor is powered on; 3. Check the slave address and baud rate)
  {
    while(1)   //Flashing light quickly, indicating that the setting failed
    {
      digitalWrite(LED_BUILTIN, HIGH);      delay(200);
      digitalWrite(LED_BUILTIN, LOW);      delay(200);
    }
  }
 
}

/*
Function: set working mode
Input: slaveAddr slave address
       Mode working mode
output: none
 */
void setMode(uint8_t slaveAddr,uint8_t Mode)
{
 
  txBuffer[0] = 0xFA;       //frame header
  txBuffer[1] = slaveAddr;  //slave address
  txBuffer[2] = 0x82;       //function code
  txBuffer[3] = Mode;       //Operating mode
  txBuffer[4] = getCheckSum(txBuffer,4);  //Calculate checksum
  Serial.write(txBuffer,5);   //The serial port issues commands
}

/*
Function: Calculate the checksum of a set of data
Input: buffer data to be verified
        size The number of data to be verified
output: checksum
*/
uint8_t getCheckSum(uint8_t *buffer,uint8_t size)
{
  uint8_t i;
  uint16_t sum=0;
  for(i=0;i<size;i++)
    {
      sum += buffer[i];  //Calculate accumulated value
    }
  return(sum&0xFF);     //return checksum
}

/*
Function: wait for the slave to answer, set the timeout to 3000ms
Input: len Length of the response frame
output:
   Run successfully 1
   failed to run 0
   timeout no reply 0
*/
uint8_t waitingForACK(uint8_t len)
{
  uint8_t retVal=0;       //return value
  unsigned long sTime;  //timing start time
  unsigned long time;  //current moment
  uint8_t rxByte;      

  sTime = millis();    //get the current moment
  rxCnt = 0;           //Receive count value set to 0
  while(1)
  {
    if (Serial.available() > 0)     //The serial port receives data
    {
      rxByte = Serial.read();       //read 1 byte data
      if(rxCnt != 0)
      {
        rxBuffer[rxCnt++] = rxByte; //Storing data
      }
      else if(rxByte == 0xFB)       //Determine whether the frame header
      {
        rxBuffer[rxCnt++] = rxByte;   //store frame header
      }
    }

    if(rxCnt == len)    //Receive complete
    {
      if(rxBuffer[len-1] == getCheckSum(rxBuffer,len-1))
      {
        retVal = rxBuffer[3];   //checksum correct
        break;                  //Exit while(1)
      }
      else
      {
        rxCnt = 0;  //Verification error, re-receive the response
      }
    }

    time = millis();
    if((time - sTime) > 3000)   //Judging whether to time out
    {
      retVal = 0;
      break;                    //timeout, exit while(1)
    }
  }
  return(retVal);
}
