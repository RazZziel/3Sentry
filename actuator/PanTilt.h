#ifndef _PANTILT_H_
#define _PANTILT_H_

#include <Arduino.h>
#include <Servo.h>

class PanTilt
{
private:
    float m_pan;
    float m_panSpeed;
    float m_tilt;
    float m_tiltSpeed;
    
    int m_maxPan;
    int m_minPan;
    int m_maxTilt;
    int m_minTilt;
    
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

            if (m_pan > m_maxPan)
                m_pan = m_maxPan;
            else if (m_pan < m_minPan)
                m_pan = m_minPan;

            if (m_tilt > m_maxTilt)
                m_tilt = m_maxTilt;
            else if (m_tilt < m_minTilt)
                m_tilt = m_minTilt;

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
        
        float realX = (dX * maxSpeed) / (float)128;
        float realY = (dY * maxSpeed) / (float)128;

        m_panSpeed = -realX;
        m_tiltSpeed = realY;
        /*
      Serial.print("New Speed: ");
      Serial.print(m_panSpeed);
      Serial.print(", ");
      Serial.println(m_tiltSpeed);
      */
        return 0;
    }
    unsigned char limits( char *buf )
    {
        buf[0] = m_minPan;
        buf[1] = m_maxPan;
        buf[2] = m_minTilt;
        buf[3] = m_maxTilt;
        
        return 4;
    }
    
    unsigned char currentPosition(char *buf)
    {
        buf[0] = m_pan;
        buf[1] = m_tilt;

        return 2;
    }
    
    void setMinPan(int minPan)
    {
        m_minPan = minPan;
    }
    
    void setMaxPan(int maxPan)
    {
        m_maxPan = maxPan;
    }
    
    void setMinTilt(int minTilt)
    {
        m_minTilt = minTilt;
    }
    
    void setMaxTilt(int maxTilt)
    {
        m_maxTilt = maxTilt;
    }
};

#endif
