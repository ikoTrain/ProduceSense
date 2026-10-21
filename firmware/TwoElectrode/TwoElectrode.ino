#include <Arduino.h>
// GPIO5 -- 10k -- junction(GPIO4, electrode A); electrode B -- GPIO6.
// Exploratory transient measurements, not calibrated impedance spectroscopy.
constexpr int DRIVE = 5, SENSE = 4, RETURN_PIN = 6;
constexpr uint32_t offsets[] = {1000, 5000, 20000};
bool recording = false;
uint32_t started = 0, nextCycle = 0, cycleNumber = 0;
struct Sample { uint32_t elapsed, raw, mv; };
void idle() { digitalWrite(DRIVE, LOW); digitalWrite(RETURN_PIN, LOW); }
void pulse(bool reverse, Sample* samples) {
  idle(); delay(30);
  uint32_t begin = micros();
  digitalWrite(reverse ? RETURN_PIN : DRIVE, HIGH);
  for (int i=0; i<3; ++i) {
    while ((uint32_t)(micros()-begin) < offsets[i]) { delayMicroseconds(10); }
    samples[i].elapsed = micros()-begin;
    samples[i].raw = analogRead(SENSE);
    samples[i].mv = analogReadMilliVolts(SENSE);
  }
  idle();
}
void setup() {
  digitalWrite(DRIVE, LOW); digitalWrite(RETURN_PIN, LOW);
  pinMode(DRIVE, OUTPUT); pinMode(RETURN_PIN, OUTPUT);
  pinMode(SENSE, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(SENSE, ADC_11db);
  Serial.begin(115200);
  rgbLedWrite(48,0,0,0);
  delay(500);
  analogRead(SENSE); analogReadMilliVolts(SENSE);
  Serial.println("# ProduceSense v3 two-electrode; 10k REQUIRED; r=15s capture s=stop i=info");
}
void loop() {
  while (Serial.available()) {
    char c=Serial.read();
    if(c=='s') { recording=false; idle(); Serial.println("# stopped"); }
    if(c=='i') Serial.println("# v3; GPIO5--10k--A+GPIO4; B--GPIO6; raw and mV are sequential conversions; rail readings invalid for resistance");
    if(c=='r') {
      started=millis(); nextCycle=started; cycleNumber=0; recording=true;
      Serial.println("cycle,direction,target_us,actual_us,raw,mv,rail");
    }
  }
  if(recording && (uint32_t)(millis()-started)>=15000) {
    recording=false; idle(); Serial.println("# done");
  }
  if(recording && (int32_t)(millis()-nextCycle)>=0) {
    Sample forward[3], reverse[3];
    pulse(false, forward); pulse(true, reverse);
    for(int d=0; d<2; ++d) {
      Sample* s=d?reverse:forward;
      for(int i=0;i<3;++i)
        Serial.printf("%lu,%c,%lu,%lu,%lu,%lu,%d\n",(unsigned long)cycleNumber,d?'R':'F',
          (unsigned long)offsets[i],(unsigned long)s[i].elapsed,(unsigned long)s[i].raw,
          (unsigned long)s[i].mv,s[i].raw<20 || s[i].raw>4075);
    }
    ++cycleNumber; nextCycle=millis()+400;
  }
  delay(1);
}
