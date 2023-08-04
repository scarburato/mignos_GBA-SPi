/**
 * Copyright (c) Dario Pagani 2023. All right reserved.
 * 
 * All code regarding power supply:
 * HID UPS Power Device Library for Arduin
 * 
 * Copyright (c) Alex Bratchik 2020. All right reserved.
 */

#include <avr/sleep.h>

#include "HIDPowerDevice.hpp"
#include "system_power.hpp"
#include "HIDGameBoy.hpp"

int iIntTimer=0;


// String constants 
// https://gitlab.freedesktop.org/upower/upower/-/blob/master/src/up-common.c#L56
const char STRING_DEVICECHEMISTRY[] PROGMEM = "lipo";
const char STRING_OEMVENDOR[] PROGMEM = "GBPI-power";
const char STRING_SERIAL[] PROGMEM = "0001"; 

const byte bDeviceChemistry = IDEVICECHEMISTRY;
const byte bOEMVendor = IOEMVENDOR;

uint16_t iPresentStatus = 0, iPreviousStatus = 0;

byte bRechargable = 1;
byte bCapacityMode = 2;  // units are in %%

// Physical parameters
const uint16_t iConfigVoltage = 420;
uint16_t iVoltage =400, iPrevVoltage = 0;
uint16_t iRunTimeToEmpty = 0, iPrevRunTimeToEmpty = 0;
uint16_t iAvgTimeToFull = 7200;
uint16_t iAvgTimeToEmpty = 7200;
uint16_t iRemainTimeLimit = 600;
int16_t  iDelayBe4Reboot = -1;
int16_t  iDelayBe4ShutDown = -1;

byte iAudibleAlarmCtrl = 2; // 1 - Disabled, 2 - Enabled, 3 - Muted


// Parameters for ACPI compliancy
const byte iDesignCapacity = 100;
byte iWarnCapacityLimit = 10; // warning at 10% 
byte iRemnCapacityLimit = 5; // low at 5% 
const byte bCapacityGranularity1 = 1;
const byte bCapacityGranularity2 = 1;
byte iFullChargeCapacity = 100;

byte iRemaining =0, iPrevRemaining=0;

int iRes=0;

// Pins
constexpr unsigned PIN_POWER_MAIN_SWITCH = 13;
constexpr unsigned PIN_POWER_BUTTON = 2;
constexpr unsigned PIN_MCP73871_STAT1 = 3;
constexpr unsigned PIN_MCP73871_STAT2 = 4;
constexpr unsigned PIN_MCP73871_NOT_PG = 5;
constexpr unsigned PIN_GB_HAT[] = {6,7,8,9};
constexpr unsigned PIN_GB_SELECT = 10;
constexpr unsigned PIN_GB_START = 11;
constexpr unsigned PIN_GB_A = 12;
constexpr unsigned PIN_GB_B = 12; // fixme

constexpr unsigned PIN_BATT_VOLTAGE = A0;
constexpr unsigned PIN_IN_CURR = A1;

// Power button
SystemPower pwrBtn;

// GameBoy controller
HIDGameBoy gbController;

// State machine's consts in us
constexpr unsigned long PERIOD_PS_EVENT = 500l*1000l;
constexpr unsigned long PERIOD_GAMEPAD_EVENT = 500l*1000l;

// State machine
unsigned long time_last_ps_event = 0;
unsigned long time_last_gamepad_event = 0;


void setup() {
  Serial.begin(9600);
  
  pinMode(PIN_POWER_MAIN_SWITCH, OUTPUT);

  pinMode(PIN_POWER_BUTTON, INPUT_PULLUP);
  pinMode(PIN_MCP73871_STAT1, INPUT_PULLUP);
  pinMode(PIN_MCP73871_STAT2, INPUT_PULLUP);
  pinMode(PIN_MCP73871_NOT_PG, INPUT_PULLUP);
  pinMode(PIN_GB_SELECT, INPUT_PULLUP);
  pinMode(PIN_GB_START, INPUT_PULLUP);
  pinMode(PIN_GB_A, INPUT_PULLUP);
  pinMode(PIN_GB_B, INPUT_PULLUP);

  for(auto i = 0; i < 4; i++)
    pinMode(PIN_GB_HAT[i], INPUT_PULLUP);


  // =========================================== Power button stuff ====
  pwrBtn.begin();

  // =========================================== Power supply stuff ====
  
  // Serial No is set in a special way as it forms Arduino port name
  PowerDevice.setSerial(STRING_SERIAL); 

  PowerDevice.setFeature(HID_PD_PRESENTSTATUS, &iPresentStatus, sizeof(iPresentStatus));
  
  PowerDevice.setFeature(HID_PD_RUNTIMETOEMPTY, &iRunTimeToEmpty, sizeof(iRunTimeToEmpty));
  PowerDevice.setFeature(HID_PD_AVERAGETIME2FULL, &iAvgTimeToFull, sizeof(iAvgTimeToFull));
  PowerDevice.setFeature(HID_PD_AVERAGETIME2EMPTY, &iAvgTimeToEmpty, sizeof(iAvgTimeToEmpty));
  PowerDevice.setFeature(HID_PD_REMAINTIMELIMIT, &iRemainTimeLimit, sizeof(iRemainTimeLimit));
  PowerDevice.setFeature(HID_PD_DELAYBE4REBOOT, &iDelayBe4Reboot, sizeof(iDelayBe4Reboot));
  PowerDevice.setFeature(HID_PD_DELAYBE4SHUTDOWN, &iDelayBe4ShutDown, sizeof(iDelayBe4ShutDown));
  
  PowerDevice.setFeature(HID_PD_RECHARGEABLE, &bRechargable, sizeof(bRechargable));
  PowerDevice.setFeature(HID_PD_CAPACITYMODE, &bCapacityMode, sizeof(bCapacityMode));
  PowerDevice.setFeature(HID_PD_CONFIGVOLTAGE, &iConfigVoltage, sizeof(iConfigVoltage));
  PowerDevice.setFeature(HID_PD_VOLTAGE, &iVoltage, sizeof(iVoltage));

  PowerDevice.setStringFeature(HID_PD_IDEVICECHEMISTRY, &bDeviceChemistry, STRING_DEVICECHEMISTRY);
  PowerDevice.setStringFeature(HID_PD_IOEMINFORMATION, &bOEMVendor, STRING_OEMVENDOR);

  PowerDevice.setFeature(HID_PD_AUDIBLEALARMCTRL, &iAudibleAlarmCtrl, sizeof(iAudibleAlarmCtrl));

  PowerDevice.setFeature(HID_PD_DESIGNCAPACITY, &iDesignCapacity, sizeof(iDesignCapacity));
  PowerDevice.setFeature(HID_PD_FULLCHRGECAPACITY, &iFullChargeCapacity, sizeof(iFullChargeCapacity));
  PowerDevice.setFeature(HID_PD_REMAININGCAPACITY, &iRemaining, sizeof(iRemaining));
  PowerDevice.setFeature(HID_PD_WARNCAPACITYLIMIT, &iWarnCapacityLimit, sizeof(iWarnCapacityLimit));
  PowerDevice.setFeature(HID_PD_REMNCAPACITYLIMIT, &iRemnCapacityLimit, sizeof(iRemnCapacityLimit));
  PowerDevice.setFeature(HID_PD_CPCTYGRANULARITY1, &bCapacityGranularity1, sizeof(bCapacityGranularity1));
  PowerDevice.setFeature(HID_PD_CPCTYGRANULARITY2, &bCapacityGranularity2, sizeof(bCapacityGranularity2));

  // =========================================== Gameboy stuff
  gbController.begin();
}

void loop() {
  const unsigned long current_time = micros();

  // =========================================== POWER BUTTON
  pwrBtn.setPowerDownBtn(digitalRead(PIN_POWER_BUTTON));
  //pwrBtn.sendState();

  if(!time_last_ps_event || time_last_ps_event + PERIOD_PS_EVENT < current_time) {
    time_last_ps_event = current_time;

    Serial.println("dentro");

    //*********** MCP73871 data ***************************
    uint8_t charger_status = 
              (digitalRead(PIN_MCP73871_STAT1) << 2) +
              (digitalRead(PIN_MCP73871_STAT2) << 1) +
              (digitalRead(PIN_MCP73871_NOT_PG));

    // @TODO Set iPresentStatus = 0 and only do sets
    iPresentStatus = 0;
    
    bitSet(iPresentStatus,PRESENTSTATUS_BATTPRESENT);
    switch(charger_status) {
    // (Not charging and no AC) or no input power(?)
    case 0b111:
    case 0b110: // This is also no battery???
      bitClear(iPresentStatus,PRESENTSTATUS_BELOWRCL);
      bitClear(iPresentStatus,PRESENTSTATUS_CHARGING);
      bitClear(iPresentStatus,PRESENTSTATUS_ACPRESENT);
      bitClear(iPresentStatus,PRESENTSTATUS_FULLCHARGE);
      bitSet(iPresentStatus,PRESENTSTATUS_DISCHARGING);
      break;
    // Charging
    case 0b010:
      bitClear(iPresentStatus,PRESENTSTATUS_BELOWRCL);
      bitSet(iPresentStatus,PRESENTSTATUS_CHARGING);
      bitSet(iPresentStatus,PRESENTSTATUS_ACPRESENT);
      bitClear(iPresentStatus,PRESENTSTATUS_FULLCHARGE);
      bitClear(iPresentStatus,PRESENTSTATUS_DISCHARGING);
      break;
    // Charge complete
    case 0b100:
      bitClear(iPresentStatus,PRESENTSTATUS_BELOWRCL);
      bitClear(iPresentStatus,PRESENTSTATUS_CHARGING);
      bitSet(iPresentStatus,PRESENTSTATUS_ACPRESENT);
      bitSet(iPresentStatus,PRESENTSTATUS_FULLCHARGE);
      bitClear(iPresentStatus,PRESENTSTATUS_DISCHARGING);
      break;
    // Fault
    case 0b000:
      // IDK? Check if HID has error reporting
      break;
    // Low battery
    case 0b011:
      bitSet(iPresentStatus,PRESENTSTATUS_BELOWRCL);
      bitClear(iPresentStatus,PRESENTSTATUS_CHARGING);
      bitClear(iPresentStatus,PRESENTSTATUS_ACPRESENT);
      bitClear(iPresentStatus,PRESENTSTATUS_FULLCHARGE);
      bitSet(iPresentStatus,PRESENTSTATUS_DISCHARGING);
      break;
    default:
      // ????
      break;
    }
      
    //*********** Measurements ****************************
    int iA7 = analogRead(PIN_BATT_VOLTAGE);       // TODO - this is for debug only. Replace with charge estimation
  
    // Assuming 3bit precision (Leonardo)
    iVoltage = (iA7 * 500l) / 0x3ffl;
    // Assuming linear relation between 15% to 100%
    iRemaining = 66;//1.735*(float)iVoltage - 628.6;
    iRemaining = min(iRemaining, 100);
    iRemaining = max(iRemaining, 0);
    iRunTimeToEmpty = (uint16_t)round((float)iAvgTimeToEmpty*iRemaining/100);
    
    
    // *********** Reports goin' out ****************************
    PowerDevice.sendReport(HID_PD_REMAININGCAPACITY, &iRemaining, sizeof(iRemaining));

    // @TODO Remove estimated runtime from HID
    PowerDevice.sendReport(HID_PD_RUNTIMETOEMPTY, &iRunTimeToEmpty, sizeof(iRunTimeToEmpty));
    
    PowerDevice.sendReport(HID_PD_PRESENTSTATUS, &iPresentStatus, sizeof(iPresentStatus));
    
    PowerDevice.sendReport(HID_PD_VOLTAGE, &iVoltage, sizeof(iVoltage));
        
    iIntTimer = 0;
    iPreviousStatus = iPresentStatus;
    iPrevRemaining = iRemaining;
    iPrevRunTimeToEmpty = iRunTimeToEmpty;
    
    return;
  }

  if(time_last_gamepad_event + PERIOD_GAMEPAD_EVENT < current_time) {
    time_last_gamepad_event = current_time;

    static bool s = true;
    gbController.setHat(s ? HIDGameBoy::HATSWITCH_UP : HIDGameBoy::HATSWITCH_NONE);
    s = !s;
    gbController.sendState();
    return;
  }

  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_mode();
}
