#include <IRremote.h>

int baudRate = 9600;
const int maxIRModuleCount = 2;
const int maxIRColorCodes = 9;
const int maxPIRModuleCount = 1;
IRsend irsend;

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

void print(const char* m)
{
  Serial.println(m);
}

class PIRModule 
{
private:
  int pirPin = 0;

public: 
  void Initialize(int pirPin)
  {
    this->pirPin = pirPin;
    pinMode(pirPin, INPUT);
  }

  int GetStatus() 
  {
    return digitalRead(pirPin);
  }
};

class ModulesController 
{
public:
    IRModule irModules[maxIRModuleCount];
    PIRModule pirModules[maxPIRModuleCount];
    int irModuleSize = 0;
    int pirModuleSize = 0;

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
};

ModulesController controller;
IRModule ceilingModule("Ceiling Module");
PIRModule frontDoorModule;

void setup() 
{
  Serial.begin(baudRate);

  ceilingModule.AddCode(CodeDictionary("white", 0xF7E01F));
  ceilingModule.AddCode(CodeDictionary("green", 0xF7A05F));
  ceilingModule.AddCode(CodeDictionary("blue", 0xF7609F));
  ceilingModule.AddCode(CodeDictionary("red", 0xF720DF));
  ceilingModule.AddCode(CodeDictionary("orange", 0xF710EF));
  ceilingModule.AddCode(CodeDictionary("purple", 0xF750AF));
  ceilingModule.AddCode(CodeDictionary("pink", 0xF76897));
  ceilingModule.AddCode(CodeDictionary("off", 0xF740BF));
  ceilingModule.AddCode(CodeDictionary("on", 0xF7C03F));

  frontDoorModule.Initialize(2);
  controller.AddModule(frontDoorModule);
  controller.AddModule(ceilingModule);
}

void loop() 
{
  if (controller.DetectMotion())
  {
    controller.ChangeColor("off");
    print("Motion detected");
  }
  else
  {
    controller.ChangeColor("on");
    print("Motion no motion detected");
  }
}
