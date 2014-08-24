#ifndef _PANTILT_H_
#define _PANTILT_H_

#include "Arduino.h"
#include <Servo.h>

#define MAX_PAN 180
#define MIN_PAN 10

#define MAX_TILT 115
#define MIN_TILT 45

class PanTilt
{
  private:
    float m_pan;
    float m_panSpeed;
    float m_tilt;
    float m_tiltSpeed;
    
    long long m_lastUpdate;
    
    Servo *m_panServo;
    Servo *m_tiltServo;
  
  public:
    PanTilt(int panPin, int tiltPin)
    {
      m_lastUpdate = micros();
      m_pan = 90;
      m_panSpeed = 0;
      m_tilt = 90;
      m_tiltSpeed = 0;
      
      
      Serial.print("Creando panTilt en pins ");
      Serial.print(panPin);
      Serial.print(" y ");
      Serial.println(tiltPin);
      
      m_panServo = new Servo();
      m_panServo->attach(panPin);
      
      m_tiltServo = new Servo();
      m_tiltServo->attach(tiltPin);
      
      moveTo(m_pan, m_tilt);
    }
    
    char update()
    {
      if ((micros() - m_lastUpdate) > 16000)
      {
        m_lastUpdate = micros();
        m_pan = m_pan + m_panSpeed;
        m_tilt = m_tilt + m_tiltSpeed;
        
        if (m_pan > MAX_PAN)
            m_pan = MAX_PAN;
          else if (m_pan < MIN_PAN)
            m_pan = MIN_PAN;
          
          if (m_tilt > MAX_TILT)
            m_tilt = MAX_TILT;
          else if (m_tilt < MIN_TILT)
            m_tilt = MIN_TILT;
        
        m_panServo->write(m_pan);
        m_tiltServo->write(m_tilt);
      }
      
      return 0;
    }
    
    char moveTo(unsigned char X, unsigned char Y)
    {
      m_pan = X;
      m_tilt = Y;
      m_panSpeed = 0;
      m_tiltSpeed = 0;
      m_panServo->write(m_pan);
      m_tiltServo->write(m_tilt);
      
      return 0;
    }
    
    char accelerate(char dX, char dY)
    {
      float maxSpeed = 3;
        
      int realX = (dX * maxSpeed) / 128;
      int realY = (dY * maxSpeed) / 128;
      
      m_panSpeed = -realX;
      m_tiltSpeed = realY;
      
      return 0;
    }
    char* limits()
    {
      char bytes[5] = { MIN_PAN, MAX_PAN, MIN_TILT, MAX_TILT, 0x00 };
      return bytes;
    }
    
    char* currentPosition()
    {
      char bytes[3] = { m_pan, m_tilt, 0x00 };
      return bytes;
    }
};

#endif
