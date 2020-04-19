
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <iostream>
#include <fstream>
#include <iomanip>

#include <nmeaparse/nmea.h>

using namespace nmea;
using namespace std;


class CarPropReader {

  private:

    // Socket setting to read GPS data from socket server
    int socket_port;
    std::string socket_server;
    int sock = 0, valread;
    struct sockaddr_in serv_addr; 
    char buffer[1024] = {0}; 

    // GPS parser
    std::unique_ptr<NMEAParser> nmea_parser;
    std::unique_ptr<GPSService> gps;


  public:
    // Status of car
    // 0: Connected to server
    // 1: Not connected to server; 
    // -1: Socket creation error
    // -2: Invalid address/ Address not supported
    // -3: Connection failed
    int signal_status = 1;
    
    float car_speed = 0; // km/h
    float longtitude = 0;
    float latitude = 0;

  public:
    CarPropReader() {
        this->socket_port = 50000;
        this->socket_server = "192.168.1.244"; // TODO: Fix this

        // Create a GPS service that will keep track of the fix data.
        nmea_parser = std::make_unique<NMEAParser>();
        gps = std::make_unique<GPSService>(*nmea_parser.get());
        nmea_parser->log = false;

    }

    int printError() {
        switch (signal_status) {
            case 0: cout << "Socket: No error" << endl; break;
            case 1: cout << "Socket: Not connected" << endl; break;
            case -1: cout << "Socket: Socket creation error" << endl; break;
            case -2: cout << "Socket: Invalid address/ Address not supported" << endl; break;
            case -3: cout << "Socket: Connection failed" << endl; break;
        }
        return signal_status;
    }

    // Update car properties
    // Return 0: success
    // Return 1: fail
    int updateProps() {

        if (sock == 0) {
            init_socket_conn();
        }

        valread = read( sock , buffer, 1024);
        if (valread <= 0) {
            // cerr << "Recreate connection" << endl;
            init_socket_conn();
            valread = read(sock , buffer, 1024);
        }

        if (valread <= 0) {
            return 1;
        }

        signal_status = 0; // Connected

        buffer[valread] = '\n';

        // From a buffer in memory...
        nmea_parser->readBuffer((uint8_t*)buffer, sizeof(buffer));

        longtitude = gps->fix.longitude;
        latitude = gps->fix.latitude;
        car_speed = gps->fix.speed;

        return 0;

    }

  private:
    int init_socket_conn() {

        if (sock != 0) {
            close(sock);
            sock = 0;
        }

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
            signal_status = -1;
            // cerr << "\n Socket creation error \n" << endl; 
            return -1; 
        } 

        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    
        serv_addr.sin_family = AF_INET; 
        serv_addr.sin_port = htons(socket_port); 
        
        // Convert IPv4 and IPv6 addresses from text to binary form 
        if(inet_pton(AF_INET, socket_server.c_str(), &serv_addr.sin_addr) <= 0)  {
            signal_status = -2;
            // cerr << "\nInvalid address/ Address not supported \n" << endl; 
            return -1; 
        } 
    
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { 
            signal_status = -3;
            // cerr << "\nConnection Failed \n" << endl; 
            return -1; 
        } 

        return 0;
    }

};