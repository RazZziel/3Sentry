#include <Arduino.h>
#include <Servo.h>
#include "PanTilt.h"

#define __DEBUG__ 1

#define LASER_PIN 6
#define LASER_PIN_2 7

#define BUF_SIZE 8

char buf[BUF_SIZE];
char bufLen = 0;

PanTilt* panTilts[2];
int nPanTilts = 0;

Servo servoPan, servoTilt;

void serialWrite(char bytes[]);
void ack();
void nack();
void processCommand();
void accelerate(unsigned char panTiltId, char dX, char dY);
void moveTo(unsigned char panTiltId, char X, char Y);
void limits(unsigned char panTiltId);
void getPosition(unsigned char panTiltId);
void fire(unsigned char gunId);
void stopFiring(unsigned char gunId);

void setup()
{
    Serial.begin(19200);
    Serial1.begin(19200);

    PanTilt* pt;
    pt = new PanTilt(2,3);
    pt->setMinPan(10);
    pt->setMaxPan(180);
    pt->setMinTilt(45);
    pt->setMaxTilt(110);
    panTilts[0] = pt;

    pt = new PanTilt(8,9);
    pt->setMinPan(0);
    pt->setMaxPan(0xb3);
    pt->setMinTilt(0);
    pt->setMaxTilt(255);
    panTilts[1] = pt;
    nPanTilts = 2;

    pinMode(LASER_PIN, OUTPUT);
    digitalWrite(LASER_PIN, LOW);

    pinMode(LASER_PIN_2, OUTPUT);
    digitalWrite(LASER_PIN_2, LOW);
}

void loop()
{
    int i;
    for (i=0; i<nPanTilts; ++i)
    {
        panTilts[i]->update();
    }
}

void serialEvent1()
{
    while (Serial1.available())
    {
        unsigned char myByte = Serial1.read();

        Serial.print("Received: ");
        Serial.println(myByte, HEX);


        /* If we receive the begining of a new command
         * while reading another one, reset the buffer and ignore
         * the previous command
         */
        if (myByte == 0x02)
        {
            bufLen = 0;
        }

        buf[bufLen] = myByte;
        bufLen++;
        if (myByte == 0x03)
        {
            processCommand();
            break;
            bufLen = 0;
        }

        if (bufLen >= BUF_SIZE)
        {
            bufLen = 0;
        }
    }
}

void processCommand()
{
    if (buf[0] == 0x02 && bufLen > 1)
    {
        switch (buf[1])
        {
        case 'L':
            Serial.println("Limits");
            if  (bufLen >= 3)
            {
                limits(buf[2]);
            }
            else
            {
                nack();
            }
            break;
        case 'A':
            Serial.println("Absolute");
            if (bufLen >= 6)
            {
                moveTo(buf[2], buf[3], buf[4]);
            }
            break;
        case 'M':
            Serial.println("Relative");
            if (bufLen >= 6)
            {
                accelerate(buf[2], buf[3], buf[4]);
            }
            else
            {
                Serial.println("Invalid message length");
                nack();
            }
            break;
        case 'P':
            Serial.println("Position");
            if (bufLen >= 3)
            {
                getPosition(buf[2]);
            }
            else
            {
                Serial.println("Invalid number of parameters");
                nack();
            }
            break;
        case 'S':
            Serial.println("Pew");
            if (bufLen >= 3)
            {
                fire(buf[2]);
            }
            break;
        case 'H':
            Serial.println("No pew");
            if (bufLen >= 3)
            {
                stopFiring(buf[2]);
            }
            break;
        default:
            Serial.print("Unknown command: ");
            Serial.println(buf[1]);
            break;
        }
    }
}

void accelerate(unsigned char panTiltId, char dX, char dY)
{
    if (panTiltId < nPanTilts)
    {
        /*
    Serial.print("Accelerate: ");
    Serial.print(panTiltId);
    Serial.print(", ");
    Serial.print(dX);
    Serial.print(", ");
    Serial.println(dY);*/
        if (!panTilts[panTiltId]->accelerate(dX, dY))
        {
            //ack();
            return;
        }
    }
    nack();
}

void moveTo(unsigned char panTiltId, char X, char Y)
{
    if (panTiltId < nPanTilts)
    {
        panTilts[panTiltId]->moveTo(X, Y);
    }
}

void limits(unsigned char panTiltId)
{
    if (panTiltId < nPanTilts)
    {
        serialWrite(panTilts[panTiltId]->limits());
    }
    else
    {
        nack();
    }
}
void getPosition(unsigned char panTiltId)
{
    if (panTiltId < nPanTilts)
    {
        serialWrite(panTilts[panTiltId]->currentPosition());
    }
    else
    {
        nack();
    }
}

void fire(unsigned char gunId)
{
    if (gunId == 0)
    {
        digitalWrite(LASER_PIN, HIGH);
    }
    else
    {
        digitalWrite(LASER_PIN_2, HIGH);
    }
}

void stopFiring(unsigned char gunId)
{
    if (gunId == 0)
    {
        digitalWrite(LASER_PIN, LOW);
    }
    else
    {
        digitalWrite(LASER_PIN_2, LOW);
    }
}

void ack()
{
    char bytes[] = {0x06, 0x00 };
    //serialWrite(bytes);
}

void nack()
{
    char bytes[] = { 0x15, 0x00 };
    //serialWrite(bytes);
}

void serialWrite(char bytes[])
{
    unsigned char idx = 0;
    unsigned char thisByte = bytes[idx++];
    Serial1.write(0x02);
    while (thisByte != 0)
    {
        Serial1.write(thisByte);
        thisByte = bytes[idx++];
    }
    Serial1.write(0x03);


}
