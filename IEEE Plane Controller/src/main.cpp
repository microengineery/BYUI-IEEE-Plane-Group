#include <Arduino.h>
#include <AlfredoCRSF.h>
#include <Servo.h>

//#define calibrate

// put function declarations here:
int myFunction(int, int);
AlfredoCRSF crsf;
// Servos on 2,3,4,5,6,7,10,11,12,13
// escs on 14,15
Servo lEsc;
Servo rEsc;
Servo tser;
Servo tser2;
Servo servo13; //rn This is caster steering
//Servo servo[10];

//int servoPins[10] = {2, 3, 4, 5, 6, 7, 10, 11, 12, 13};
int cnt = 0;
unsigned long now;
unsigned long then;
bool armed = false;
bool rearmed = false;
bool state;
bool state2;
int throttle;
bool state3;
float cpuTemp;

void setup()
{
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
  pinMode(25, OUTPUT);
  pinMode(22, OUTPUT);
  pinMode(21, OUTPUT);
  pinMode(20, OUTPUT);
  Serial.begin(115200);
  Serial2.setTX(8);
  Serial2.setRX(9);
  Serial2.begin(CRSF_BAUDRATE);
  crsf.begin(Serial2);
  lEsc.attach(14, 1000, 2000);
  rEsc.attach(15, 1000, 2000);
  tser.attach(2, 700, 2300);
  tser2.attach(3, 700, 2300);
  servo13.attach(13, 700, 2300);
  #ifdef calibrate
  lEsc.write(2000);
  rEsc.write(2000);
  delay(3000);
  lEsc.write(1500);
  rEsc.write(1500);
  #endif
  digitalWrite(20, HIGH);
  delay(250);
  digitalWrite(20, LOW);
  digitalWrite(21, HIGH);
  delay(250);
  digitalWrite(21,LOW);
  digitalWrite(22, HIGH);
  delay(250);
  digitalWrite(22, LOW);
  delay(500);
}
// all of this code was writen by me except for included libs unless otherwise stated by small errors were debugged with the help of AI
// code that is part of pio/arduino default project setup was also not written by me
void loop()
{
  // put your main code here, to run repeatedly:
  cpuTemp = analogReadTemp();
  now = millis();
  throttle = map(crsf.getChannel(4),988,2012,1000,2000);
  crsf.update();
  if (crsf.isLinkUp() && armed)
  {
    if (labs(now - then - 225) <= 75 || labs(now - then - 525) <= 75)
    {
      digitalWrite(25, HIGH);
      digitalWrite(21, HIGH);
    }
    else
    {
      digitalWrite(25, LOW);
      digitalWrite(21, LOW);
    }

    if (now - then >= 1625)
    {
      then = now;
    }
  digitalWrite(22, LOW);
  digitalWrite(20, LOW);
  }
  else if (!crsf.isLinkUp() && armed){
    digitalWrite(20, HIGH);
    digitalWrite(21, LOW);
  }
  else if (crsf.isLinkUp())
  {
    digitalWrite(25, HIGH);
    digitalWrite(22, HIGH);
    digitalWrite(21, LOW);
  }
  else
  {
    digitalWrite(20, LOW);
    digitalWrite(25, LOW);
    digitalWrite(22, HIGH);
    digitalWrite(21, HIGH);
  }
  
  if(now - then == 1000) {
    Serial.println(cpuTemp);
    Serial.println(cpuTemp * 9 / 5 + 32);
  }
  //decides whether to "arm" the craft or not
  if (crsf.getChannel(5) > 1500 ^ state)
  {
    if (crsf.getChannel(11) > 1500 && (crsf.getChannel(5) > 1500))
    {
      armed = true;
      Serial.println("armed");
    }
    else if (armed)
    {
      armed = false;
      Serial.println("disarmed");
    }
  }
  state = crsf.getChannel(5) > 1500;
  
  if(armed && (crsf.getChannel(10) > 1500 ^ state2)) {
    if(crsf.getChannel(4) < 1000) {
      rearmed = true;
    }
  } 
  else if (!armed || (crsf.getChannel(10) < 1500 && crsf.getChannel(4) < 1000))
  {
    rearmed = false;
  }
  

  //this handles control of the escs when armed vs disarmed
  //writeMicroseconds: 1000 = backwards   1500 = stay still   2000 = forwards
  if(rearmed) {
    rEsc.writeMicroseconds(map(crsf.getChannel(4), 988, 2012, 1500, 1000));
    lEsc.writeMicroseconds(map(crsf.getChannel(4), 988, 2012, 1500, 1000));
  }
  else if (armed)
  {
    tser.write(map(crsf.getChannel(4), 988, 2012, 0, 180));
    // This thing basically just is google translate between the video game controller and the crsf
    rEsc.writeMicroseconds(constrain(map(crsf.getChannel(4), 988, 2012, 1500, 2000) + constrain(map(crsf.getChannel(3),988,2012,throttle/2, -throttle/2), -250, 250),1500,2000));
    lEsc.writeMicroseconds(constrain(map(crsf.getChannel(4), 988, 2012, 1500, 2000) - constrain(map(crsf.getChannel(3),988,2012,throttle/2, -throttle/2), -250, 250),1500,2000));
  }
  else
  {
    lEsc.writeMicroseconds(1500);
    rEsc.writeMicroseconds(1500);
    tser.write(0);
  }

  tser2.write(map(crsf.getChannel(2), 988, 2012, 0, 180));
  if(crsf.getChannel(1) > 500) {
    servo13.write(map(crsf.getChannel(1), 988, 2012, 161, 19));
  }


  //this detaches the esc so that they start beeping and retaches them when ch10 is released
  if (crsf.getChannel(10) > 1500 ^ state2)
  {
    if (crsf.getChannel(12) > 1500 && crsf.getChannel(10) > 1500)
    {
      tser2.detach();
      lEsc.detach();
      rEsc.detach();
      Serial.println("detached");
    }
    else if (crsf.getChannel(10) < 1500 && !lEsc.attached() && !rEsc.attached())
    {
      tser2.attach(3, 700, 2300);
      lEsc.attach(14, 1000, 2000);
      rEsc.attach(15, 1000, 2000);
      Serial.println("attached");
    }
  }

  //update state variables
  state2 = crsf.getChannel(10) > 1500;
  state3 = crsf.getChannel(9) > 1500;

}

// put function definitions here:
int myFunction(int x, int y)
{
  return x + y;
}