#ifndef CLIENT_H
#define CLIENT_H

#include <sys/un.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <string>

#include "rapidjson/document.h"
#include "cflie/CCrazyflie.h"

using namespace std;

class Command {
public:
    int Turn() {
        return this->turnDir;
    }
    int Tilt() {
        return this->tiltDir;    
    }
    bool Squeeze() {
        return this->squeeze;
    }
    bool Stop() {
        return this->shouldStop;
    } 

    const int CENTER = 0;
    const int RIGHT = 1;
    const int LEFT = 2;
    const int UP = 1;
    const int DOWN = 2;
    const int NO_CHANGE = 3;

    Command(rapidjson::Document input) {
        // parse json if valid
        if (input.IsObject()) {
            if (input.HasMember("stop")) {
                this->shouldStop = input["stop"].GetBool();
            }
            if (input.HasMember("squeeze")) {
                this->squeeze = input["squeeze"].GetBool();
            }
            if (input.HasMember("acc")) {
                // tilt
                int pitch = input["acc"][0];
                if (abs(pitch) < 200) {
                    this->tiltDir = this->CENTER;
                else if ( pitch > 0 ) {
                    this->tiltDir = this->UP;
                } else {
                    this->tiltDir = this->DOWN;
                }
                // turn
                int roll = input["acc"][2];
                if (abs(roll) < 400) {
                    this->turnDir = CENTER;
                else if ( roll > 0 ) {
                    this->turnDir = RIGHT;
                } else {
                    this->turnDir = LEFT;
                }
            }
        }
    }

    ~Command() {
        // do nothing
    }

private:
    int turnDir = this->NO_CHANGE;
    int tiltDir = this->NO_CHANGE;
    bool shouldStop = false;
    bool squeeze = false;
}

void printOrientation(Accel accel) {
    if ( abs(accel.x) < 200 ) {
        cout << "center";
    } else if (accel.x > 0) {
        cout << "up";
    } else {
        cout << "down";
    }

    cout << "\t";

    if ( abs(accel.z) < 400 ) {
        cout << "center";
    } else if (accel.z > 0) {
        cout << "right";
    } else {
        cout << "left";
    }

    cout << endl;
}

class Client {
private:
    const char* socketName = "./uds_socket";

public:
    Client(CCrazyfile *flie) {
        this->buflen = 1024;
        this->buf = new char[buflen+1];
        this->flie = flie;
    }

    ~Client() {
        delete this->flie;
    }

    void run() {
        cout << "about to run create()" << endl;
        create();
        cout << "about to run listenForData()" << endl;
        listenForData();
    }

protected:
    void create() {
        struct sockaddr_un serverAddress;
        memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sun_family = AF_UNIX;
        strncpy(serverAddress.sun_path, socketName, sizeof(serverAddress.sun_path) - 1);

        server = socket(PF_UNIX, SOCK_STREAM, 0);
        if (!server) {
            perror("socket");
            exit(-1);
        }

        if (connect(server, (const struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
            perror("connect");
            exit(-1);
        }
    }

    void listenForData() {
        while ( this->flie->cycle() ) {
            bool stop = getCommand();
            if (stop) {
                break;
            } 
            closeSocket();
        }

    }

    void closeSocket() {

    }

    bool sendRequest(string request) {
        const char *ptr = request.c_str();
        int amountLeft = request.length();
        int amountWritten = 0;
        while (amountLeft) {
            if ((amountWritten = send(server, ptr, amountLeft, 0)) < 0 ) {
                if (errno == EINTR) {
                    continue;
                } else {
                    perror("write");
                    return false;
                }
            } else if (amountWritten == 0) {
                return false;
            }
            amountLeft -= amountWritten;
            ptr += amountWritten;
        }
        return true;    
    }

    bool getCommand() {
        string input = "";
        while( input.find("\n") == string::npos) {
            int amountRead = recv(server, buf, 1024, 0);
            if (amountRead < 0) {
                if (errno == EINTR) {
                    continue;
                } else {
                    return "";
                }
            } else if (amountRead == 0) {
                return "";
            }
            input.append(buf, amountRead);
        }
        cout << "received input: " << input;

        rapidjson::Document json;
        json.Parse(input.c_str());
        Command *cmd = new Command(json);

        return handleCommand(cmd);
    }

    bool handleCommand(Command *cmd) {

        if (cmd->ShouldStop()) {
            this->flie->setThrust(0);
            return true;
        }

        if (cmd->Squeeze()) {
            this->flie->setThrust(0);
            // hover...
        } else {
            this->flie->setThrust(15000);
        }

        switch (cmd->Turn()) {
            case RIGHT:
                this->flie->setRoll(20);
                break;
            case LEFT:
                this->flie->setRoll(-20);
                break;
            case CENTER:
                this->flie->setRoll(0);
                break;
            case NO_CHANGE:
            default:
        }

        switch (cmd->Tilt()) {
            case UP:
                this->flie->setPitch(15);
                break;
            case DOWN:
                this->flie->setPitch(-15);
                break;
            case CENTER:
                this->flie->setPitch(0);
                break;
            case NO_CHANGE:
            default:
        }

        return false;

    }


    int server;
    int buflen;
    char *buf;
    CCrazyflie *flie;

};

#endif
