#ifndef IQS316_H
#define IQS316_H

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

boolean scale = false;
boolean LTA = false;
boolean printRange = false;
boolean filter_print = false;

struct {
  byte prox_detected;   //5 bytes
  byte prox4_11;
  byte prox12_19;
  byte touch4_11;
  byte touch12_19;
  
  //From here
  short samples[16]; //short for zero is 2 bytes => 32 bytes in total
  short LTA_samples[16];  

  short sample_differences[16]; // LTA - current samples
  short filtered_samples[16]; //Samples fter applying MA_Filter

  //for calibration
  short channel_min[16]; //min number for that sensor channel
  short channel_max[16]; //max number for that sensor channel
  
  short scaled[16];

  short channel_calibration_val[5];   
  //to here uses 32 * 8 = 256 bytes

  boolean attached[5];
  boolean proper_attach;

} IQS316;

#endif