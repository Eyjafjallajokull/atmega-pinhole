#include <FiniteStateMachine.h>
// #include <Bounce.h>
#include <CmdMessenger.h>
#include <Servo.h>
#include <Stepper.h>
#include <avr/eeprom.h>

//  HARDWARE  ///////////////////////////////////////////////////////////////////////////

int lightMeterPin = A0;
int statusLedPin = 13;
int frameSizeLedPin = 12;

int resetFrameCountSwitchPin = 9;
int frameSizeSwitchPin = 10; // nie uzywane
//Bounce frameSizeSwitch = Bounce(frameSizeSwitchPin, frameSizeSwitchPin); 
int shutterSwitchPin = 11; 
//Bounce shutterSwitch = Bounce(shutterSwitchPin, shutterSwitchPin); 

int shutterServoPin = 3;
Servo shutterServo;
int winderStepperSpeed = 150;
Stepper winderStepper(64, 5, 6, 7, 8);


//  CAMERA PARAMETERS  //////////////////////////////////////////////////////////////////


int cameraConfig[6];
enum {
  kCCUseLightMeter = 1,
  kCCManualShutterValue = 2,
  kCCAutoWind = 3,
  kCCShots = 4,
  kCCPinholeSize = 5
};
void initializeCameraConfig() {
  cameraConfig[kCCUseLightMeter] = 0;
  cameraConfig[kCCManualShutterValue] = 500;
  cameraConfig[kCCAutoWind] = 1;
  cameraConfig[kCCShots] = 0;
  cameraConfig[kCCPinholeSize] = 2;

  uint16_t* address;
  for (int value, param=1, n=getConfigLength(); param<n; param++) {
    address = getEppromAddress(param);
    value = eeprom_read_word(address);
    Serial.println(value);
    if (value == -1) { // epprom initial value
      eeprom_write_word(address, value);
    } else {
      cameraConfig[param] = value;
    }
  }
}
int getConfigLength() {
  return sizeof(cameraConfig) / sizeof(int);
}
int getCameraConfig(int param) {
  return cameraConfig[param];
}
int setCameraConfig(int param, int value) {
  if (getCameraConfig(param) != value) {
    cameraConfig[param] = value;
    eeprom_write_word(getEppromAddress(param), value);
  }
}
uint16_t* getEppromAddress(int param) {
  return (uint16_t*)(param * 2 - 1);
}


//  STATE MACHINE  //////////////////////////////////////////////////////////////////////

bool smallFrameSize = false;
State CSStart = State(CSStartUpdate);
State CSIdle = State(CSIdleEnter, CSIdleUpdate);
State CSTakePicture = 
  State(CSTakePictureEnter, CSTakePictureUpdate, CSTakePictureExit);
State CSWindFilm = 
  State(CSWindFilmEnter, CSWindFilmUpdate, CSWindFilmExit);
State CSResetFrameCount = State(CSResetFrameCountUpdate);

FSM camera = FSM(CSStart);


//  SERIAL KILLER  //////////////////////////////////////////////////////////////////////


enum {
  kSKMsgAck = 1,
  kSKMsgErr = 3,
  kSKMsgSetConfig = 4,
  kSKMsgGetConfig = 5,
  kSKMsgConfigValue = 6,
  kSKMsgState = 7
};
enum {
  kSKValueIdle = 1,
  kSKValueTakePicture = 2,
  kSKValueReset = 3
};
CmdMessenger serialKiller = CmdMessenger(Serial);
void unknownCmd() {
  serialKiller.sendCmd(kSKMsgErr,"Unknown command");
}
void serialKillerCallbackSetConfig() {
  int parameter = serialKiller.readInt();
  int value = serialKiller.readInt();
  if (!camera.isInState(CSIdle)) {
    serialKiller.sendCmd(kSKMsgErr, "camera is busy");
    return;
  }
  if (parameter == 0 || getConfigLength() <= parameter) {
    serialKiller.sendCmd(kSKMsgErr, "invalid parameter id");
    return;
  }
  setCameraConfig(parameter, value);
  serialKiller.sendCmd(kSKMsgAck, "ok");
}
void serialKillerCallbackGetConfig() {
  char msg[20], param[8], value[8];
  for (int i=1, n=getConfigLength(); i<n; i++) {
    strncpy(msg, "", 1);
    itoa(i, param, 10);
    itoa(getCameraConfig(i), value, 10);
    strncat(msg, param, 7);
    strncat(msg, " ", 1);
    strncat(msg, value, 7);
    serialKiller.sendCmd(kSKMsgConfigValue, msg);
  }
}
void serialKillerStateChangeNotify(int stateId) {
  char sStateId[8];
  itoa(stateId, sStateId, 10);
  if (sizeof(cameraConfig) / sizeof(int) > stateId) {
    serialKiller.sendCmd(kSKMsgState, sStateId);
  }
}
void spawnSerialKiller() {
  Serial.begin(115200);
  serialKiller.print_LF_CR();
  serialKiller.attach(unknownCmd);
  serialKiller.attach(kSKMsgSetConfig, serialKillerCallbackSetConfig);
  serialKiller.attach(kSKMsgGetConfig, serialKillerCallbackGetConfig);
}


//  MAIN  ///////////////////////////////////////////////////////////////////////////////


void setup() {
  initializeCameraConfig();
  spawnSerialKiller();
  pinMode(statusLedPin, OUTPUT);
  pinMode(frameSizeLedPin, OUTPUT);
  pinMode(shutterServoPin, OUTPUT);
  pinMode(frameSizeSwitchPin, INPUT);
  pinMode(shutterSwitchPin, INPUT);
  pinMode(resetFrameCountSwitchPin, INPUT);
  winderStepper.setSpeed(winderStepperSpeed);
  shutterServo.attach(shutterServoPin);
  shutterServo.write(0);
}
 

void loop() {
  camera.update();
  serialKiller.feedinSerialData();
}


//  STATE MACHINE FUCKTIONS  ////////////////////////////////////////////////////////////


bool blinkerLedOn = false;
int blinkerTimeOfLastSwitch = -1;
void blinkerInit() {
  blinkerTimeOfLastSwitch = -1;
  blinkerLedOn = true;
}
void blinker(int timeInCurrentState, int delay) {
  int lastStep = (timeInCurrentState / delay) * delay;
  if (blinkerTimeOfLastSwitch < lastStep) {
    blinkerTimeOfLastSwitch = timeInCurrentState;
    blinkerLedOn = !blinkerLedOn;
    digitalWrite(statusLedPin, blinkerLedOn ? HIGH : LOW);
  }
}
void blinkerFinish() {
  digitalWrite(statusLedPin, HIGH);
}


void CSStartUpdate() {
  if (digitalRead(frameSizeSwitchPin) == HIGH) {
    digitalWrite(frameSizeLedPin, HIGH);
    smallFrameSize = true;
  }
  serialKiller.sendCmd(kSKMsgAck, "ready");
  digitalWrite(statusLedPin, HIGH); delay(100);
  digitalWrite(statusLedPin, LOW); delay(100);
  digitalWrite(statusLedPin, HIGH);
  camera.transitionTo(CSIdle);
}


void CSIdleEnter() {
  serialKillerStateChangeNotify(kSKValueIdle);
}
void CSIdleUpdate() {
  if (digitalRead(shutterSwitchPin) == HIGH) {
    camera.transitionTo(CSTakePicture);
  }
  if (digitalRead(resetFrameCountSwitchPin) == HIGH) {
    camera.transitionTo(CSResetFrameCount);
  }
}


int _currentShutterValue;
int _calculateShutterValue() {
  if (cameraConfig[kCCUseLightMeter]) {
    _currentShutterValue = analogRead(lightMeterPin); // 0 - 1023
    int aperture = 1; // średnica otworu
    int focalLength = 23; // odległość kliszy od otworu
    int fnumber = focalLength / aperture;
    int maxShutter = 10000;
    _currentShutterValue = map(_currentShutterValue, 0, 1023, 3000, 100);
  } else {
    _currentShutterValue = cameraConfig[kCCManualShutterValue];
  }
}
void CSTakePictureEnter() {
  serialKillerStateChangeNotify(kSKValueTakePicture);
  shutterServo.write(20);
  _currentShutterValue = _calculateShutterValue();
  Serial.println(_currentShutterValue);
  blinkerInit();
}
void CSTakePictureUpdate() {
  int timeInState = camera.timeInCurrentState();
  blinker(timeInState, 100);
  if (timeInState > _currentShutterValue) {
    if (cameraConfig[kCCAutoWind]) {
      camera.transitionTo(CSWindFilm);
    } else {
      camera.transitionTo(CSIdle);
    }
  }
}
void CSTakePictureExit() { 
  shutterServo.write(0);
  delay(50);
  blinkerFinish();
}


int winderSteps;
int winderStepsPerMove = 8;
void CSWindFilmEnter() {
  if (smallFrameSize) {
    winderSteps = 512;
  } else {
    winderSteps = 2048;
  }
  blinkerInit();
}
void CSWindFilmUpdate() {
  int timeInState = camera.timeInCurrentState();
  blinker(timeInState, 300);

  if (winderSteps > 0) {
    int stepsToStepNow = winderStepsPerMove;
    if (winderSteps < winderStepsPerMove) {
      stepsToStepNow = winderSteps;
    }
    winderSteps -= stepsToStepNow;
    winderStepper.step(stepsToStepNow);
  } else {
    camera.transitionTo(CSIdle);
  }
}
void CSWindFilmExit() {
  setCameraConfig(kCCShots, getCameraConfig(kCCShots)+1);
  blinkerFinish();
}


void CSResetFrameCountUpdate() {
  setCameraConfig(kCCShots, 0);
  digitalWrite(statusLedPin, LOW); delay(300);
  digitalWrite(statusLedPin, HIGH); delay(500);
  camera.transitionTo(CSIdle);
}