#include "client.h"
#include <iostream>

#include <cflie/CCrazyflie.h>

int main() {

    CCrazyRadio *radio = new CCrazyRadio("radio://0/10/250K");

    if (radio->startRadio()) {
        CCrazyflie *flie = new CCrazyflie(radio);
        flie->setSendSetpoints(true);

        Client *client = new Client(flie);
        client->run();
        delete flie;
        delete client;
    } else {
        std::cerr << "Failed to connect to dongle; check connection" << std::endl;
        Client *client = new Client(nullptr);
        client->run();
        delete client;
    }

    delete radio;
    return 0;
}
