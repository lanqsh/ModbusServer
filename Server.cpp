#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <memory>
#include "ModbusServer.h"

enum {
    TCP,
    TCP_PI,
    RTU
};

int main(int argc, char *argv[])
{
    int use_backend;
    std:: string ip_or_device;

    if (argc > 1) {
        if (strcmp(argv[1], "tcp") == 0) {
            use_backend = TCP;
        } else if (strcmp(argv[1], "tcppi") == 0) {
            use_backend = TCP_PI;
        } else if (strcmp(argv[1], "rtu") == 0) {
            use_backend = RTU;
        } else {
            printf("Modbus server for unit testing.\n");
            printf("Usage:\n  %s [tcp|tcppi|rtu] [<ip or device>]\n", argv[0]);
            printf("Eg. tcp 127.0.0.1 or rtu /dev/ttyUSB0\n\n");
            return -1;
        }
    } else {
        /* By default */
        use_backend = TCP;
        ip_or_device = "0.0.0.0";
    }

    if (argc > 2) {
        ip_or_device = argv[2];
    } else {
        switch (use_backend) {
        case TCP:
            ip_or_device = "0.0.0.0";
            break;
        case TCP_PI:
            ip_or_device = "::1";
            break;
        case RTU:
            ip_or_device = "/dev/ttyS1";
            break;
        default:
            break;
        }
    }

    std::shared_ptr<ModbusServer> sp_server = nullptr;
    if (use_backend == TCP) {
        sp_server = std::make_shared<ModbusServer>(ip_or_device, 1502);
    } else if (use_backend == TCP_PI) {

    } else {
        sp_server = std::make_shared<ModbusServer>(ip_or_device, 9600, 'N', 8, 1);
        int id = 1;
        sp_server->SetSlave(id);
    }

    std::vector<unsigned short> v;
    int size = 100;
    for (int i = 0; i < size; ++i)
    {
        v.emplace_back(i);
    }
    sp_server->SetRegister(reinterpret_cast<unsigned char*>(v.data()), v.size() *2);
    sp_server->SetInputRegister(reinterpret_cast<unsigned char*>(v.data()), v.size() *2);

    std::thread t(&ModbusServer::Start, sp_server.get());
    //sp_server->Start();

    //test set & get
    {
        std::cout << "test set & get" << std::endl;
        sp_server->SetRegister(10, 2555);
        unsigned short val = sp_server->GetRegister(10);
        std::cout << "GetRegister:" << val << std::endl;
    }

    int runTime = 30;
    for (int i = 0; i < runTime; ++i)
    {
        std::cout << "running time " << i << "s" << std::endl;
        sleep(1);
    }

    sp_server->Stop();
    t.join();
}
