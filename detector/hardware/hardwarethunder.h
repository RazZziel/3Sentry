#ifndef HARDWARETHUNDER_H
#define HARDWARETHUNDER_H

#include "hardware.h"

struct usb_dev_handle;

class HardwareThunder : public Hardware
{
    Q_OBJECT
public:
    HardwareThunder(QObject *parent=0);
    ~HardwareThunder();

    bool getLimits(Pantilt pantilt, int &minX, int &maxX, int &minY, int &maxY);
    bool currentPosition(Pantilt pantilt, uint &x, uint &y) const;
    bool targetAbsolute(Pantilt pantilt, uint x, uint y, bool convertPos=true);
    bool targetRelative(Pantilt pantilt, qreal dx, qreal dy);
    bool enableFiring(Gun gun);
    bool stopFiring(Gun gun);

private:
    int send_message(char* msg, int index);
    void movement_handler(char control);

    usb_dev_handle* m_usbHandler;
};

#endif // HARDWARETHUNDER_H
