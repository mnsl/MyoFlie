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
#include <chrono>

#include "rapidjson/document.h"
#include "cflie/CCrazyflie.h"

using namespace std;
using namespace rapidjson;
using namespace std::chrono;

class Command {
private:
    int turnDir = 3;
    int tiltDir = 3;
    bool shouldStop = false;
    bool squeeze = false;
public:
    Command(Document* input) {
        // parse json if valid
        if (input->IsObject()) {
            if (input->HasMember("stop")) {
                this->shouldStop = (*input)["stop"].GetBool();
            }
            if (input->HasMember("squeeze")) {
                this->squeeze = (*input)["squeeze"].GetBool();
            }
            if (input->HasMember("acc")) {
                // tilt
                int pitch = (*input)["acc"][0].GetInt();
                if (abs(pitch) < 200) {
                    this->tiltDir = this->CENTER;
                } else if ( pitch > 0 ) {
                    this->tiltDir = this->UP;
                } else {
                    this->tiltDir = this->DOWN;
                }
                // turn
                int roll = (*input)["acc"][2].GetInt();
                if (abs(roll) < 400) {
                    this->turnDir = this->CENTER;
                } else if ( roll > 0 ) {
                    this->turnDir = this->RIGHT;
                } else {
                    this->turnDir = this->LEFT;
                }
            }
        }
    }

    ~Command() {
        // do nothing
    }
    const static int CENTER = 0;
    const static int RIGHT = 1;
    const static int LEFT = 2;
    const static int UP = 1;
    const static int DOWN = 2;
    const static int NO_CHANGE = 3;

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

};

class Client {
private:
    const char* socketName = "./uds_socket";

public:
    Client(CCrazyflie *flie) {
        this->buflen = 1024;
        this->buf = new char[buflen+1];
        this->flie = flie;
    }

    ~Client() {
        delete this->flie;
    }

    void run() {
        create();
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
        if (this->flie) {
            while ( this->flie->cycle() ) {
                bool stop = getCommand();
                if (stop) {
                    break;
                } 
                closeSocket();
            }
        } else {
            while (true) {
                bool stop = getCommand();
                if (stop) {
                    break;
                } 
                closeSocket();
            }
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

        rapidjson::Document *json = new rapidjson::Document();
        json->Parse(input.c_str());
        Command *cmd = new Command(json);

        return handleCommand(cmd);
    }

    bool handleCommand(Command *cmd) {
        if ( this->flie ) {
            system_clock::time_point currentTime = system_clock::now();
            double currentAccZ = this->flie->accZ();
            double diffAccZ = currentAccZ - this->lastAccZ;
            double currentThrust = this->flie->thrust();

            if (cmd->Stop()) {
                this->flie->setThrust(0);
                this->lastTime = currentTime;
                this->lastAccZ = currentAccZ;
                return true;
            }

            if (cmd->Squeeze()) {
                // hover...
                double loopTime = duration_cast<milliseconds>(currentTime - this->lastTime).count();
                double velZ = diffAccZ * loopTime;
                this->flie->setThrust(currentThrust + 1000*velZ);
            } else {
                this->flie->setThrust(45000);
            }

            switch (cmd->Turn()) {
                case Command::RIGHT:
                    this->flie->setRoll(20);
                    break;
                case Command::LEFT:
                    this->flie->setRoll(-20);
                    break;
                case Command::CENTER:
                    this->flie->setRoll(0);
                    break;
            }

            switch (cmd->Tilt()) {
                case Command::UP:
                    this->flie->setPitch(15);
                    break;
                case Command::DOWN:
                    this->flie->setPitch(-15);
                    break;
                case Command::CENTER:
                    this->flie->setPitch(0);
                    break;
            }

            this->lastTime = currentTime;
            this->lastAccZ = currentAccZ;
            return false;
        } else {
            return cmd->Stop();
        }

    }


    int server;
    int buflen;
    char *buf;
    CCrazyflie *flie;

    system_clock::time_point lastTime;
    double lastAccZ;

};

#endif
