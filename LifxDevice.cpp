#include "Arduino.h"
#include "LifxDevice.h"
#include <Udp.h>
#include "LifxController.h"


LifxDevice::LifxDevice( LifxController* controller, IPAddress ip, unsigned int port)
{
  //pinMode(pin, OUTPUT);
  this->ip=ip;
  this->port=port;
  this->_controller = controller;
}

void LifxDevice::probeForConfiguration()
{

    lifx_header header;
    lifx_no_payload payload;
    
    // Initialise both structures
    memset(&header, 0, sizeof(header));
    memset(&payload, 0, sizeof(payload));
    
    // Setup the header
    header.size = sizeof(lifx_header) + sizeof(payload); // Size of header + payload
    header.tagged = 1;
    header.addressable = 1;
    header.protocol = 1024;
    header.source = 123;
    header.ack_required = 1;
    header.sequence = 100;
    header.type = GET_BULB_LABEL;
    this->_controller->sendPacket(this,&header,(byte*) &payload,sizeof(payload));   
}

void LifxDevice::probeForLightState()
{


    lifx_header header;
    lifx_no_payload payload;
    
    // Initialise both structures
    memset(&header, 0, sizeof(header));
    memset(&payload, 0, sizeof(payload));
    
    // Setup the header
    header.size = sizeof(lifx_header) + sizeof(payload); // Size of header + payload
    header.tagged = 1;
    header.addressable = 1;
    header.protocol = 1024;
    header.source = 123;
    header.ack_required = 1;
    header.sequence = 100;
    
    
    header.type = GET_LIGHT_STATE;
    
    this->_controller->sendPacket(this,&header,(byte*) &payload,sizeof(payload));   
}

void LifxDevice::setColorHSV(uint16_t h, uint16_t s, uint16_t v)
{


    lifx_header header;
    lifx_payload_device_set_color payload;
    
    // Initialise both structures
    memset(&header, 0, sizeof(header));
    memset(&payload, 0, sizeof(payload));
    
    // Setup the header
    header.size = sizeof(lifx_header) + sizeof(payload); // Size of header + payload
    header.tagged = 1;
    header.addressable = 1;
    header.protocol = 1024;
    header.source = 123;
    header.ack_required = 1;
    header.sequence = 100;
    
    payload.hue=h;
    payload.brightness=v;
    payload.saturation=s;
    payload.kelvin=0;

    payload.duration=250;
    
    header.type = SET_LIGHT_STATE;
    
    this->_controller->sendPacket(this,&header,(byte*) &payload,sizeof(payload));   
}

void LifxDevice::setColorZonesHSV(hsbk* colors, int colorCount)
{


    lifx_header header;
    lifx_payload_device_set_extended_color_zones payload;
    
    // Initialise both structures
    memset(&header, 0, sizeof(header));
    memset(&payload, 0, sizeof(payload));
    
    // Setup the header
    header.size = sizeof(lifx_header) + sizeof(payload); // Size of header + payload
    header.tagged = 1;
    header.addressable = 1;
    header.protocol = 1024;
    header.source = 123;
    header.ack_required = 1;
    header.sequence = 100;
    

    payload.duration=100;
    payload.apply=1;
    payload.colors_count=colorCount;
    memcpy(&payload.colors,colors,sizeof(payload.colors));
    
    header.type = SET_EXTENDED_COLOR_ZONES;
    
    this->_controller->sendPacket(this,&header,(byte*) &payload,sizeof(payload));   
}

void LifxDevice::getColorZones()
{

    lifx_header header;
    lifx_no_payload payload;
    
    // Initialise both structures
    memset(&header, 0, sizeof(header));
    memset(&payload, 0, sizeof(payload));
    
    // Setup the header
    header.size = sizeof(lifx_header) + sizeof(payload); // Size of header + payload
    header.tagged = 1;
    header.addressable = 1;
    header.protocol = 1024;
    header.source = 123;
    header.ack_required = 1;
    header.sequence = 100;
    header.type = GET_EXTENDED_COLOR_ZONES;
    this->_controller->sendPacket(this,&header,(byte*) &payload,sizeof(payload));   
}

void LifxDevice::printDetails()
{

  Serial.println("Device:");
  Serial.print("  IP      ");
  Serial.println(this->ip);
  Serial.print("  Port    ");
  Serial.println(this->port);
  if (this->hasLabel)
  {
    Serial.print("  Label   ");
    Serial.println(this->label);
  }
}
void LifxDevice::parseMessage(lifx_header* header, byte* payload, int payloadLength)
{
  Serial.print("device has message of type ");
  Serial.println(header->type);

  if (header->type==BULB_LABEL)
  {  
    
    memcpy(&(this->label), payload, 32);
    this->hasLabel=true;
  }
  if (header->type==LIGHT_STATUS)
  {
    memcpy(&(this->color),payload, sizeof(hsbk));
    this->hasColor=true;
  }
}
