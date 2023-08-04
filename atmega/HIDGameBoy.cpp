#include "HIDGameBoy.hpp"

// HID Usage Tables: 1.4.0
// Descriptor size: 46 (bytes)
// +----------+-------+-------------------+
// | ReportId | Kind  | ReportSizeInBytes |
// +----------+-------+-------------------+
// |        1 | Input |                 2 |
// +----------+-------+-------------------+
static const uint8_t _hidReportDescriptor[] PROGMEM = {
  0x05, 0x01,          // UsagePage(Generic Desktop[0x0001])
  0x09, 0x05,          // UsageId(Gamepad[0x0005])
  0xA1, 0x01,          // Collection(Application)
  0x85, 0x55,          //     ReportId(55)
  0x05, 0x09,          //     UsagePage(Button[0x0009])
  0x19, 0x01,          //     UsageIdMin(Button 1[0x0001])
  0x29, 0x06,          //     UsageIdMax(Button 6[0x0006])
  0x15, 0x00,          //     LogicalMinimum(0)
  0x25, 0x01,          //     LogicalMaximum(1)
  0x95, 0x06,          //     ReportCount(6)
  0x75, 0x01,          //     ReportSize(1)
  0x81, 0x02,          //     Input(Data, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, BitField)
  0x05, 0x01,          //     UsagePage(Generic Desktop[0x0001])
  0x09, 0x39,          //     UsageId(Hat Switch[0x0039])
  0x46, 0x3B, 0x01,    //     PhysicalMaximum(315)
  0x65, 0x14,          //     Unit('degrees', EnglishRotation, Degrees:1)
  0x25, 0x07,          //     LogicalMaximum(7)
  0x95, 0x01,          //     ReportCount(1)
  0x75, 0x03,          //     ReportSize(3)
  0x81, 0x02,          //     Input(Data, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, BitField)
  0x75, 0x07,          //     ReportSize(7)
  0x81, 0x03,          //     Input(Constant, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, BitField)
  0xC0,                // EndCollection()
};

HIDGameBoy::HIDGameBoy(){
  
  // report
  //static DynamicHIDSubDescriptor node(_hidReportDescriptor, sizeof(_hidReportDescriptor), true);
  static HIDSubDescriptor node(_hidReportDescriptor, sizeof(_hidReportDescriptor));
  HID().AppendDescriptor(&node);
}

void HIDGameBoy::begin() {
  sendState();
}

void HIDGameBoy::sendState() {
  HID().SendReport(0x55, &(uint8_t[]){buttons, hat}, 2);
}