#include "client.h"
#include <iostream>

#include <cflie/CCrazyflie.h>

int main() {

    CCrazyRadio *radio = new CCrazyRadio("radio://0/10/250K");

    if (radio->startRadio()) {
        CCrazyflie *flie = new CCrazyflie(radio);
        flie->setSendPoints(true);

        Client client = Client(flie);
        client.run();
        delete flie;
    } else {
        std::cerr << "Failed to connect to dongle; check connection" << std::endl;
    }

    delete radio;

}
