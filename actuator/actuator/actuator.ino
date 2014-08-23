#include <Servo.h>

#define BUF_SIZE 8

#define MAX_TILT 115
#define MIN_TILT 45

#define MIN_PAN 10
#define MAX_PAN 180



char buf[BUF_SIZE];
char bufLen = 0;

int  pan1 = 90;
int tilt1 = 90;

Servo servoPan, servoTilt;

void ack();
void nack();
void processCommand();
void moveRelative(unsigned char panTiltId, unsigned char dX, unsigned char dY);
void limits(unsigned char panTiltId);
void getPosition(unsigned char panTiltId);

void setup()
{
	Serial.begin(19200);
	Serial1.begin(19200);
	
	servoPan.attach(2);
        servoTilt.attach(3);
}

void loop()
{
        servoPan.write(pan1);
        servoTilt.write(tilt1);
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
			break;
		case 'M':
                        Serial.println("Relative");
                        if (bufLen >= 6)
                        {
			  moveRelative(buf[2], buf[3], buf[4]);
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
                          nack();
                        }
			break;
		case 'S':
			Serial.println("Pew");
			break;
		case 'H':
			Serial.println("No pew");
			break;
		default:
			Serial.print("Unknown command: ");
			Serial.println(buf[1]);
			break;
		}
	}
}

void moveRelative(unsigned char panTiltId, unsigned char dX, unsigned char dY)
{
  int maxSpeed = 40;
  if (panTiltId == 0)
  {
    Serial.print("Input: ");
    Serial.print(dX);
    Serial.print(" ");
    Serial.println(dY);
    int realX = dX - 128;
    int realY = dY - 128;
    
    Serial.print("Centered: ");
    Serial.print(realX);
    Serial.print(" ");
    Serial.println(realY);
    
    realX = (realX * maxSpeed) / 128;   
    realY = (realY * maxSpeed) / 128;
    pan1 += realX;
    tilt1 += realY;
    
    if (pan1 > MAX_PAN)
      pan1 = MAX_PAN;
    else if (pan1 < MIN_PAN)
      pan1 = MIN_PAN;
    
    if (tilt1 > MAX_TILT)
      tilt1 = MAX_TILT;
    else if (tilt1 < MIN_TILT)
      tilt1 = MIN_TILT;
      
    Serial.print("Moveiendo pantilt: ");
    Serial.print(pan1);
    Serial.print(", ");
    Serial.println(tilt1);
    ack();
  }
  else
  {
    nack();
  }
}

void limits(unsigned char panTiltId)
{
  if (panTiltId == 0)
  {
    Serial1.write(0x02);
    Serial1.write(MIN_PAN);
    Serial1.write(MAX_PAN);
    Serial1.write(MIN_TILT);
    Serial1.write(MAX_TILT);
    Serial1.write(0x03);
  }
  else
  {
    nack();
  }
}
void getPosition(unsigned char panTiltId)
{
  if (panTiltId == 0)
  {
    Serial1.write(0x02);
    Serial1.write(MIN_PAN);
    Serial1.write(MAX_PAN);
    Serial1.write(MIN_TILT);
    Serial1.write(MAX_TILT);
    Serial1.write(0x03);
  }
  else
  {
    nack();
  }
}

void ack()
{
  Serial1.write(0x02);
  Serial1.write(0x06);
  Serial1.write(0x03);
}

void nack()
{
  Serial1.write(0x02);
  Serial1.write(0x15);
  Serial1.write(0x03);
}
