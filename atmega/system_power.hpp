#pragma once

class SystemPower {  
private:
  bool powerDown;
public:
  SystemPower();

  inline void setPowerDownBtn(bool p) {powerDown = p;};
  
  void begin();
  void sendState();
};
