#include <Servo.h>
#include "PanTilt.h"

#define LASER_PIN 6

#define BUF_SIZE 8

char buf[BUF_SIZE];
char bufLen = 0;

PanTilt* panTilts[1];
int nPanTilts = 1;

Servo servoPan, servoTilt;

void serialWrite(char bytes[]);
void ack();
void nack();
void processCommand();
void accelerate(unsigned char panTiltId, unsigned char dX, unsigned char dY);
void moveTo(unsigned char panTiltId, char X, char Y);
void limits(unsigned char panTiltId);
void getPosition(unsigned char panTiltId);
void fire(unsigned char gunId);
void stopFiring(unsigned char gunId);

void setup()
{
  Serial.begin(19200);
  Serial1.begin(19200);

  PanTilt *pt = new PanTilt(2,3);
  panTilts[0] = pt;
  
  pinMode(LASER_PIN, OUTPUT);
  digitalWrite(LASER_PIN, LOW);
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
                          Serial.println("Pew Pew");
                          fire(buf[1]);
                        }
			break;
		case 'H':
			Serial.println("No pew");
                        if (bufLen >= 3)
                        {
                          stopFiring(buf[1]);
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
    digitalWrite(LASER_PIN, HIGH);
}

void stopFiring(unsigned char gunId)
{
    digitalWrite(LASER_PIN, LOW);
}

void ack()
{
  char bytes[] = {0x06, 0x00};
  //serialWrite(bytes);
}

void nack()
{
  char bytes[] = { 0x15, 0x00};
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
