#include <IRremote.h>
#include <Wire.h>
#include <DS3231.h>
#include <SoftwareSerial.h>

#define SEMANTIC_VERSION "v1.0.0"

/*
** Adjust these variables to suit your needs.
*/
int baudRate                = 9600;
const int maxIRModuleCount  = 2;
const int maxIRColorCodes   = 9;
const int maxPIRModuleCount = 1;
const int maxRTCModuleCount = 1;
SoftwareSerial ATDevice(5, 4); //RX, TX pins for the WiFi module: comment out if you don't need it
IRsend irsend;
DS3231 rtc;

/*
** Method used for debugging.
** This will be removed in a later commit.
*/
template <class T>
void print(T t)
{
  Serial.println(t);
}

struct CodeDictionary
{
  const char* key;
  long int code;

  CodeDictionary(){}
  CodeDictionary(const char* key, long int code)
  {
    this->key = key;
    this->code = code;
  }
};

class IRModule 
{
public:
  const char* name;
  int size = 0;
  CodeDictionary codes[maxIRColorCodes];

  IRModule(){}
  IRModule(const char* name)
  {
    this->name = name;
  }

  void AddCode(CodeDictionary code)
  {
    codes[size] = code;
    size = size + 1;
  }

  CodeDictionary FindCode(const char* name)
  {
    for (int i = 0; i < size; i++)
    {
      if (codes[i].key == name)
      {
        return codes[i];
      }
    }
  }
};

class PIRModule 
{
private:
  int pirPin = 0;

public: 
  void SetDefaultPin(int pirPin)
  {
    this->pirPin = pirPin;
    pinMode(pirPin, INPUT);
  }

  int GetStatus() 
  {
    return digitalRead(pirPin);
  }
};

class RTCModule 
{
private:
  bool century = false;
  bool h12Flag = false;
  bool pmFlag = false;

public:
  enum DateTimeFormat
  {
    YEAR = 0,
    MONTH = 1,
    DAY = 2,
    HOUR = 3,
    MINUTE = 4,
    SECOND = 5
  };

  void SetDateTime(int year, int month, int day, int hour, int minute, int second)
  {
    rtc.setYear(year);
    rtc.setMonth(month);
    rtc.setDate(day);
    rtc.setHour(hour);
    rtc.setMinute(minute);
    rtc.setSecond(second);
  }

  int GetYear()
  {
    return rtc.getYear();
  }

  int GetMonth()
  {
    return rtc.getMonth(century);
  }

  int GetDay()
  {
    return rtc.getDate();
  }

  int GetHour()
  {
    return rtc.getHour(h12Flag, pmFlag);
  }

  int GetMinute()
  {
    return rtc.getMinute();
  }

  int GetSecond()
  {
    return rtc.getSecond();
  }
};

class ModulesController 
{
private:
    IRModule irModules[maxIRModuleCount];
    PIRModule pirModules[maxPIRModuleCount];
    RTCModule rtcModules[maxRTCModuleCount];
    int irModuleSize = 0;
    int pirModuleSize = 0;
    int rtcModuleSize = 0;

public:
    void AddModule(IRModule module)
    {
      irModules[irModuleSize] = module;
      irModuleSize++;
    }

    void AddModule(PIRModule module)
    {
      pirModules[pirModuleSize] = module;
      pirModuleSize++;
    }

    void AddModule(RTCModule module)
    {
      rtcModules[rtcModuleSize] = module;
      rtcModuleSize++;
    }

    void ChangeColor(const char* color)
    {
      for (int i = 0; i < maxIRModuleCount; i++)
      {
        irsend.sendNEC(irModules[i].FindCode(color).code, 32);
      }
    }

    bool DetectMotion()
    {
      for (int i = 0; i < maxPIRModuleCount; i++)
      {
        if (pirModules[i].GetStatus() == HIGH)
        {
          return true;
        }
      }

      return false;
    }

    int GetFromRTC(int RTCModuleIndex, RTCModule::DateTimeFormat format)
    {
      switch (format) {
        case RTCModule::YEAR:
          return rtcModules[RTCModuleIndex].GetYear();
        case RTCModule::MONTH:
          return rtcModules[RTCModuleIndex].GetMonth();
        case RTCModule::DAY:
          return rtcModules[RTCModuleIndex].GetDay();
        case RTCModule::HOUR:
          return rtcModules[RTCModuleIndex].GetHour();
        case RTCModule::MINUTE:  
          return rtcModules[RTCModuleIndex].GetMinute();
        case RTCModule::SECOND:
          return rtcModules[RTCModuleIndex].GetSecond();
      }
    }
};

String SetATCommand(const char *toSend, unsigned long milliseconds = 500) 
{
  String result;
  Serial.print("Sending: ");
  Serial.println(toSend);
  ATDevice.println(toSend);
  unsigned long startTime = millis();
  Serial.print("Received: ");
  
  while (millis() - startTime < milliseconds) 
  {
    if (ATDevice.available()) 
    {
      char c = ATDevice.read();
      Serial.write(c);
      result += c;
    }
  }

Serial.println();
return result;
}

/*
**  The code below can be modified to suit your needs.
**  The default values are for my setup.
**
**  Avoid modifying the code above unless you know what you are doing.
*/
ModulesController controller;
IRModule irCeilingModule("Ceiling Module");
PIRModule pirDoorModule;
RTCModule rtcModule;

// Executes once at the beginning of the program.
void setup() 
{
  Serial.begin(baudRate);
  Wire.begin();
  ATDevice.begin(baudRate);
  SetATCommand("AT+CIPMUX=1");
  SetATCommand("AT+CIPSERVER=1,8225");
  Serial.println("Arduino UNO running RinTerminal " + String(SEMANTIC_VERSION));

  irCeilingModule.AddCode(CodeDictionary("white", 0xF7E01F));
  irCeilingModule.AddCode(CodeDictionary("green", 0xF7A05F));
  irCeilingModule.AddCode(CodeDictionary("blue", 0xF7609F));
  irCeilingModule.AddCode(CodeDictionary("red", 0xF720DF));
  irCeilingModule.AddCode(CodeDictionary("orange", 0xF710EF));
  irCeilingModule.AddCode(CodeDictionary("purple", 0xF750AF));
  irCeilingModule.AddCode(CodeDictionary("pink", 0xF76897));
  irCeilingModule.AddCode(CodeDictionary("off", 0xF740BF));
  irCeilingModule.AddCode(CodeDictionary("on", 0xF7C03F));

  pirDoorModule.SetDefaultPin(2);
  controller.AddModule(pirDoorModule);
  controller.AddModule(irCeilingModule);
  controller.AddModule(rtcModule);
}

// Runs repeatedly throughout the program's lifecycle.
void loop() 
{
  if (ATDevice.available())
  {
    Serial.println(ATDevice.readString());
  }
}
