#include <unistd.h>
#include <iostream>
#include <string>
#include "ModbusServer.h"


int main()
{
    ModbusServer server("0.0.0.0", 1502);

    server.Start();

    for (;;)
    {
        std::cout << "running..." << std::endl;
        sleep(1);
    }
    //server.Stop();
}
