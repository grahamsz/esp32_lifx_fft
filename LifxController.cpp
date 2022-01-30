
#include "Arduino.h"
#include "LifxController.h"

#include <Udp.h>
#include "lifx.h"


LifxController::LifxController()
{
}
LifxController::LifxController(UDP& udp)
{  
    this->_udp            = &udp;    
}
void LifxController::begin() {  
  this->begin(DEFAULT_LOCAL_PORT);
}


void LifxController::begin(unsigned int local_port) { 
  this->_local_port = local_port;  
  this->_udp->begin(this->_local_port);  
  Serial.print("Controller listening on port ");
  Serial.println(this->_local_port);


  
}

LifxDevice* LifxController::getDevice(int id)
{
  return this->_devices[id];
}

LifxDevice* LifxController::getDeviceByLabel(const char* label)
{
  for (int i=0;i<this->_deviceCount;i++)
  {
      if ((this->_devices[i]->hasLabel ) && (strcmp(this->_devices[i]->label,label) ==0))
      {
        return this->_devices[i];
      }
  }
  return nullptr;
}

int LifxController::getDeviceCount()
{
  return this->_deviceCount;
}

LifxDevice* LifxController::getDevice(IPAddress ip, unsigned int p)
{
  for (int i=0;i<this->_deviceCount;i++)
  {
      if ((this->_devices[i]->ip == ip) && (this->_devices[i]->port ==p))
      {
        return this->_devices[i];
      }
  }
  return nullptr;
}

LifxDevice* LifxController::createDevice(IPAddress ip, unsigned int p)
{
  LifxDevice* result = new LifxDevice(this, ip,p); 
  this->_devices[this->_deviceCount] = result;
  this->_deviceCount++;      
  result->probeForConfiguration();
  return result;
}


void LifxController::sendPacket(LifxDevice* device, lifx_header* header, byte* payload, int payloadLength)
{
    _udp->beginPacket(device->ip, device->port);
    _udp->write((uint8_t*) header, sizeof(lifx_header)); // Treat the structures like byte arrays
    _udp->write(payload, payloadLength);    // Which makes the data on wire correct
    _udp->endPacket();  

}

void LifxController::probeForDevices()
{
  
    
    IPAddress broadcast_ip(255, 255, 255, 255);
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
    header.type = GET_SERVICE;
    
    // Setup payload

    _udp->beginPacket(broadcast_ip, 56700);
    _udp->write((uint8_t *) &header, sizeof(lifx_header)); // Treat the structures like byte arrays

    _udp->endPacket();
}


void LifxController::readIncomingPackets()
{
  byte packetBuffer[1024]; //buffer to hold incoming packet
  bool keepRunning = true;
  while (true)
  {
    int packetSize = _udp->parsePacket();
    if (!packetSize) {
      break;
    }
  
      Serial.print("Received packet of size ");  
      Serial.print(packetSize);  
      Serial.print("  from ");  
      IPAddress remoteIp = _udp->remoteIP();  
      Serial.print(remoteIp);  
      Serial.print(", port ");  
      Serial.println(_udp->remotePort());

      LifxDevice* device = this->getDevice(_udp->remoteIP(), _udp->remotePort());
      
      if (device==nullptr)
      {
        //Serial.println ("  creating new device");
          device = this->createDevice(_udp->remoteIP(), _udp->remotePort());
          
      } else
      {
        //Serial.println ("  got existing device");
      }

      

    
  
  
      // read the packet into packetBufffer
  
      int len = _udp->read(packetBuffer, 1024);
  
      if (len > 0) {
  
        packetBuffer[len] = 0;
  
      }

      

      
  
 /*     Serial.print("  Contents:");
  
      
  
  
  
      for(int i = 0; i < len; i++) {
        Serial.print(packetBuffer[i], HEX);
        Serial.print(F(" "));
      }

      Serial.println();
*/

    lifx_header header ;
    byte payload[len-sizeof(lifx_header)]; // this is probalby overkill
    

    memcpy(&header,packetBuffer,sizeof(lifx_header));
    memcpy(&payload,packetBuffer+ sizeof(lifx_header),len-sizeof(lifx_header) );

    device->parseMessage(&header,payload,len-sizeof(lifx_header) );
  
    }
    
}
