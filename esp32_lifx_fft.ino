#include <WiFi.h>
#include <nvs.h>
#include <nvs_flash.h>
#include "esp_wifi.h"
#include "esp_adc_cal.h"
#include <WiFiUdp.h>
#include <Udp.h>
#include "esp_sleep.h"
#include "lifx.h"
#include "color.h"
#include "LifxDevice.h"
#include <esp_task_wdt.h>
#include "LifxController.h"
#include "arduinoFFT.h"

#define DEBUG

const char* WIFI_SSID = "xxxxx";
const char* WIFI_PASSWORD = "xxxxxx";
const char* LIFX_DEVICE_LABEL = "Hifi";

const int LIFX_MIN_BRIGHTNESS = 10000;
const int LIFX_MAX_BRIGHTNESS = 65535;

// Create the FFT object
arduinoFFT FFT = arduinoFFT();

// This is my custom Lifx Controller
LifxController controller;

// Here's a device to record the light strip that we're going to control
LifxDevice* hifiLightStrip;

// We need this to speak Lifx
WiFiUDP Udp;


const uint16_t SAMPLES = 1024; //This value MUST ALWAYS be a power of 2
const double SAMPLE_RATE_HZ = 30720;  // This is customized to get me a 30Khz band size!

unsigned int samplingPeriodMicroseconds;  // We'll compute a period so we can sample as close to our desired rate as posible


#define NUM_BANDS  8   // The number of bands that we want to break the signal into
double peakRight[NUM_BANDS];  
double peakLeft[NUM_BANDS];

const int bands[NUM_BANDS] = {  120, 360, 720, 960, 2000, 4000, 8000, 15000};  // frequencies in Hz
int bins[NUM_BANDS];


// We're going to disable the FFT when there's silence so we need to count the periods thare are quiet
long periodsWithSilence = 0;
int periodsWithMusic = 0;

const double SILENCE_LEVEL = 24 * SAMPLES;

// Define loop variable so these don't need to go on the stack
int i, band, adjustedLeft, adjustedRight;
unsigned long microseconds;
double running_delta,bandTotalLeft, bandTotalRight;

// buffers to hold our FFT components
double realRight[SAMPLES],realLeft[SAMPLES], imaginaryRight[SAMPLES],imaginaryLeft[SAMPLES];

// Buffer to hold all 82 possible zones a lifx strip can have
hsbk colors[82];


void initializeWiFi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  i = 0;

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    if (i++ == 20)
    {
      // give up after two seconds
      return;
    }
  }

  Serial.print("connected to wifi");

  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());

}


void initializeColorArray(uint16_t hue)
{

  for (i = 0; i < 82; i++)
  {
    colors[i].hue = hue;
    colors[i].saturation = 65535;
    colors[i].brightness = 0;
    colors[i].kelvin = 0;

  }
}

void setup() {


  // We increase the Watch Dog Timer to 3 seconds so the ESP doesn't reboot if it's slow connecting to LiFX
  esp_task_wdt_init(3, true); //3 second timeout


  // Serial is only used for debugging
  Serial.begin(115200);

  initializeWiFi();

  // We precalculate this so that our loop can cruise through and sample everything
  samplingPeriodMicroseconds = round(1000000 * (1.0 / SAMPLE_RATE_HZ));

  int binSize = SAMPLE_RATE_HZ / SAMPLES;

  Serial.print("With a sample rate of ");
  Serial.print(SAMPLE_RATE_HZ);
  Serial.print("Hz, and a buffer size of ");
  Serial.print(SAMPLES);
  Serial.print("samples, we'll have an FFT Bin Size of ");
  Serial.print(binSize);
  Serial.println(" Hz");


  for (i = 0; i < NUM_BANDS; i++)
  {

    bins[i] = bands[i] / binSize;

    Serial.print("Band ");
    Serial.print(i);
    Serial.print(" will run to ");
    Serial.print(bins[i]*binSize);
    Serial.print(" Hz vs an target of ");
    Serial.print(bands[i]);
    Serial.println(" Hz");

  }

  // This is used with an oscillioscope to measure timings - could probably be removed
  pinMode(22, OUTPUT);

  // This is my custom Lifx controller class, it communicates with the z-strip over UDP
  controller = LifxController(Udp);
  controller.begin();

  // We loop until we see a Lifx device called "Hifi"
  while (controller.getDeviceByLabel(LIFX_DEVICE_LABEL) == nullptr)
  {
    Serial.println("Waiting for device by name");
    controller.probeForDevices();
    controller.readIncomingPackets();
    for (i = 0; i < controller.getDeviceCount(); i++)
    {
      if (!controller.getDevice(i)->hasLabel)
      {
        controller.getDevice(i)->probeForConfiguration();
      }
    }
    controller.readIncomingPackets();

    Serial.print("We have detected ");
    Serial.print(controller.getDeviceCount());
    Serial.println(" devices.");

    for (i = 0; i < controller.getDeviceCount(); i++)
    {
      controller.getDevice(i)->printDetails();
      if (!controller.getDevice(i)->hasLabel)
      {
        controller.getDevice(i)->probeForConfiguration();
      }
    }
  }

  hifiLightStrip = controller.getDeviceByLabel(LIFX_DEVICE_LABEL);

  // Once we've got the device, we should wait until it's updated the controller with the current color
  // We'll use the color to modulate based on frequency
  while (!hifiLightStrip->hasColor)
  {
    hifiLightStrip->probeForLightState();
    delay(10);
    controller.readIncomingPackets();
  }

  // We never change the hue as part of our prcoess, so we can preset the whole array to match the hue that the strip currently has
  initializeColorArray(hifiLightStrip->color.hue);
}



void loop() {

  microseconds = micros();

  // We're going to read 1024 samples from each of the left and right channels
  for (i = 0; i < SAMPLES; i++)
  {
    realRight[i] = analogRead(36);  // read from GPIO 36
    realLeft[i] = analogRead(32);   // read from GPIO 32
    imaginaryRight[i] = 0;
    imaginaryLeft[i] = 0;
    while (micros() - microseconds < samplingPeriodMicroseconds) {
      // A small wait while we wait for the appropriate number of microseconds to hit or 32kHz sampling rate
      // slower Arduinos might need the sampling rate reduced
    }
    microseconds += samplingPeriodMicroseconds;
  }

  // My audio input is centered on 1386 because my front end circuit doesn't exactly center the signal on 1.65V
  // This process is used to detect silence

  running_delta = 0;

  // calculate a running delta to efficiently see if we have silence
  for (i = 1; i < SAMPLES; i++)
  {
    running_delta += abs(realLeft[i - 1] - realLeft[i]) + abs(realRight[i - 1] - realRight[i]);
  }

  if (running_delta < SILENCE_LEVEL)
  {
    // This set of samples saw silence on both audio channels
    // Lets record that this happened so we can decide when the audio is turned off and stop burning CPU & Network

    periodsWithSilence++;


    // Seperately we track a periodsWithMusic counter to detect when audio resumes, we'll decrease this if it's
    // quiet
    if (periodsWithMusic > 0) {
      periodsWithMusic--;
    }

    Serial.print("Detected Silence ");
    Serial.print(running_delta);
    Serial.print(" < ");
    Serial.print(SILENCE_LEVEL);
    Serial.print(" for ");
    Serial.print(periodsWithSilence);
    Serial.println(" periods.");

  } else
  {

    // If we get here then there _is_ audio in the current sample
    if (periodsWithSilence > 100)
    {

      // It's been 100 sample periods since we last had audio, so things appear to be starting up
      Serial.print("Detected music, will resume FFT when periodsWithMusic==4, currently periodsWithMusic=");
      Serial.println(periodsWithMusic);


      if (periodsWithMusic++ > 4)
      {

        // Now we're good to start the FFT process again
        Serial.println("Resuming FFT process");

        // The strip might have had its color changed in the UI while we've been paused
        // so let's probe it and chang eour default color to match

        hifiLightStrip->hasColor = false;
        while (!hifiLightStrip->hasColor)
        {
          hifiLightStrip->probeForLightState();
          vTaskDelay(10 / portTICK_PERIOD_MS);
          controller.readIncomingPackets();
        }

        // Let's reinitialize the color array with the new Hue
        initializeColorArray(hifiLightStrip->color.hue);

        // and reset our silence timeout
        periodsWithSilence = 0;
      }

    }
    else
    {

      periodsWithSilence = 0;
    }
  }



  if (periodsWithSilence > 50)
  {
    // Start to fade out when there's been no audio for 50 periods
    for (i = 0; i < 8; i++)
    {
      peakRight[i] = peakRight[i] * 0.9;
      peakLeft[i] = peakLeft[i] * 0.9;

    }
  } else
  {

    // Now this is the meat of the process - we do the usual FFT computation on the samples for each channel

    FFT.DCRemoval(realRight, SAMPLES);
    FFT.DCRemoval(realLeft, SAMPLES);
    FFT.Windowing(realRight, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Windowing(realLeft, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(realRight, imaginaryRight, SAMPLES, FFT_FORWARD);
    FFT.Compute(realLeft, imaginaryLeft, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(realRight, imaginaryRight, SAMPLES);
    FFT.ComplexToMagnitude(realLeft, imaginaryLeft, SAMPLES);

    band = 0;
    bandTotalLeft = 0;
    bandTotalRight = 0;

    for (i = 1; i < (SAMPLES / 2); i++)
    {
      // Sample 0 is unusable in an FFT and only first SAMPLES/2 are usable. Each array eleement represents a frequency and its value the amplitude.
  

      // Add up the total power in the band, while removing really low numbers
      if (realLeft[i] > 5)
      {
        bandTotalLeft += realLeft[i] - 5;
      }
      if (realRight[i] > 5)
      {
        bandTotalRight += realRight[i] - 5;
      }
      if (i == bins[band])
      {

        // Fudge the FFT levels to get a visually appealing result

        adjustedLeft = bandTotalLeft * (LIFX_MAX_BRIGHTNESS - LIFX_MIN_BRIGHTNESS) / 400000;
        adjustedRight = bandTotalRight * (LIFX_MAX_BRIGHTNESS - LIFX_MIN_BRIGHTNESS) / 400000  ;


        // Fade out the prior peaks
        peakLeft[band] *= 0.88;
        peakRight[band] *= 0.88;


        // Only apply our new band if it's higher than the existing (faded out value)
        if (adjustedLeft>peakLeft[band]) { peakLeft[band] = adjustedLeft; }
        if (adjustedRight>peakRight[band]) { peakRight[band] = adjustedRight; }

        // Make sure we aren't too high
        if (peakLeft[band] > LIFX_MAX_BRIGHTNESS - LIFX_MIN_BRIGHTNESS) {
          peakLeft[band] = LIFX_MAX_BRIGHTNESS - LIFX_MIN_BRIGHTNESS;
        }
        if (peakRight[band] > LIFX_MAX_BRIGHTNESS - LIFX_MIN_BRIGHTNESS) {
          peakRight[band] = LIFX_MAX_BRIGHTNESS - LIFX_MIN_BRIGHTNESS;
        }

        // proceed to the next band
        bandTotalLeft = 0;
        bandTotalRight = 0;
        band++;
      }



    }
  }


  // This then builds out the actual Lifx brightnesses
  // We apply the left and right bands to both the top and bottom of the furniture
  
  for (i = 0; i < 8; i++)
  {

    // colors 0 - 15 are on top
    //16-31 on bottom

    colors[i].brightness = peakLeft[i] + LIFX_MIN_BRIGHTNESS;
    colors[15 - i].brightness =  peakRight[i] + LIFX_MIN_BRIGHTNESS;
    colors[16 + i].brightness =  peakRight[i] + LIFX_MIN_BRIGHTNESS;
    colors[31 - i].brightness = peakLeft[i] + LIFX_MIN_BRIGHTNESS;
  }
  //Serial.println(".");
  //  Serial.println(maxVal);

  digitalWrite(22, 1);  // This allows a DSO to figure out how fast the timing runs


  if (periodsWithSilence < 500)
  {
    hifiLightStrip->setColorZonesHSV(&(colors[0]), 82);

    {
      for (band = 0; band < NUM_BANDS; band++)
      {
        float decay = (490.0 - band) / 500.0;
        if (peakRight[band] > 0)
          peakRight[band] =  peakRight[band] * decay;
        if (peakLeft[band] > 0)
          peakLeft[band] =  peakLeft[band] * decay;
      }
    } // Decay the peak


  }


  if (periodsWithSilence > 500)
  {

    delay(500);
    periodsWithSilence += 25; // this slows down our system when we don't have activity
  }

  digitalWrite(22, 0);
}
