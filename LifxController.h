#ifndef LifxController_h
#define LifxController_h


#include <Udp.h>
#include "LifxDevice.h"

#define MAX_DEVICES 100

// Port we listen to
const unsigned int DEFAULT_LOCAL_PORT = 8888;

// Port for talking to LIFX devices
const unsigned int LIFX_PORT  = 56700;


class LifxController
{
  public:
    LifxController();
    LifxController(UDP& udp);
    void begin();
    void begin(unsigned int localPort);
    void probeForDevices();
    void readIncomingPackets();
    LifxDevice* createDevice(IPAddress ip, unsigned int port);
    LifxDevice* getDevice(IPAddress ip, unsigned int p);
    LifxDevice* getDeviceByLabel(const char* label);
    LifxDevice* getDevice(int id);
    int getDeviceCount();
    void sendPacket(LifxDevice* device, lifx_header* header, byte* payload, int payloadLength);
  private:
     UDP*          _udp;
     int    _local_port = DEFAULT_LOCAL_PORT;
     LifxDevice*  _devices[MAX_DEVICES];
     int  _deviceCount=0;
     
     
};

#endif
