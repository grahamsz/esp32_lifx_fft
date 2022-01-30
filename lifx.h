#ifndef Lifx_h
#define Lifx_h

#pragma pack(push, 1)
typedef struct {
  /* frame */
  uint16_t size;
  uint16_t protocol:12;
  uint8_t  addressable:1;
  uint8_t  tagged:1;
  uint8_t  origin:2;
  uint32_t source;
  /* frame address */
  uint8_t  target[8];
  uint8_t  reserved[6];
  uint8_t  res_required:1;
  uint8_t  ack_required:1;
  uint8_t  :6;
  uint8_t  sequence;
  /* protocol header */
  uint64_t :64;
  uint16_t type;
  uint16_t :16;
  /* variable length payload follows */
} lifx_header;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct{
  uint16_t hue;
  uint16_t saturation;
  uint16_t brightness;
  uint16_t kelvin;
} hsbk;
#pragma pack(pop)

// Device::Color Payload
#pragma pack(push, 1)
typedef struct {
  uint8_t reserved;
  uint16_t hue;
  uint16_t saturation;
  uint16_t brightness;
  uint16_t kelvin;
  uint32_t duration;
} lifx_payload_device_set_color;
#pragma pack(pop)

// Device::Color Payload
#pragma pack(push, 1)
typedef struct {

  uint32_t duration;
  uint8_t apply;
  uint16_t index;
  uint8_t colors_count;
  hsbk colors[82];

} lifx_payload_device_set_extended_color_zones;
#pragma pack(pop)



// Device::SetPower Payload
#pragma pack(push, 1)
typedef struct {
  uint16_t level;
} lifx_payload_device_set_power;
#pragma pack(pop)

// Device::StatePower Payload
#pragma pack(push, 1)
typedef struct {
  uint16_t level;
} lifx_payload_device_state_power;
#pragma pack(pop)



// Device::StatePower Payload
#pragma pack(push, 1)
typedef struct {  
} lifx_no_payload;
#pragma pack(pop)




// packet types
const uint8_t GET_SERVICE = 0x02;          //REQ 2 GetService 
const uint8_t STATE_SERVICE = 0x03;              //RSP 3 StateService

//const uint8_t GET_HOST_INFO = 0x0c;          //REQ 12
//const uint8_t STATE_HOST_INFO = 0x0d;        //RSP 13

const uint8_t GET_MESH_FIRMWARE_STATE = 0x0e;  //REQ 14 GetHostFirmware 
const uint8_t MESH_FIRMWARE_STATE = 0x0f;      //REQ 15 StateHostFirmware 

const uint8_t GET_WIFI_INFO = 0x10;          //REQ 16
const uint8_t STATE_WIFI_INFO = 0x11;        //RSP 17

const uint8_t GET_WIFI_FIRMWARE_STATE = 0x12;  //REQ 18 GetWifiFirmware 
const uint8_t WIFI_FIRMWARE_STATE = 0x13;		  //RSP 19 StateWifiFirmware 

const uint8_t GET_POWER_STATE = 0x74;          //REQ 116  GetPower 
const uint8_t SET_POWER_STATE = 0x75;	        //REQ 117 SetPower 
const uint8_t POWER_STATE = 0x76;               //RSP 118  StatePower 

const uint8_t GET_BULB_LABEL = 0x17; 		      //REQ 23 GetLabel 
const uint8_t SET_BULB_LABEL = 0x18; 		      //REQ 24 SetLabel 
const uint8_t BULB_LABEL = 0x19; 			        //RSP 25 StateLabel 

const uint8_t GET_VERSION_STATE = 0x20;	      //REQ 32 GetVersion 
const uint8_t VERSION_STATE = 0x21;		        //RSP 33 StateVersion 



//const uint8_t GET_INFO = 0x22;                 //REQ 34 GetInfo
//const uint8_t STATE_INFO = 0x23;               //RSP 35 StateInfo

const uint8_t LIFX_ACK = 0x2d;                      //RSP 45 Acknowledgement 

const uint8_t GET_LOCATION = 0x30;              //REQ 48 GetLocation
const uint8_t SET_LOCATION = 0x31;             //REQ 49 SetLocation
const uint8_t STATE_LOCATION = 0x32;           //RSP 50 StateLocation

const uint8_t GET_GROUP = 0x33;                //REQ 51
const uint8_t SET_GROUP = 0x34;                //REQ 52
const uint8_t STATE_GROUP = 0x35;              //RSP 53

//const uint8_t ECHO_REQUEST = 0x3A;             //REQ 58
//const uint8_t ECHO_RESPONSE = 0x3B;            //RSP 59

const uint8_t GET_LIGHT_STATE = 0x65;		        //REQ 101 Get
const uint8_t SET_LIGHT_STATE = 0x66;		        //REQ 102 SetColor   (reserved,color,duration)

//const uint8_t SET_DIM_ABSOLUTE = 0x68;           //dec: 104
//const uint8_t SET_DIM_RELATIVE = 0x69;	          //dec: 105
const uint8_t LIGHT_STATUS = 0x6b;               //RSP 107 State

//const uint8_t GET_INFRARED = 0x78;               //REQ 120
//const uint8_t STATE_INFRARED = 0x79;               //RSP 121
//const uint8_t SET_INFRARED = 0x7A;               //REQ 122

const uint8_t SET_WAVEFORM = 0x67;               //REQ 103
const uint8_t SET_WAVEFORM_OPTIONAL = 0x77;      //dec 119

//MULTI-ZONE MESSAGES
const uint16_t SET_COLOR_ZONES = 501;            //REQ
const uint16_t GET_COLOR_ZONES = 0x1F6;          //REQ 502

const uint16_t STATE_ZONE = 503;                 //RSP 0x1F7
const uint16_t STATE_MULTI_ZONE = 506;           //RSP

const uint16_t GET_MULTIZONE_EFFECT = 507;       //REQ 0x1FB
const uint16_t SET_MULTIZONE_EFFECT = 508;       //REQ 0x1FC
const uint16_t STATE_MULTIZONE_EFFECT = 509;     //RSP 0x1FD

//multi-zone messages (extended multizone >=v2.77)

const uint16_t SET_EXTENDED_COLOR_ZONES = 510;
const uint16_t GET_EXTENDED_COLOR_ZONES = 511;
const uint16_t STATE_EXTENDED_COLOR_ZONES = 512;

//TILE MESSAGES
//
//const uint16_t GET_DEVICE_CHAIN = 701;
//const uint16_t STATE_DEVICE_CHAIN = 702;
//const uint16_t SET_USER_POSITION = 703;
//const uint16_t GET_TILE_STATE_64 = 707;
//const uint16_t STATE_TILE_STATE_64 = 711;
//const uint16_t SET_TILE_STATE_64 = 715;
//



#endif
