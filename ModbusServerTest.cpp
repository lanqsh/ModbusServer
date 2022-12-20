#include <unistd.h>
#include <iostream>
#include <string>
#include <thread>
#include "ModbusServer.h"


int main()
{
    ModbusServer server("0.0.0.0", 1502);

    std::thread t(&ModbusServer::Start, &server);
    int runTime = 30;
    for (int i = 0; i < runTime; ++i)
    {
        std::cout << "running time " << i << "s" << std::endl;
        sleep(1);
    }

    //server.Stop();
    t.join();
}
