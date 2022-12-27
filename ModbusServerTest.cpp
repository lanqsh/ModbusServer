#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include "ModbusServer.h"


int main()
{
    ModbusServer server("0.0.0.0", 1502);
    std::vector<unsigned short> v;
    int size = 100;
    for (int i = 0; i < size; ++i)
    {
        v.emplace_back(i);
    }
    server.SetRegister(reinterpret_cast<unsigned char*>(v.data()), v.size() *2);
    server.SetInputRegister(reinterpret_cast<unsigned char*>(v.data()), v.size() *2);

    std::thread t(&ModbusServer::Start, &server);

    //test set & get
    {
        std::cout << "test set & get" << std::endl;
        server.SetRegister(10, 2555);
        unsigned short val = server.GetRegister(11);
        std::cout << "GetRegister:" << val << std::endl;
    }

    int runTime = 30;
    for (int i = 0; i < runTime; ++i)
    {
        std::cout << "running time " << i << "s" << std::endl;
        sleep(1);
    }

    //server.Stop();
    t.join();
}
