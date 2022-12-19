#include <modbus.h>
#include <string>


class ModbusServer
{
public:
    ModbusServer(const std::string &ip = "0.0.0.0", const int port = 1502);
    ~ModbusServer();

    void ModbusMapping(int nb_bits,
                             int nb_input_bits,
                             int nb_registers,
                             int nb_input_registers);

     void Start();
     void Stop();

private:
    void Init();
private:
    std::string             m_ip;
    int                     m_port;
    int                     m_fd;
    modbus_t               *m_ctx;
    modbus_mapping_t       *m_mapping;

};
