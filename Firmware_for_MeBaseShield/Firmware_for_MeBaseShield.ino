/*************************************************************************
* File Name          : Firmware.ino
* Author             : Ander
* Updated            : Ander
* Version            : V1.1.0
* Date               : 11/20/2014
* Description        : Firmware for Makeblock Electronic modules with Scratch.  
* License            : CC-BY-SA 3.0
* Copyright (C) 2013 - 2014 Maker Works Technology Co., Ltd. All right reserved.
* http://www.makeblock.cc/
**************************************************************************/

#include <Servo.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include "MePort.h"
#include "MeServo.h" 
#include "MeDCMotor.h" 
#include "MeUltrasonic.h" 
#include "MeGyro.h"
#include "Me7SegmentDisplay.h"
#include "MeTemperature.h"
#include "MeRGBLed.h"
#include "MeInfraredReceiver.h"
//#include "MeStepper.h"  -- need to fix - BT

MeServo servo;  
MeDCMotor dc;
MeTemperature ts;
MeRGBLed led;
MeUltrasonic us;
Me7SegmentDisplay seg;
MePort generalDevice;
MeInfraredReceiver ir;
MeGyro gyro;
//MeStepper steppers[2];

SoftwareSerial sw(2,8);
//Port4 for Bluetooth

typedef struct MeModule
{
    int device;
    int port;
    int slot;
    int pin;
    int index;
    float values[3];
} MeModule;

union{
    byte byteVal[4];
    float floatVal;
    long longVal;
}val;

union{
  byte byteVal[2];
  short shortVal;
}valShort;

MeModule modules[12];
int analogs[8]={A0,A1,A2,A3,A4,A5,A6,A7};

String sVersion = "1.1119";
float mVersion = 1.1119;
boolean isAvailable = false;
boolean isBluetooth = false;

double lastTime = 0.0;
double currentTime = 0.0;

void setup(){
  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
  delay(300);
  digitalWrite(13,LOW);
  
  Serial.begin(115200);
  //Hardware Serial for usb
  Serial.println("setup");
  
  sw.begin(115200);
  //SoftwareSerial for bluetooth

}

int len = 52;
char buffer[52];
char bufferBt[52];
byte index = 0;
byte dataLen;
byte modulesLen=0;
boolean isStart = false;
unsigned char irRead;
char serialRead;
#define VERSION 0
#define ULTRASONIC_SENSOR 1
#define TEMPERATURE_SENSOR 2
#define LIGHT_SENSOR 3
#define POTENTIONMETER 4
#define JOYSTICK 5
#define GYRO 6
#define SOUND_SENSOR 7
#define RGBLED 8
#define SEVSEG 9
#define MOTOR 10
#define SERVO 11
#define ENCODER 12
#define PIRMOTION 15
#define INFRARED 16
#define LINEFOLLOWER 17
#define SHUTTER 20
#define LIMITSWITCH 21
#define DIGITAL 30
#define ANALOG 31
#define PWM 32
//#define ANGLE 33
#define SERVO_PIN 33
#define TONE 34
#define STEPPER 40
#define ENCODER 41
#define TIMER 50

#define GET 1
#define RUN 2
#define RESET 4
#define START 5


float angleServo = 90.0;

unsigned char prevc=0;

void loop(){
  currentTime = millis()/1000.0-lastTime;

  if(ir.buttonState()==1){ 
    if(ir.available()>0){
      irRead = ir.read();
    }
  }else{
    irRead = 0;
  }

  readSerial();
 
  if(isAvailable){
    Serial.println("begin");
    
    unsigned char c = serialRead&0xff;
    Serial.println(c, HEX);
    
    if(c==0x55 && isStart==false){
      if(prevc==0xff){
        index=1; 
        isStart = true;
      }
    }else{
      prevc = c;
      
      if(isStart){
        if(index==2){
          dataLen = c; 
        }else if(index>2){
          dataLen--;
        }
        
        writeBuffer(index,c);
      }
    }
     
    index++;
     
    if(index>51){
      index=0; 
      isStart=false;
    }
     
    Serial.print(isStart);    
    Serial.print("-");    
    Serial.print(dataLen);    
    Serial.print("-");    
    Serial.print(index);    
    Serial.println();    
    
    if(isStart && dataLen==0 && index>2){ 
      isStart = false;
      parseData(); 
      index=0;
    }
  }
}

unsigned char readBuffer(int index){
 return isBluetooth?bufferBt[index]:buffer[index]; 
}

void writeBuffer(int index,unsigned char c){
 if(isBluetooth){
  bufferBt[index]=c;
 }else{
  buffer[index]=c;
 } 
}

void writeEnd(){
  isBluetooth?sw.println():Serial.println(); 
}

void writeSerial(unsigned char c){
 isBluetooth?sw.write(c):Serial.write(c); 
}

void readSerial(){
  isAvailable = false;
  if(Serial.available()>0){
    isAvailable = true;
    isBluetooth = false;
    serialRead = Serial.read();
  }
//  else 
  if(sw.available()>0){
   isAvailable = true;
   isBluetooth = true;
   serialRead = sw.read();
   
  }
}

/*
ff 55 len idx action device port  slot  data a
0  1  2   3   4      5      6     7     8
*/
void parseData(){
  isStart = false;
  
  Serial.println("parseData");
  
  int i;
  for (i = 0; i < 52; i = i + 1) {
    Serial.print(readBuffer(i), HEX);
    Serial.print(" ");
  }  
  Serial.println(" buff");

  int idx = readBuffer(3);
  int action = readBuffer(4);
  int device = readBuffer(5);
  
  Serial.print("idx:");
  Serial.print(idx);
  Serial.print("-");
  Serial.print("action:");
  Serial.print(action);
  Serial.print("-");
  Serial.print("device:");
  Serial.print(device);
  Serial.println("");
    
  switch(action){
    case GET:{
      writeHead();
      writeSerial(idx);
      readSensor(device);
      writeEnd();

      /*
      if(readBuffer(4)==0){
        Serial.println("ver");
        writeSerial(0xff);
        writeSerial(0x55);
        writeSerial(0xFA);
        sendValue(mVersion);
        writeEnd();
      }else{
        int pin = readBuffer(5);
        writeSerial(0xff);
        writeSerial(0x55);
        writeSerial(readBuffer(6));
        
        readSensor(readBuffer(4),(pin&0xf0)>>4,pin&0xf,pin);
        writeEnd();
      }
      */
     }
     break;

     case RUN:{
        runModule(device);
        callOK();

        /*
        int l = readBuffer(3);
        int dataIndex = 4;

        while(l>dataIndex-4){
          int device = readBuffer(dataIndex);
          dataIndex++;
          int pin = readBuffer(dataIndex);
          int port = (pin&0xf0)>>4;
          int slot = pin&0xf;
          dataIndex++;
          MeModule module;
          module.device = device;
          module.port = port;
          module.slot = slot;
          module.pin = pin;
          
          if(device==RGBLED){
            module.index = readBuffer(dataIndex++);
            module.values[0]=readBuffer(dataIndex++);
            module.values[1]=readBuffer(dataIndex++);
            module.values[2]=readBuffer(dataIndex++);
            if(led.getPort()!=port){
              led.reset(port);
            }
            if(module.index>0){
              led.setColorAt(module.index-1,module.values[0],module.values[1],module.values[2]);
            }else{
              for(int t=0;t<led.getNumber();t++){
                led.setColorAt(t,module.values[0],module.values[1],module.values[2]);
              }
            }
            led.show();
            callOK();
          }else if(device==MOTOR){
            val.byteVal[0]=readBuffer(dataIndex++);
            val.byteVal[1]=readBuffer(dataIndex++);
            val.byteVal[2]=readBuffer(dataIndex++);
            val.byteVal[3]=readBuffer(dataIndex++);
            module.values[0]=val.floatVal;
            dc.reset(module.port);
            dc.run(module.values[0]);
            callOK();
          }else if(device==SEVSEG){
            val.byteVal[0]=readBuffer(dataIndex++);
            val.byteVal[1]=readBuffer(dataIndex++);
            val.byteVal[2]=readBuffer(dataIndex++);
            val.byteVal[3]=readBuffer(dataIndex++);
            module.values[0]=val.floatVal;
            if(seg.getPort()!=port){
               seg.reset(port);
            }
            seg.display(module.values[0]);
            callOK();
          }else if(device==SERVO){
            val.byteVal[0]=readBuffer(dataIndex++);
            val.byteVal[1]=readBuffer(dataIndex++);
            val.byteVal[2]=readBuffer(dataIndex++);
            val.byteVal[3]=readBuffer(dataIndex++);
              //module.values[0]=val.floatVal;
              //angleServo=module.values[0];
            //module.pin = servo.pin(port,slot);
           
           // if(servo.pin()!=module.pin){
              servo.attach(servo.pin(port,slot));
            //}
            servo.write(servo.pin(port,slot),(int)(val.floatVal));
            callOK();
          }else if(device==LIGHT_SENSOR||device==SHUTTER){
            val.byteVal[0]=readBuffer(dataIndex++);
            val.byteVal[1]=readBuffer(dataIndex++);
            val.byteVal[2]=readBuffer(dataIndex++);
            val.byteVal[3]=readBuffer(dataIndex++);
            module.values[0]=val.floatVal;
            if(generalDevice.getPort()!=port){
              generalDevice.reset(module.port);
            }
            generalDevice.dWrite1(val.floatVal>=1?HIGH:LOW);
            callOK();
          }else if(device==DIGITAL){
              val.byteVal[0]=readBuffer(dataIndex++);
              val.byteVal[1]=readBuffer(dataIndex++);
              val.byteVal[2]=readBuffer(dataIndex++);
              val.byteVal[3]=readBuffer(dataIndex++);
              pinMode(module.pin,OUTPUT);
              digitalWrite(module.pin,val.floatVal>=1?HIGH:LOW);         
          }else if(device==ANALOG||device==PWM){
              val.byteVal[0]=readBuffer(dataIndex++);
              val.byteVal[1]=readBuffer(dataIndex++);
              val.byteVal[2]=readBuffer(dataIndex++);
              val.byteVal[3]=readBuffer(dataIndex++);
              if(device==ANALOG){
                pin = analogs[pin];
              }
              pinMode(module.pin,OUTPUT);
              analogWrite(module.pin,val.floatVal);
              callOK();
          }else if(device==ANGLE){
              val.byteVal[0]=readBuffer(dataIndex++);
              val.byteVal[1]=readBuffer(dataIndex++);
              val.byteVal[2]=readBuffer(dataIndex++);
              val.byteVal[3]=readBuffer(dataIndex++);
              module.values[0]=val.floatVal;
              angleServo=module.values[0];
              //if(servo.pin()!=module.pin){
                servo.attach(module.pin);
              //}
              servo.write(module.pin,abs(angleServo));
              callOK();
          }else if(device==TONE){
              val.byteVal[0]=readBuffer(dataIndex++);
              val.byteVal[1]=readBuffer(dataIndex++);
              val.byteVal[2]=readBuffer(dataIndex++);
              val.byteVal[3]=readBuffer(dataIndex++);
              int toneHz = val.byteVal[1]*256+val.byteVal[0];
              int timeMs = val.byteVal[3]*255+val.byteVal[2];
              if(timeMs!=0){
                tone(module.pin,toneHz ,timeMs); 
              }else{
                noTone(module.pin); 
              }
          }
        }
      */
      }
      break;

      case RESET:{
        //reset
        dc.reset(M1);
        dc.run(0);
        dc.reset(M2);
        dc.run(0);
        dc.reset(PORT_1);
        dc.run(0);
        dc.reset(PORT_2);
        dc.run(0);
        callOK();
      }
     break;

     case START:{
        //start
        callOK();
      }
     break;
  }
}

void callOK(){
//    writeSerial(0xff);
//    writeSerial(0x55);
//    writeEnd();
}

void readModules(){
    
    writeSerial(0xff);
    writeSerial(0x55);
    writeSerial(0x1);
    if(modulesLen>0){
      for(int i=0;i<modulesLen;i++){
        MeModule module = modules[i];
        readSensor(module.device,module.port,module.slot,module.pin);
      }
    }
    writeEnd();
}

void sendValue(float value){ 
     val.floatVal = value;
     writeSerial(val.byteVal[0]);
     writeSerial(val.byteVal[1]);
     writeSerial(val.byteVal[2]);
     writeSerial(val.byteVal[3]);
}

void writeHead(){
  writeSerial(0xff);
  writeSerial(0x55);
}


void readSensor(int device,int port,int slot,int pin){
  float value=0.0;
  
  switch(device){
   case  ULTRASONIC_SENSOR:{
     if(us.getPort()!=port){
       us.reset(port);
     }
     value = us.distanceCm();
     
     Serial.println(value);
     
     sendValue(value);
   }
   break;
   case  TEMPERATURE_SENSOR:{
     if(ts.getPort()!=port||ts.getSlot()!=slot){
       ts.reset(port,slot);
     }
     value = ts.temperature();
     sendValue(value);
   }
   break;
   case  LIGHT_SENSOR:
   case  SOUND_SENSOR:
   case  POTENTIONMETER:{
     if(generalDevice.getPort()!=port){
       generalDevice.reset(port);
       pinMode(generalDevice.pin2(),INPUT);
     }
     value = generalDevice.aRead2();
     
//            Serial.print("device:");
//            Serial.print((pin&0xf0)>>4);
//            Serial.print(" - ");
//            Serial.print(pin&0xf);
//            Serial.print(" - ");
//            Serial.println(value);
//            return;
     sendValue(value);
   }
   break;
   case  JOYSTICK:{
     if(generalDevice.getPort()!=port){
       generalDevice.reset(port);
       pinMode(generalDevice.pin1(),INPUT);
       pinMode(generalDevice.pin2(),INPUT);
     }
     if(slot==1){
       value = generalDevice.aRead1();
       sendValue(value);
     }else if(slot==2){
       value = generalDevice.aRead2();
       sendValue(value);
     }
   }
   break;
   case  INFRARED:{
     if(ir.getPort()!=port){
       ir.reset(port);
     }
     sendValue(irRead);
   }
   break;
   case  PIRMOTION:{
     if(generalDevice.getPort()!=port){
       generalDevice.reset(port);
       pinMode(generalDevice.pin2(),INPUT);
     }
     value = generalDevice.dRead2();
     sendValue(value);
   }
   break;
   case  LINEFOLLOWER:{
     if(generalDevice.getPort()!=port){
       generalDevice.reset(port);
         pinMode(generalDevice.pin1(),INPUT);
         pinMode(generalDevice.pin2(),INPUT);
     }
     value = generalDevice.dRead1()*2+generalDevice.dRead2();
     sendValue(value);
   }
   break;
   case LIMITSWITCH:{
     if(generalDevice.getPort()!=port||generalDevice.getSlot()!=slot){
       generalDevice.reset(port,slot);
     }
     if(slot==1){
       pinMode(generalDevice.pin1(),INPUT_PULLUP);
       value = generalDevice.dRead1();
     }else{
       pinMode(generalDevice.pin2(),INPUT_PULLUP);
       value = generalDevice.dRead2();
     }
     sendValue(value);  
   }
   break;
   case  GYRO:{
       gyro.update();
       if(slot==1){
         value = gyro.getAngleX();
         sendValue(value);
       }else if(slot==2){
         value = gyro.getAngleY();
         sendValue(value);
       }else if(slot==3){
         value = gyro.getAngleZ();
         sendValue(value);
       }
   }
   break;
   case  VERSION:{
     sendValue(mVersion);
   }
   break;
   case  DIGITAL:{
     pinMode(pin,INPUT);
     sendValue(digitalRead(pin));
   }
   break;
   case  ANALOG:{
     pin = analogs[pin];
     pinMode(pin,INPUT);
     sendValue(analogRead(pin));
   }
   break;
  }
}


void readSensor(int device){
  /**************************************************
      ff 55 len idx action device port slot data a
      0  1  2   3   4      5      6    7    8
  ***************************************************/
  float value=0.0;
  int port,slot,pin;
  port = readBuffer(6);
  pin = port;
  switch(device){
   case  ULTRASONIC_SENSOR:{
     if(us.getPort()!=port){
       us.reset(port);
     }
     value = us.distanceCm();
     sendFloat(value);
   }
   break;
   case  TEMPERATURE_SENSOR:{
     slot = readBuffer(7);
     if(ts.getPort()!=port||ts.getSlot()!=slot){
       ts.reset(port,slot);
     }
     value = ts.temperature();
     sendFloat(value);
   }
   break;
   case  LIGHT_SENSOR:
   case  SOUND_SENSOR:
   case  POTENTIONMETER:{
     if(generalDevice.getPort()!=port){
       generalDevice.reset(port);
       pinMode(generalDevice.pin2(),INPUT);
     }
     value = generalDevice.aRead2();
     sendFloat(value);
   }
   break;
   case  JOYSTICK:{
     slot = readBuffer(7);
     if(generalDevice.getPort()!=port){
       generalDevice.reset(port);
       pinMode(generalDevice.pin1(),INPUT);
       pinMode(generalDevice.pin2(),INPUT);
     }
     if(slot==1){
       value = generalDevice.aRead1();
       sendFloat(value);
     }else if(slot==2){
       value = generalDevice.aRead2();
       sendFloat(value);
     }
   }
   break;
   case  INFRARED:{
     if(ir.getPort()!=port){
       ir.reset(port);
     }
     sendFloat(irRead);
   }
   break;
   case  PIRMOTION:{
     if(generalDevice.getPort()!=port){
       generalDevice.reset(port);
       pinMode(generalDevice.pin2(),INPUT);
     }
     value = generalDevice.dRead2();
     sendFloat(value);
   }
   break;
   case  LINEFOLLOWER:{
     if(generalDevice.getPort()!=port){
       generalDevice.reset(port);
         pinMode(generalDevice.pin1(),INPUT);
         pinMode(generalDevice.pin2(),INPUT);
     }
     value = generalDevice.dRead1()*2+generalDevice.dRead2();
     sendFloat(value);
   }
   break;
   case LIMITSWITCH:{
     slot = readBuffer(7);
     if(generalDevice.getPort()!=port||generalDevice.getSlot()!=slot){
       generalDevice.reset(port,slot);
     }
     if(slot==1){
       pinMode(generalDevice.pin1(),INPUT_PULLUP);
       value = generalDevice.dRead1();
     }else{
       pinMode(generalDevice.pin2(),INPUT_PULLUP);
       value = generalDevice.dRead2();
     }
     sendFloat(value);  
   }
   break;
   case  GYRO:{
       int axis = readBuffer(7);
       gyro.update();
       if(axis==1){
         value = gyro.getAngleX();
         sendFloat(value);
       }else if(axis==2){
         value = gyro.getAngleY();
         sendFloat(value);
       }else if(axis==3){
         value = gyro.getAngleZ();
         sendFloat(value);
       }
   }
   break;
   case  VERSION:{
     sendString(sVersion);
   }
   break;
   case  DIGITAL:{
     pinMode(pin,INPUT);
     sendFloat(digitalRead(pin));
   }
   break;
   case  ANALOG:{
     pin = analogs[pin];
     pinMode(pin,INPUT);
     sendFloat(analogRead(pin));
   }
   break;
   case TIMER:{
     sendFloat((float)currentTime);
   }
   break;
  }
}

void runModule(int device){
  //0xff 0x55 0x6 0x0 0x1 0xa 0x9 0x0 0x0 0xa 
  int port = readBuffer(6);
  int pin = port;
  switch(device){
   case MOTOR:{
     valShort.byteVal[0] = readBuffer(7);
     valShort.byteVal[1] = readBuffer(8);
     int speed = valShort.shortVal;
     dc.reset(port);
     dc.run(speed);
   } 
    break;
    case STEPPER:{
     valShort.byteVal[0] = readBuffer(7);
     valShort.byteVal[1] = readBuffer(8);
     int maxSpeed = valShort.shortVal;
     valShort.byteVal[0] = readBuffer(9);
     valShort.byteVal[1] = readBuffer(10);
     int distance = valShort.shortVal;
     if(port==PORT_1){
       // need to fix - BT
       //steppers[0] = MeStepper(PORT_1);
       //steppers[0].setMaxSpeed(maxSpeed);
      //steppers[0].moveTo(distance);
     }else if(port==PORT_2){
      //steppers[1] = MeStepper(PORT_2);
      //steppers[1].setMaxSpeed(maxSpeed);
      //steppers[1].moveTo(distance);
     }
   } 
    break;
    case ENCODER:{
      valShort.byteVal[0] = readBuffer(7);
      valShort.byteVal[1] = readBuffer(8);
      int maxSpeed = valShort.shortVal;
      valShort.byteVal[0] = readBuffer(9);
      valShort.byteVal[1] = readBuffer(10);
      int distance = valShort.shortVal;
      int slot = port;
//      if(slot==SLOT_1){
//         encoders[0].move(distance,maxSpeed);
//      }else if(slot==SLOT_2){
//         encoders[1].move(distance,maxSpeed);
//      }
    }
    break;
   case RGBLED:{
     int idx = readBuffer(7);
     int r = readBuffer(8);
     int g = readBuffer(9);
     int b = readBuffer(10);
     led.reset(port);
     if(idx>0){
       led.setColorAt(idx-1,r,g,b); 
     }else{
        // bt led.setColor(r,g,b); 
     }
     led.show();
   }
   break;
   case SERVO:{
     int slot = readBuffer(7);
     pin = slot==1?mePort[port].s1:mePort[port].s2;
     int v = readBuffer(8);
     if(v>=0&&v<=180){
       servo.attach(pin);
       //servo.write(v);
     }
   }
   break;
   case SEVSEG:{
     if(seg.getPort()!=port){
       seg.reset(port);
     }
     float v = readFloat(7);
     seg.display(v);
   }
   break;
   case LIGHT_SENSOR:{
     if(generalDevice.getPort()!=port){
       generalDevice.reset(port);
     }
     int v = readBuffer(7);
     generalDevice.dWrite1(v);
   }
   break;
   case SHUTTER:{
     if(generalDevice.getPort()!=port){
       generalDevice.reset(port);
     }
     int v = readBuffer(7);
     if(v<2){
       generalDevice.dWrite1(v);
     }else{
       generalDevice.dWrite2(v-2);
     }
   }
   break;
   case DIGITAL:{
     pinMode(pin,OUTPUT);
     int v = readBuffer(7);
     digitalWrite(pin,v);
   }
   break;
   case PWM:{
     pinMode(pin,OUTPUT);
     int v = readBuffer(7);
     analogWrite(pin,v);
   }
   break;
   case TONE:{
     pinMode(pin,OUTPUT);
     int hz = readShort(7);
     int ms = readShort(9);
     if(ms>0){
       tone(pin, hz, ms); 
     }else{
       noTone(pin); 
     }
   }
   break;
   case SERVO_PIN:{
     int v = readBuffer(7);
     if(v>=0&&v<=180){
       servo.attach(pin);
       //servo.write(v);
     }
   }
   break;
   case TIMER:{
    lastTime = millis()/1000.0; 
   }
   break;
  }
}

void sendFloat(float value){ 
     writeSerial(0x2);
     val.floatVal = value;
     writeSerial(val.byteVal[0]);
     writeSerial(val.byteVal[1]);
     writeSerial(val.byteVal[2]);
     writeSerial(val.byteVal[3]);
}

void sendString(String s){
  int l = s.length();
  writeSerial(4);
  writeSerial(l);
  for(int i=0;i<l;i++){
    writeSerial(s.charAt(i));
  }
}

float readFloat(int idx){
  val.byteVal[0] = readBuffer(idx);
  val.byteVal[1] = readBuffer(idx+1);
  val.byteVal[2] = readBuffer(idx+2);
  val.byteVal[3] = readBuffer(idx+3);
  return val.floatVal;
}

short readShort(int idx){
  valShort.byteVal[0] = readBuffer(idx);
  valShort.byteVal[1] = readBuffer(idx+1);
  return valShort.shortVal; 
}



