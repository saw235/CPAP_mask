
#include <avr/wdt.h> //for watchdog
//////////////////////////////////////
// Arduino's Routine code /////////// 
////////////////////////////////////

int test_samples[16] = { 1000, 2000, 3000, 4000,
                    5000, 5252, 0xFEFE, 0x2828,
                    0xAAAA, 1010, 0x1234, 0xABCD,
                    0xFFFF, 0x1919, 0x2929, 0x3939};


//run once when microcontroller starts
void setup(){
  
  Serial.begin(115200);// initialize UART
  Serial.println(F("Done initializing UART..."));
  

}


void loop(){

}




//SerialEvent Handler
void serialEvent(){

  switch(Serial.read()){
    case(0x28) : 
      //sendTransmitReadyByte();
      sendSample2GUI();
      break;
    default :
      break;
      //start send samples routine
  }
}


//sendSample2GUI
//Description : Send samples in highbyte and low byte
void sendSample2GUI(){

  //make a buffer to store 2 byte for each int
  for (unsigned char i = 0; i < 16; i++){
    int num2convert = test_samples[i];
    byte buf[2];

    //parse into high and low byte
    buf[0] = ((num2convert & 0xFF00) >> 8); //HI_BYTE
    buf[1] = num2convert & 0x00FF; // LO_BYTE

    Serial.write(buf, 2);
  }
}


void sendTransmitReadyByte(){
  byte buf = 0x96;
  //send acknowledge byte
  Serial.write(buf);
}

//reboot()
//Desciption : Reboots after x seconds
void reboot()
{
  wdt_disable();
  wdt_enable(WDTO_4S);
  while(1) {}
}


