#ifndef LifxDevice_h
#define LifxDevice_h

#include "lifx.h"
typedef class LifxController LifxController;

class LifxDevice
{
  public:
    LifxDevice(LifxController* controller, IPAddress ip, unsigned int port);
    IPAddress ip;
    unsigned int port;
    char label[32];
    bool hasLabel=false;
    hsbk color;
    bool hasColor=false;
    void parseMessage(lifx_header* header, byte* payload, int payloadLength);
    void probeForConfiguration();
        void probeForLightState();
    void getColorZones();
    void printDetails();
    void setColorHSV(uint16_t h, uint16_t s, uint16_t v);
    void setColorZonesHSV(hsbk* colors, int colorCount);
  private:
    LifxController* _controller;
    
};

#endif
