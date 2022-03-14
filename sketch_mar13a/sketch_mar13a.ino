#include <IRremote.h>

int baudRate = 9600;
const int maxModuleCount = 2;
const int maxColorCodes = 9;
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
  CodeDictionary codes[maxColorCodes];

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

class IRController 
{
public:
    IRModule modules[maxModuleCount];
    int size = 0;

    void AddModule(IRModule module)
    {
      modules[size] = module;
      size++;
    }

    void ChangeColor(const char* color)
    {
      for (int i = 0; i < maxModuleCount; i++)
      {
        irsend.sendNEC(modules[i].FindCode(color).code, 32);
      }
    }
};

IRController controller;
IRModule ceilingModule("Ceiling Module");

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
  controller.AddModule(ceilingModule);
}

void loop() 
{
  controller.ChangeColor("purple");
}
