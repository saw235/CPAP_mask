#include "MovingAverageFilter.h"

// potentiometer wiper (middle terminal) connected to analog pin 0
#define PRESSURE_AN_PIN A0  
#define V_S 5 //voltage source
#define FILTER_SIZE 5
int pressure_filter_buffer[FILTER_SIZE];  // 32 bytes * 10 = 320 bytes 

boolean scale = false;
boolean LTA = false;
boolean printRange = false;
boolean filter_print = false;

float kPA_Pressure = 0; //Pressure in kPA   //float uses 4 bytes in uno

void setup(){
	Serial.begin(115200);// initialize UART
}

void loop(){
	loadPressure_data();
    sendSample2GUI();
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
      //Calibrate2Face();
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
  //send 0x12 at the start of the bytes if scaled
  //send 0x13 if else
  if (scale){
    buf[0] = 0x12;
  } else {buf[0] = 0x13;} 


  Serial.write(buf,1);

  //send IQS316 samples
  //send 16 samples of int (2 bytes each) => 32 bytes

  //make a buffer to store 2 byte for each int
  for (unsigned char i = 0; i < 16; i++){
    int num2convert;

    if (!scale){
      num2convert = 1000;
    } else { num2convert = 500;}

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
//////////////////////////////////////////////////////
//HIGH LEVEL FUNCTIONS for Pressure Sensor Handling //
/////////////////////////////////////////////////////

void loadPressure_data(void){
  int nval_Pressure = 0;
  nval_Pressure = analogRead(PRESSURE_AN_PIN);

  filtered_Pressure_nval = MA_Filter_current(pressure_filter_buffer,nval_Pressure,FILTER_SIZE)
  kPA_Pressure = convert_digital_to_kPA(filtered_Pressure_nval);
}

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