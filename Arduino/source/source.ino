#include <Wire.h>
#include <avr/wdt.h> //for watchdog
#include "MovingAverageFilter.h"
#include "MemoryFree.h"

#define PUTTY_DISABLE //disable putty handler

#define IRDY                2 //i2c ready pin - indicates to master, in this case the arduino, that the iqs is ready for data transmittion. 
#define I2CA0               4
#define MCLR                7


// potentiometer wiper (middle terminal) connected to analog pin 0
#define PRESSURE_AN_PIN A0
#define V_S 5 //voltage source

//Defines for watchdog timer
#define WDTO_15MS 0
#define WDTO_30MS 1 
#define WDTO_1S 6 
#define WDTO_4S 8 
#define WDTO_8S 9 

//Defines for UNO-specifics
#define UNO_MAX_INT 32767
#define UNO_MIN_INT -32768

//Enabler 
#define DEBUG_EN 0
#define CUR_SAM_EN 1  //Enables polling of current samples
#define LTA_SAM_EN 1  //Enables polling of LTA samples

//Refer to IQS316 Design Guide for what each register does ie. azd032_iqs316_communication_interface.pdf
// Addresses in the IQS316Memory Map
#define PROD_NUM 0x00
#define VERSION_NUM 0x01
#define UI_FLAGS0 0x10
#define PROX_STAT 0x31
#define TOUCH_STAT 0x35
#define HALT_STAT 0x39
#define GROUP_NUM 0x3D
#define CUR_SAM_04_HI 0x42
#define CUR_SAM_04_LO 0x43
#define CUR_SAM_15_HI 0x44
#define CUR_SAM_15_LO 0x45
#define CUR_SAM_26_HI 0x46
#define CUR_SAM_26_LO 0x47
#define CUR_SAM_37_HI 0x48
#define CUR_SAM_37_LO 0x49
#define LTA_04_HI 0x83
#define LTA_04_LO 0x84
#define LTA_15_HI 0x85
#define LTA_15_LO 0x86
#define LTA_26_HI 0x87
#define LTA_26_LO 0x88
#define LTA_37_HI 0x89
#define LTA_37_LO 0x8A
#define UI_SETTINGS0 0xC4
#define POWER_SETTINGS 0xC5
#define PROX_SETTINGS_1 0xC6
#define PROX_SETTINGS_2 0xC7
#define PROX_CFG1 0xC8
#define CMT_SETTINGS 0xC9
#define ATIC0 0xCA
#define ATIC1 0xCB
#define ATIC2 0xCC
#define ATIC3 0xCD
#define SHLD_SETTINGS 0xCE
#define INT_CAL_SETTINGS 0xCF
#define PM_CX_SELECT 0xD0
#define DEFAULT_COMMS_PTR 0xD1
#define CHAN_ACTIVE0 0xD2
#define CHAN_ACTIVE1 0xD3
#define CHAN_ACTIVE2 0xD4
#define CHAN_ACTIVE3 0xD5
#define CHAN_ACTIVE4 0xD6
#define CHAN_RESEED0 0xD7
#define CHAN_RESEED1 0xD8
#define CHAN_RESEED2 0xD9
#define CHAN_RESEED3 0xDA
#define CHAN_RESEED4 0xDB
#define AUTO_ATI_TARGET_HI 0xDC
#define AUTO_ATI_TARGET_LO 0xDD
#define DIRECT_ADDR_RW 0xFC
#define DIRECT_DATA_RW 0xFD

#define IQS316_ADDRESS    0x74
#define PROX_MODE_ATI 0
#define TOUCH_MODE_ATI 1

//#define GUI_PRINT            // toggle GUI and DEBUG printing

struct {
  byte prox_detected;   //5 bytes
  byte prox4_11;
  byte prox12_19;
  byte touch4_11;
  byte touch12_19;
  
  //From here
  int samples[16]; //int for uno is 16-bit (2 bytes) *16 samples => 32 bytes
  int LTA_samples[16];  

  int sample_differences[16]; // LTA - current samples
  int filtered_samples[16]; //Samples fter applying MA_Filter

  //for calibration
  int channel_min[16]; //min number for that sensor channel
  int channel_max[16]; //max number for that sensor channel
  
  int scaled[16];

  int channel_calibration_val[5];   
  //to here uses 32 * 8 = 256 bytes

  boolean attached[5];
  boolean proper_attach;

} IQS316;
 
#define DATA_FLAGS 0x01 //Sent serially as start byte indication
#define SCALE_MASK 0x02 //OR with DATA_FLAGS to indicate scaled_data to GUI
#define PROPERATTACH_MASK 0x04 //Bit Flag for IQS316.proper_attached
#define ATTACH_0_MASK 0x08 //Bits flags for when IQS316.attached[i] is TRUE
#define ATTACH_1_MASK 0x10 
#define ATTACH_2_MASK 0x20
#define ATTACH_3_MASK 0x40
#define ATTACH_4_MASK 0x80

boolean scale = false;
boolean LTA = false;
boolean printRange = false;
boolean filter_print = false;

#define FILTER_SIZE 8
int filter_buffer[16][FILTER_SIZE];  // 32 bytes * 10 = 320 bytes 

#define PRESSURE_FILTER_SIZE 5
int pressure_filter_buffer[PRESSURE_FILTER_SIZE];  // 32 bytes * 10 = 320 bytes 

float kPA_Pressure = 0; //Pressure in kPA   //float uses 4 bytes in uno

//////////////////////////////////////
// Arduino's Routine code /////////// 
////////////////////////////////////

//run once when microcontroller starts
void setup(){
  
  
  pins_init();    //initialize pins
  init_globals(); //Initializes global variables

  resetIQS(); //reset the IQS316
  
  Serial.begin(115200);// initialize UART
  
  #ifndef PUTTY_DISABLE
  Serial.println(F("Done initializing UART..."));
  #endif


  Wire.begin();

  #ifndef PUTTY_DISABLE
  Serial.println(F("Done initializing I2C Library..."));
  #endif

  //Wait until product number is read, device is ready if product number is retrieved from register
  
  #ifndef PUTTY_DISABLE 
  Serial.println(F("Checking for IQS316 connection... "));
  #endif

  while(readRegister(PROD_NUM)!=27)
    delay(20);

  #ifndef PUTTY_DISABLE
  Serial.println(F("IQS316 connection established."));
  #endif

  IQS316_Settings_Init();

}


//Initializes global variables
void init_globals()
{
    for (unsigned char i = 0; i < 16; i++) {
      IQS316.channel_min[i] = UNO_MAX_INT; // set to highest possible value  
      IQS316.channel_max[i] = UNO_MIN_INT; // set to lowest possible value 

      IQS316.samples[i] = 0;
      IQS316.LTA_samples[i] = 0;

      clearBuf(filter_buffer[i], FILTER_SIZE);

    } //might varies depending on platform.  

    clearBuf(pressure_filter_buffer, PRESSURE_FILTER_SIZE);

}

void loop(){

  IQS316_New_Conversion(); //Polls data from IQS316
  loadPressure_data(); // Polls data from Pressure Sensor

  if (IQS316.prox_detected) {
      
      ApplyMAFilter(); //Apply Moving average filters to IQS316 data
      checkTreshHold();

      #ifndef PUTTY_DISABLE
      putty_Handler();
      Serial.print("freeMemory()=");
      Serial.println(freeMemory());
      #endif


      #ifdef PUTTY_DISABLE
      sendSample2GUI();
      #endif
  }
}


void putty_Handler(void)
{
     clearTerminal();
     if (scale) { 
       if (!printRange){
        printScaled();
       } else {
        printMinMax();
       }
      } else { 
        if (LTA) {
          printLTASamples();
        }
        else{
          if (filter_print){
            printFilterSamples();
          }else{
            printData2();
          }
        }
      }
}


//clearTerminal()
//Description : Clears the terminal
void clearTerminal(void)
{
      Serial.write(27);       // ESC command
      Serial.print("[2J");    // clear screen command
      Serial.write(27);
      Serial.print("[H");     // cursor to home command
}

void printData(void)
{

      Serial.print("Sensor1 - CH 4: "); Serial.println(IQS316.sample_differences[0]); 
      Serial.print("Sensor2 - CH 5: "); Serial.println(IQS316.sample_differences[1]);
      Serial.print("Sensor3 - CH 6: "); Serial.println(IQS316.sample_differences[2]);
      Serial.print("Sensor4 - CH 7: "); Serial.println(IQS316.sample_differences[3]);
      Serial.print("Sensor5 - CH 8: "); Serial.println(IQS316.sample_differences[4]);
      Serial.print("Sensor6 - CH 9: "); Serial.println(IQS316.sample_differences[5]);
      Serial.print("Sensor7 - CH 10: "); Serial.println(IQS316.sample_differences[6]);
      Serial.print("Sensor8 - CH 11: "); Serial.println(IQS316.sample_differences[7]);
      Serial.print("Sensor9 - CH 12: "); Serial.println(IQS316.sample_differences[8]);
      Serial.print("Sensor10 - CH 13: "); Serial.println(IQS316.sample_differences[9]);
      Serial.print("Sensor11 - CH 14: "); Serial.println(IQS316.sample_differences[10]);
      Serial.print("Sensor12 - CH 15: "); Serial.println(IQS316.sample_differences[11]);
      Serial.print("Sensor13 - CH 16: "); Serial.println(IQS316.sample_differences[12]);
      Serial.print("Sensor14 - CH 17: "); Serial.println(IQS316.sample_differences[13]);
      Serial.print("Sensor15 - CH 18: "); Serial.println(IQS316.sample_differences[14]);
      Serial.print("Sensor16 - CH 19: "); Serial.println(IQS316.sample_differences[15]);
}

void printData2(void)
{

      int temp;
      const int max = 500;

      for (unsigned char i = 0; i < 16; i++ )
      {
          temp = IQS316.sample_differences[i];
          int p = (temp*10/max);

          Serial.print("Sensor "); Serial.print(i); Serial.print("\t"); Serial.print(temp); Serial.print("\t : ");
          for (unsigned char j = 0; j < p; j++) { Serial.print("#");}
          Serial.println("");
      }

      Serial.println("Unscaled data. Press 's' to view scaled data. press 't' to view the Long Term Average(LTA).");
}

void printFilterSamples(void)
{
      int temp;
      const int max = 500;

      for (unsigned char i = 0; i < 16; i++ )
      {
          temp = IQS316.filtered_samples[i];
          int p = (temp*10/max);

          Serial.print("Sensor "); Serial.print(i); Serial.print("\t"); Serial.print(temp); Serial.print("\t : ");
          for (unsigned char j = 0; j < p; j++) { Serial.print("#");}
          Serial.println("");
      }
}

void printScaled(void)
{

      int temp;
      const int max = 1000;

      for (unsigned char i = 0; i < 16; i++ )
      {
          temp = IQS316.scaled[i];
          
          unsigned char p = (temp*50/max);

          Serial.print("Sensor "); Serial.print(i); Serial.print("\t"); Serial.print(temp); Serial.print("\t : ");
          for (unsigned char j = 0; j < p; j++) { Serial.print("#");}
          Serial.println("");
      }

    Serial.println("Data are scaled to 1000. Press '1' to view the original range. press 's' to go back.");
}


void printMinMax(void)
{

      for (unsigned char i = 0; i < 16; i++ )
      {

          Serial.print("Sensor "); Serial.print(i); Serial.print("\t MIN : "); Serial.print(IQS316.channel_min[i]); Serial.print("\t MAX : ");
          Serial.print(IQS316.channel_max[i]); Serial.println("");
      }

}

void printCurrentSamples(void)
{
      Serial.print("CH 4: "); Serial.println(IQS316.samples[0]); 
      Serial.print("CH 5: "); Serial.println(IQS316.samples[1]);
      Serial.print("CH 6: "); Serial.println(IQS316.samples[2]);
      Serial.print("CH 7: "); Serial.println(IQS316.samples[3]);
      Serial.print("CH 8: "); Serial.println(IQS316.samples[4]);
      Serial.print("CH 9: "); Serial.println(IQS316.samples[5]);
      Serial.print("CH 10: "); Serial.println(IQS316.samples[6]);
      Serial.print("CH 11: "); Serial.println(IQS316.samples[7]);
      Serial.print("CH 12: "); Serial.println(IQS316.samples[8]);
      Serial.print("CH 13: "); Serial.println(IQS316.samples[9]);
      Serial.print("CH 14: "); Serial.println(IQS316.samples[10]);
      Serial.print("CH 15: "); Serial.println(IQS316.samples[11]);
      Serial.print("CH 16: "); Serial.println(IQS316.samples[12]);
      Serial.print("CH 17: "); Serial.println(IQS316.samples[13]);
      Serial.print("CH 18: "); Serial.println(IQS316.samples[14]);
      Serial.print("CH 19: "); Serial.println(IQS316.samples[15]);
}

void printLTASamples(void)
{
      Serial.print("CH 4: "); Serial.println(IQS316.LTA_samples[0]); 
      Serial.print("CH 5: "); Serial.println(IQS316.LTA_samples[1]);
      Serial.print("CH 6: "); Serial.println(IQS316.LTA_samples[2]);
      Serial.print("CH 7: "); Serial.println(IQS316.LTA_samples[3]);
      Serial.print("CH 8: "); Serial.println(IQS316.LTA_samples[4]);
      Serial.print("CH 9: "); Serial.println(IQS316.LTA_samples[5]);
      Serial.print("CH 10: "); Serial.println(IQS316.LTA_samples[6]);
      Serial.print("CH 11: "); Serial.println(IQS316.LTA_samples[7]);
      Serial.print("CH 12: "); Serial.println(IQS316.LTA_samples[8]);
      Serial.print("CH 13: "); Serial.println(IQS316.LTA_samples[9]);
      Serial.print("CH 14: "); Serial.println(IQS316.LTA_samples[10]);
      Serial.print("CH 15: "); Serial.println(IQS316.LTA_samples[11]);
      Serial.print("CH 16: "); Serial.println(IQS316.LTA_samples[12]);
      Serial.print("CH 17: "); Serial.println(IQS316.LTA_samples[13]);
      Serial.print("CH 18: "); Serial.println(IQS316.LTA_samples[14]);
      Serial.print("CH 19: "); Serial.println(IQS316.LTA_samples[15]);
}

void printProxStat(void)
{
  Serial.println(IQS316.prox4_11, BIN);
  Serial.println(IQS316.prox12_19, BIN);
}

void printTouchStat(void)
{
  Serial.println(IQS316.touch4_11, BIN);
  Serial.println(IQS316.touch12_19, BIN);
}


//SerialEvent Handler
void serialEvent(){
  byte a;
  while (Serial.available()){
    a = Serial.read();

    //Gui Handler
    if ( a == 0x27) {
      scale = !scale;
    } else if ( a == 0x26){
      Calibrate2Face();
    } 

    #ifndef PUTTY_DISABLE
      //Putty handler
      else if ( a == 's'){
      scale = !scale;
    } else if ( a == 't'){
      if (!scale) { LTA = !LTA;}
    } else if ( a == '1'){
      if (scale) { printRange = !printRange; }
    } else if ( a == 'f'){
      if (!scale) { filter_print = !filter_print;}
    }
    #endif

  }


}

//sendSample2GUI
//Description : Send samples in highbyte and low byte
void sendSample2GUI(){
  
 
  byte buf[1];
  buf[0] = DATA_FLAGS;
  
  if (scale){
    buf[0] = buf[0] | SCALE_MASK; //Apply mask
  } 

  if (IQS316.proper_attach) {
    buf[0] = buf[0] | PROPERATTACH_MASK;
  }

  if (IQS316.attached[0]){
    buf[0] = buf[0] | ATTACH_0_MASK;
  }

  if (IQS316.attached[1]){
    buf[0] = buf[0] | ATTACH_1_MASK;
  }

  if (IQS316.attached[2]){
    buf[0] = buf[0] | ATTACH_2_MASK;
  }

  if (IQS316.attached[3]){
    buf[0] = buf[0] | ATTACH_3_MASK;
  }

  if (IQS316.attached[4]){
    buf[0] = buf[0] | ATTACH_4_MASK;
  }

  Serial.write(buf,1); 


  //send IQS316 samples
  //send 16 samples of int (2 bytes each) => 32 bytes

  //make a buffer to store 2 byte for each int
  for (unsigned char i = 0; i < 16; i++){
    int num2convert;

    if (!scale){
      num2convert = IQS316.sample_differences[i];
    } else { num2convert = IQS316.scaled[i];}

    byte buf[2];
    //parse into high and low byte
    buf[0] = ((num2convert & 0xFF00) >> 8); //HI_BYTE
    buf[1] = num2convert & 0x00FF; // LO_BYTE

    Serial.write(buf, 2);
  }

  //send pressure sensor sample 4 bytes in total
  byte float_buf[4];

  float2Bytes(kPA_Pressure, float_buf);

  //send from hi byte to lo byte
  byte invert[4];

  for (unsigned char i = 0; i < 4; i++){
    invert[i] = float_buf[4 - i - 1];
  }
  Serial.write(invert,4);

  //sent 1 + 32 + 4 bytes = 37 bytes in total
}

/***********************************************************************/
///////////////////////////////////////////////////////////////
//HIGH LEVEL FUNCTIONS for Capacitive Sensor Data Processing //
//////////////////////////////////////////////////////////////


//Calibrate
//Description : Calibrate for proper mask attachment
//              Uses
//              
void Calibrate2Face(void){

    memset(IQS316.channel_calibration_val, 0, sizeof(int) * 5); //clears the array

  //Calculate the average for Group 0 (Top Upper Right) sensor : [0] [1] [3]
  IQS316.channel_calibration_val[0] += IQS316.filtered_samples[0];
  IQS316.channel_calibration_val[0] += IQS316.filtered_samples[1];
  IQS316.channel_calibration_val[0] += IQS316.filtered_samples[3];
  IQS316.channel_calibration_val[0] = IQS316.channel_calibration_val[0] / 3;
  IQS316.channel_calibration_val[0] = IQS316.channel_calibration_val[0]*4 / 5; //takes the 80% as the threshold

  //Calculate the average for Group 1 (Top Lower Right) sensor : [2] [5] [4]
  IQS316.channel_calibration_val[1] += IQS316.filtered_samples[2];
  IQS316.channel_calibration_val[1] += IQS316.filtered_samples[4];
  IQS316.channel_calibration_val[1] += IQS316.filtered_samples[5];
  IQS316.channel_calibration_val[1] = IQS316.channel_calibration_val[1] / 3;
  IQS316.channel_calibration_val[1] = IQS316.channel_calibration_val[1]*4 / 5;

  //Calculate the average for Group 2 (Top Upper Left) sensor : [8] [10] [11]
  IQS316.channel_calibration_val[2] += IQS316.filtered_samples[8];
  IQS316.channel_calibration_val[2] += IQS316.filtered_samples[10];
  IQS316.channel_calibration_val[2] += IQS316.filtered_samples[11];
  IQS316.channel_calibration_val[2] = IQS316.channel_calibration_val[2] / 3;
  IQS316.channel_calibration_val[2] = IQS316.channel_calibration_val[2]*4 / 5;

  //Calculate the average for Group 3 (Top Lower Left) sensor : [6] [7] [9]
  IQS316.channel_calibration_val[3] += IQS316.filtered_samples[6];
  IQS316.channel_calibration_val[3] += IQS316.filtered_samples[7];
  IQS316.channel_calibration_val[3] += IQS316.filtered_samples[9];
  IQS316.channel_calibration_val[3] = IQS316.channel_calibration_val[2] / 3;
  IQS316.channel_calibration_val[3] = IQS316.channel_calibration_val[3]*4 / 5;

  //Calculate the average for Group 4 (Bottom) sensor : [12] [13] [14] [15]
  IQS316.channel_calibration_val[4] += IQS316.filtered_samples[12];
  IQS316.channel_calibration_val[4] += IQS316.filtered_samples[13];
  IQS316.channel_calibration_val[4] += IQS316.filtered_samples[14];
  IQS316.channel_calibration_val[4] += IQS316.filtered_samples[15];
  IQS316.channel_calibration_val[4] = IQS316.channel_calibration_val[4] / 4;
  IQS316.channel_calibration_val[4] = IQS316.channel_calibration_val[4]*4 / 5;

}

//Split sensors into various group and find their average
void checkTreshHold(void){

  int average[5] = {0,0,0,0,0};

  //Calculate the average for Group 0 (Top Upper Right) sensor : [0] [1] [3]
  average[0] += IQS316.filtered_samples[0];
  average[0] += IQS316.filtered_samples[1];
  average[0] += IQS316.filtered_samples[3];
  average[0] = average[0] / 3;

  //Calculate the average for Group 1 (Top Lower Right) sensor : [2] [5] [4]
  average[1] += IQS316.filtered_samples[2];
  average[1] += IQS316.filtered_samples[4];
  average[1] += IQS316.filtered_samples[5];
  average[1] = average[1] / 3;

  //Calculate the average for Group 2 (Top Upper Left) sensor : [8] [10] [11]
  average[2] += IQS316.filtered_samples[8];
  average[2] += IQS316.filtered_samples[10];
  average[2] += IQS316.filtered_samples[11];
  average[2] = average[2] / 3;

  //Calculate the average for Group 3 (Top Lower Left) sensor : [6] [7] [9]
  average[3] += IQS316.filtered_samples[6];
  average[3] += IQS316.filtered_samples[7];
  average[3] += IQS316.filtered_samples[9];
  average[3] = average[2] / 3;

  //Calculate the average for Group 4 (Bottom) sensor : [12] [13] [14] [15]
  average[4] += IQS316.filtered_samples[12];
  average[4] += IQS316.filtered_samples[13];
  average[4] += IQS316.filtered_samples[14];
  average[4] += IQS316.filtered_samples[15];
  average[4] = average[4] / 4;

  for (unsigned char i = 0; i < 5; i++){
    IQS316.attached[i] = average[i] >= IQS316.channel_calibration_val[i];
  }

  IQS316.proper_attach = IQS316.attached[0] && IQS316.attached[1] && IQS316.attached[2] &&
               IQS316.attached[3] && IQS316.attached[4];  
}


//Applies Moving Average Filter to each of the sensor channels
void ApplyMAFilter(void){
  for (int i = 0; i < 16; i++){
    IQS316.filtered_samples[i] = MA_Filter_current(filter_buffer[i], IQS316.sample_differences[i], FILTER_SIZE);
  }

}


/***********************************************************************/
//////////////////////////////////////////////////////
//HIGH LEVEL FUNCTIONS for Pressure Sensor Handling //
/////////////////////////////////////////////////////

//Description: Load Pressure data to the global variable 
void loadPressure_data(void){
  int nval_Pressure = 0;
  nval_Pressure = analogRead(PRESSURE_AN_PIN);

  int filtered_Pressure_nval = MA_Filter_current(pressure_filter_buffer,nval_Pressure,PRESSURE_FILTER_SIZE);
  kPA_Pressure = convert_digital_to_kPA(filtered_Pressure_nval);
}


//Description: Convert digital value to kiloPascal
float convert_digital_to_kPA(int nval_Pressure){
  
  float volt_Pressure  = V_S * (float)nval_Pressure/1023;  // conversion to voltage reading
  return volt_Pressure - 1;   // pressure conversion in kPa
}

//convert float2bytes data
//credits to Tyler Durden from stackoverflow.com
//http://stackoverflow.com/questions/24420246/c-function-to-convert-float-to-byte-array
void float2Bytes(float val,byte* bytes_array){
  // Create union of shared memory space
  union {
    float float_variable;
    byte temp_array[4];
  } u;
  // Overite bytes of union with float variable
  u.float_variable = val;
  // Assign bytes to input array
  memcpy(bytes_array, u.temp_array, 4);
}

/***********************************************************************/


/////////////////////////////////////////////////
//HIGH LEVEL FUNCTIONS to configure IQS316 //////
////////////////////////////////////////////////
//pins_init()
//Description : Initializes the arduino hardware and configures the pin
void pins_init(void)
{
  pinMode(IRDY, INPUT);     //set the pin to input
  digitalWrite(IRDY, HIGH); //use internal pullups
  pinMode(6, OUTPUT);
  pinMode(I2CA0,OUTPUT);
  pinMode(MCLR,OUTPUT);
  
  digitalWrite(I2CA0,0);
  digitalWrite(MCLR,1);
}


//IQS316_Settings_Init()
//Description : Initializes and configures IQS316
void IQS316_Settings_Init(void){
  
  #ifndef PUTTY_DISABLE
  Serial.println(F("Initializing IQS316..."));
  #endif

  byte start_num, temp;
  
  //read group number
  start_num = readRegister(GROUP_NUM, false);
  
  #ifndef PUTTY_DISABLE
  Serial.print(F("Start group number read... -> ")); Serial.println(start_num);
  Serial.println(F("Setting CONV_SKIP"));
  #endif

  //set the CONV_SKIP bit
  writeRegister(PROX_SETTINGS_2, 0x09, false);
  
  temp = start_num;
  
  do
  {
      switch(temp)
      {
          case(0): Clear_LTA(); break;
          case(1): Clear_LTA(); break;
          case(2): Clear_LTA(); break;
          case(3): Clear_LTA(); break;
          case(4): Clear_LTA(); break;
      }
            
      temp = readRegister(GROUP_NUM, false);
      
      #ifndef PUTTY_DISABLE
      Serial.print("Next group number read... -> "); Serial.print(temp);
      #endif

  } while (temp != start_num); //ensuer all groups have been set.
  
  #ifndef PUTTY_DISABLE
  Serial.println("");
  Serial.println(F("Clearing CONV_SKIP"));
  #endif

  //clear CONV_SKIP bit
  writeRegister(PROX_SETTINGS_2, 0x01);
  
  //Read anything to initiate initial conversion and allow system to settle
  for (int i = 0; i <= 23; i++)
  { 
     temp = readRegister(PROD_NUM); 
  }
  
  #ifndef PUTTY_DISABLE
  Serial.println(F("Configuring PM_CX_SELECT and UI_SETTINGS0"));
  #endif

  writeRegister(PM_CX_SELECT,0x0F); //Set all channels to have prox mode activated
  writeRegister(UI_SETTINGS0,0xE6); //Configures and Initiate a conversion 1010 0110
  writeRegister(PROX_SETTINGS_2,0x41); //Configures and Initiate a conversion 1010 0110
  //Auto_ATI_routine(800,PROX_MODE_ATI); //Start ATI Routine for prox mode
  Auto_ATI_routine(2048,TOUCH_MODE_ATI); //Start ATI Routine for touch mode

  #ifndef PUTTY_DISABLE
  Serial.println(F("Done Initializing IQS316."));
  #endif

}


//Clears LTA Register for the current active group
void Clear_LTA()
{
      writeRegister(LTA_04_HI, 0x00, false);
      writeRegister(LTA_15_HI, 0x00, false);
      writeRegister(LTA_26_HI, 0x00, false);
      writeRegister(LTA_37_HI, 0x00); //send stop bit to initiate conversion
}


//Auto_ATI_routine()
//Description : Start the routine for AUTO-ATI
//Parameters : touch_target - count target,
//             target_mode - PROX_MODE_ATI OR TOUCH_MODE_ATI    (determines which mode to perform ATI on)
void Auto_ATI_routine(int touch_target, boolean target_mode)
{
    #ifndef PUTTY_DISABLE
    Serial.println("Starting AUTO ATI Routine...");
    #endif

    int target = touch_target;
    byte hi_byte = (target & 0xFF00) >> 8;
    byte lo_byte = target & 0x00FF;
    boolean ati_busy;

    //set auto ATI Target to 400
    writeRegister(AUTO_ATI_TARGET_HI, hi_byte, false);
    writeRegister(AUTO_ATI_TARGET_LO, lo_byte, false);

    //set Touch Mode for ATI by setting ATI_MODE bit (UI_SETTINGS0<6> = 1)
    byte temp = readRegister(UI_SETTINGS0, false);
    
    switch(target_mode)
    {
      case 0 :  writeRegister(UI_SETTINGS0, temp & 0xBF, false); break;//clear bit 6 
      case 1 :  writeRegister(UI_SETTINGS0, temp | 0x40, false); break;//set bit 6
    }

    //Start Auto-ATI procedure by setting AUTO-ATI bit (PROX_SETTINGS<3>)
    temp = readRegister(PROX_SETTINGS_1, false);
    writeRegister(PROX_SETTINGS_1,  temp | 0x08); 

    #ifndef PUTTY_DISABLE
    Serial.println("Waiting for AUTO ATI Routine to finish...");
    #endif

    do{
      temp = readRegister(UI_FLAGS0); //read UI_FLAGS0
      ati_busy = (temp & 0x04); //get bit 2 information

      //Serial.println(ati_busy,HEX); //for debugging

    } while (ati_busy == 0x04);

    #ifndef PUTTY_DISABLE
    Serial.println("AUTO ATI Routine completed.");
    #endif
}


//IQS316_New_Conversion
//Retrieves data from IQS316. Ported and modified from the IQS316 Communication Interface.pdf for I2C communications - Saw
//Return : none
void IQS316_New_Conversion(void){
    byte temp_num, start_num, temp_touch, temp_prox = 0;
    byte temp_samples[8];
    byte LTA_temp_samples[8];
    
    //initiate a conversion - conversion is initiated when stop bit is send
    //Serial.println(F("Checking for IQS316 connection... "));
    while(readRegister(PROD_NUM)!=27)
      delay(10);
   //Serial.println(F("IQS316 connection established."));
    
    
    //read group number 
    temp_num = readRegister(GROUP_NUM,false);
    //Serial.println("Retrieving data. Currently at group "); Serial.println(temp_num);
    if (temp_num == 0)
    {
        IQS316.prox_detected = 0; //clear flag
        
    }
    else
    {
        IQS316.prox_detected = 1; //set flag
        start_num = temp_num;
        
        do
        {
            
            //gets the current sample
            if (CUR_SAM_EN) {
              temp_samples[0] = readRegister(CUR_SAM_04_HI, false); //do not send a stop bit to performs repeated read
              temp_samples[1] = readRegister(CUR_SAM_04_LO, false);
              temp_samples[2] = readRegister(CUR_SAM_15_HI, false);
              temp_samples[3] = readRegister(CUR_SAM_15_LO, false);
              temp_samples[4] = readRegister(CUR_SAM_26_HI, false);
              temp_samples[5] = readRegister(CUR_SAM_26_LO, false);
              temp_samples[6] = readRegister(CUR_SAM_37_HI, false);
              temp_samples[7] = readRegister(CUR_SAM_37_LO, false);
            }
            
            if (LTA_SAM_EN) {  
            LTA_temp_samples[0] = readRegister(LTA_04_HI, false);
            LTA_temp_samples[1] = readRegister(LTA_04_LO, false);
            LTA_temp_samples[2] = readRegister(LTA_15_HI, false);
            LTA_temp_samples[3] = readRegister(LTA_15_LO, false);
            LTA_temp_samples[4] = readRegister(LTA_26_HI, false);
            LTA_temp_samples[5] = readRegister(LTA_26_LO, false);
            LTA_temp_samples[6] = readRegister(LTA_37_HI, false);
            LTA_temp_samples[7] = readRegister(LTA_37_LO, false);
            }

            //Serial.println("Retrieving TOUCH_STAT and PROX_STAT...");
            temp_touch = readRegister(TOUCH_STAT, false); 
            temp_prox = readRegister(PROX_STAT, true); //send a stop bit

            switch(temp_num)
            {
            case 1:
            IQS316.prox4_11 = IQS316.prox4_11 & 0xF0;
            IQS316.touch4_11 = IQS316.touch4_11 & 0xF0;
            IQS316.prox4_11 = IQS316.prox4_11 | (temp_prox & 0x0F);
            IQS316.touch4_11 = IQS316.touch4_11 | (temp_touch & 0x0F);
            
            if (CUR_SAM_EN) {
              //concatenate the two bytes to yield 1 sample each,  for a total of 4 samples per group
              IQS316.samples[0] = ((temp_samples[0] & 0x00FF) << 8); 
              IQS316.samples[0] = (IQS316.samples[0] |  (temp_samples[1] & 0x00FF));
              
              IQS316.samples[1] = ((temp_samples[2] & 0x00FF) << 8); 
              IQS316.samples[1] = (IQS316.samples[1] |  (temp_samples[3] & 0x00FF));
             
              IQS316.samples[2] = ((temp_samples[4] & 0x00FF) << 8); 
              IQS316.samples[2] = (IQS316.samples[2] |  (temp_samples[5] & 0x00FF)); 
             
              IQS316.samples[3] = ((temp_samples[6] & 0x00FF) << 8); 
              IQS316.samples[3] = (IQS316.samples[3] |  (temp_samples[7] & 0x00FF)); 
            }

            if (LTA_SAM_EN) {
              IQS316.LTA_samples[0] = ((LTA_temp_samples[0] & 0x000F) << 8); //retrieve the first 4 bit
              IQS316.LTA_samples[0] = (IQS316.LTA_samples[0] |  (LTA_temp_samples[1] & 0x00FF)); //concatenate with the lower byte
              
              IQS316.LTA_samples[1] = ((LTA_temp_samples[2] & 0x000F) << 8); 
              IQS316.LTA_samples[1] = (IQS316.LTA_samples[1] |  (LTA_temp_samples[3] & 0x00FF));
             
              IQS316.LTA_samples[2] = ((LTA_temp_samples[4] & 0x000F) << 8); 
              IQS316.LTA_samples[2] = (IQS316.LTA_samples[2] |  (LTA_temp_samples[5] & 0x00FF)); 
             
              IQS316.LTA_samples[3] = ((LTA_temp_samples[6] & 0x000F) << 8); 
              IQS316.LTA_samples[3] = (IQS316.LTA_samples[3] |  (LTA_temp_samples[7] & 0x00FF)); 

            }

      
            break;
            case 2:
            IQS316.prox4_11 = IQS316.prox4_11 & 0x0F;
            IQS316.touch4_11 = IQS316.touch4_11 & 0x0F;
            IQS316.prox4_11 = IQS316.prox4_11 | ((temp_prox & 0x0F) << 4);
            IQS316.touch4_11 = IQS316.touch4_11 | ((temp_touch & 0x0F) <<4);
            
            if (CUR_SAM_EN) {            
              //concatenate the two bytes to yield 1 sample each,  for a total of 4 samples per group            
              IQS316.samples[4] = ((temp_samples[0] & 0x00FF) << 8); 
              IQS316.samples[4] = (IQS316.samples[4] |  (temp_samples[1] & 0x00FF));
              
              IQS316.samples[5] = ((temp_samples[2] & 0x00FF) << 8); 
              IQS316.samples[5] = (IQS316.samples[5] |  (temp_samples[3] & 0x00FF));
             
              IQS316.samples[6] = ((temp_samples[4] & 0x00FF) << 8); 
              IQS316.samples[6] = (IQS316.samples[6] |  (temp_samples[5] & 0x00FF)); 
             
              IQS316.samples[7] = ((temp_samples[6] & 0x00FF) << 8); 
              IQS316.samples[7] = (IQS316.samples[7] |  (temp_samples[7] & 0x00FF));       
              }      

            if (LTA_SAM_EN) {
              IQS316.LTA_samples[4] = ((LTA_temp_samples[0] & 0x000F) << 8); //retrieve the first 4 bit
              IQS316.LTA_samples[4] = (IQS316.LTA_samples[4] |  (LTA_temp_samples[1] & 0x00FF)); //concatenate with the lower byte
              
              IQS316.LTA_samples[5] = ((LTA_temp_samples[2] & 0x000F) << 8); 
              IQS316.LTA_samples[5] = (IQS316.LTA_samples[5] |  (LTA_temp_samples[3] & 0x00FF));
             
              IQS316.LTA_samples[6] = ((LTA_temp_samples[4] & 0x000F) << 8); 
              IQS316.LTA_samples[6] = (IQS316.LTA_samples[6] |  (LTA_temp_samples[5] & 0x00FF)); 
             
              IQS316.LTA_samples[7] = ((LTA_temp_samples[6] & 0x000F) << 8); 
              IQS316.LTA_samples[7] = (IQS316.LTA_samples[7] |  (LTA_temp_samples[7] & 0x00FF)); 

            }


            
            
            break;
            case 3:
            IQS316.prox12_19 = IQS316.prox12_19 & 0xF0;
            IQS316.touch12_19 = IQS316.touch12_19 & 0xF0;
            IQS316.prox12_19 = IQS316.prox12_19 | (temp_prox & 0x0F);
            IQS316.touch12_19 = IQS316.touch12_19 | (temp_touch & 0x0F);
         
            if (CUR_SAM_EN) {            
              //concatenate the two bytes to yield 1 sample each,  for a total of 4 samples per group            
              IQS316.samples[8] = ((temp_samples[0] & 0x00FF) << 8); 
              IQS316.samples[8] = (IQS316.samples[8] |  (temp_samples[1] & 0x00FF));
              
              IQS316.samples[9] = ((temp_samples[2] & 0x00FF) << 8); 
              IQS316.samples[9] = (IQS316.samples[9] |  (temp_samples[3] & 0x00FF));
             
              IQS316.samples[10] = ((temp_samples[4] & 0x00FF) << 8); 
              IQS316.samples[10] = (IQS316.samples[10] |  (temp_samples[5] & 0x00FF)); 
             
              IQS316.samples[11] = ((temp_samples[6] & 0x00FF) << 8); 
              IQS316.samples[11] = (IQS316.samples[11] |  (temp_samples[7] & 0x00FF));            
              } 

            if (LTA_SAM_EN) {
              IQS316.LTA_samples[8] = ((LTA_temp_samples[0] & 0x000F) << 8); //retrieve the first 4 bit
              IQS316.LTA_samples[8] = (IQS316.LTA_samples[8] |  (LTA_temp_samples[1] & 0x00FF)); //concatenate with the lower byte
              
              IQS316.LTA_samples[9] = ((LTA_temp_samples[2] & 0x000F) << 8); 
              IQS316.LTA_samples[9] = (IQS316.LTA_samples[9] |  (LTA_temp_samples[3] & 0x00FF));
             
              IQS316.LTA_samples[10] = ((LTA_temp_samples[4] & 0x000F) << 8); 
              IQS316.LTA_samples[10] = (IQS316.LTA_samples[10] |  (LTA_temp_samples[5] & 0x00FF)); 
             
              IQS316.LTA_samples[11] = ((LTA_temp_samples[6] & 0x000F) << 8); 
              IQS316.LTA_samples[11] = (IQS316.LTA_samples[11] |  (LTA_temp_samples[7] & 0x00FF)); 

            }              
            
            break;
            case 4:
            IQS316.prox12_19 = IQS316.prox12_19 & 0x0F;
            IQS316.touch12_19 = IQS316.touch12_19 & 0x0F;
            IQS316.prox12_19 = IQS316.prox12_19 | ((temp_prox & 0x0F) <<4);
            IQS316.touch12_19 = IQS316.touch12_19 | ((temp_touch & 0x0F) << 4);
            

            if (CUR_SAM_EN) {            
              //concatenate the two bytes to yield 1 sample each,  for a total of 4 samples per group
              IQS316.samples[12] = ((temp_samples[0] & 0x00FF) << 8); 
              IQS316.samples[12] = (IQS316.samples[12] |  (temp_samples[1] & 0x00FF));
              
              IQS316.samples[13] = ((temp_samples[2] & 0x00FF) << 8); 
              IQS316.samples[13] = (IQS316.samples[13] |  (temp_samples[3] & 0x00FF));
             
              IQS316.samples[14] = ((temp_samples[4] & 0x00FF) << 8); 
              IQS316.samples[14] = (IQS316.samples[14] |  (temp_samples[5] & 0x00FF)); 
             
              IQS316.samples[15] = ((temp_samples[6] & 0x00FF) << 8); 
              IQS316.samples[15] = (IQS316.samples[15] |  (temp_samples[7] & 0x00FF));
              }

            if (LTA_SAM_EN) {
              IQS316.LTA_samples[12] = ((LTA_temp_samples[0] & 0x000F) << 8); //retrieve the first 4 bit
              IQS316.LTA_samples[12] = (IQS316.LTA_samples[12] |  (LTA_temp_samples[1] & 0x00FF)); //concatenate with the lower byte
              
              IQS316.LTA_samples[13] = ((LTA_temp_samples[2] & 0x000F) << 8); 
              IQS316.LTA_samples[13] = (IQS316.LTA_samples[13] |  (LTA_temp_samples[3] & 0x00FF));
             
              IQS316.LTA_samples[14] = ((LTA_temp_samples[4] & 0x000F) << 8); 
              IQS316.LTA_samples[14] = (IQS316.LTA_samples[14] |  (LTA_temp_samples[5] & 0x00FF)); 
             
              IQS316.LTA_samples[15] = ((LTA_temp_samples[6] & 0x000F) << 8); 
              IQS316.LTA_samples[15] = (IQS316.LTA_samples[15] |  (LTA_temp_samples[7] & 0x00FF)); 

            }    

            break;
            }
            
            //A new conversion is initiated automatically at this point when stop bit is send
            temp_num = readRegister(GROUP_NUM, false); 
            
        }  while ((temp_num != 0) && (temp_num != start_num)); //ensure all groups are checked
    }

    calculateSampleDifference(); //get LTA - samples
    trackChannelMinMax();
    
    if (scale) {
      scale_to_same();
    }
}

//calculateSampleDifference()
//Description : calculates the differences between LTA and the normal sample
void calculateSampleDifference()
{
      for (unsigned char i = 0; i < 16; i++) {
          IQS316.sample_differences[i] = IQS316.LTA_samples[i] - IQS316.samples[i];
          if ( IQS316.sample_differences[i] < 0) //account for negative numbers
            { IQS316.sample_differences[i] =  0; }
      }
}

//trackChannelMinMax()
//Description : Tracks the min and max of each sensor channels to use for calibration
void trackChannelMinMax(void)
{
   for (unsigned char i = 0; i < 16; i++) {
      
      //updates min and max
      if (IQS316.sample_differences[i] < IQS316.channel_min[i]) {
          IQS316.channel_min[i] = IQS316.sample_differences[i];
      }
      if (IQS316.sample_differences[i] > IQS316.channel_max[i]) {
          IQS316.channel_max[i] = IQS316.sample_differences[i];
      }
   }
}


void scale_to_same()
{
    int range;
    
    for ( unsigned char i = 0; i < 16; i++)
    {
       range = IQS316.channel_max[i] - IQS316.channel_min[i];
       IQS316.scaled[i] = (int)(((long) IQS316.sample_differences[i] * 1000)/range);
    }   
}
/***********************************************************************/
/////////////// LOW LEVEL FUNCTIONS /////////////////////////////
//writeRegister()
//Description : Writes a byte of data to the register address using I2C
void writeRegister(byte registerAddress, byte data, boolean send_stop){
  while(digitalRead(IRDY)==1){};// wait for IQS to be ready
  
  Wire.beginTransmission(IQS316_ADDRESS); // fixed device address : 111 0100  ) 
  Wire.write(registerAddress);  
  Wire.write(data);
  int ret = Wire.endTransmission(send_stop);

  if (ret != 0) //if return value is not 0, there is a problem
  {
    Comms_Error(ret);
  }

  delay(1);
}

//same as above but send stop by default
void writeRegister(byte registerAddress, byte data)
{
  writeRegister(registerAddress, data, true);
}

//readRegister()
//Description : Reads a byte of data from a register using I2C, send a stop byte at the end
byte readRegister(byte registerAddress)
{
  readRegister(registerAddress, true);
}


//readRegister()
//Description : Reads a byte of data from the register address using I2C
//Retuns : Byte received 
byte readRegister(byte registerAddress, boolean send_stop){
  byte c;
  while(digitalRead(IRDY)==1){};// wait for IQS to be ready
  
  Wire.beginTransmission(IQS316_ADDRESS);   
  Wire.write(registerAddress);    // request device info
  int ret = Wire.endTransmission(false);    // send restart transmitting

  if (ret != 0) //if return value is not 0, there is a problem
  {

    Comms_Error(ret);
  }

  
  Wire.requestFrom(IQS316_ADDRESS, 1, (int) send_stop);
  while (Wire.available()) {      // slave may send less than requested
    c = Wire.read();          // receive a byte as character
  }// while
  
  delay(1);
  return c;
}

//Comms_Error()
//Description : Communication Error Routine
//Retuns : None
void Comms_Error(int err_code)
{
  #ifndef PUTTY_DISABLE
  Serial.print("Error occured during communications. Code - "); Serial.println(err_code,DEC);
  Serial.println("Rebooting...");
  #endif

  reboot();
}

//reboot()
//Desciption : Reboots after x seconds
void reboot()
{
  wdt_disable();
  wdt_enable(WDTO_4S);
  while(1) {}
}


//resetIQS()
//Description : IQS316 is reset when MCLR is held LOW and then high the time it takes after resetting to setup is approximately 16ms
void resetIQS(){
  digitalWrite(MCLR,0);
  delay(2);
  digitalWrite(MCLR,1);
  delay(16);
}