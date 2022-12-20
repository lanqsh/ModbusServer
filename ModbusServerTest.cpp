#include <unistd.h>
#include <iostream>
#include <string>
#include <thread>
#include "ModbusServer.h"


int main()
{
    ModbusServer server("0.0.0.0", 1502);

    std::thread t(&ModbusServer::Start, &server);


    //test set & get
    {
        //usleep(10 * 1000);
        std::cout << "test set & get" << std::endl;
        server.SetRegister(11, 2555);
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
