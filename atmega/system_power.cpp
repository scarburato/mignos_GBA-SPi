//#include <DynamicHID/DynamicHID.h>
#include "HID.hpp"
#include "system_power.hpp"

static const uint8_t _hidReportDescriptor[] PROGMEM = {
    // ================= Pulsante di spegnimento
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x80,                    // USAGE (System Control)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, 0xaa,                    //   REPORT_ID (170)
    0x09, 0x81,                    //   USAGE (System Power Down)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0xc0,                          // END_COLLECTION
};

SystemPower::SystemPower(){
  powerDown = false;
  
  // report
  //static DynamicHIDSubDescriptor node(_hidReportDescriptor, sizeof(_hidReportDescriptor), true);
  static HIDSubDescriptor node(_hidReportDescriptor, sizeof(_hidReportDescriptor));
  HID().AppendDescriptor(&node);
}

void SystemPower::begin() {  
  sendState();
}

void SystemPower::sendState() {
  HID().SendReport(0xaa, &powerDown, 1);
}
