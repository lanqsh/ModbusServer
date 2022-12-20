#include <modbus.h>
#include <string>


class ModbusServer
{
public:
    ModbusServer(const std::string &ip = "0.0.0.0"
    , const int port = 1502
    , unsigned int start_bits = 0
    , unsigned int nb_bits = 200
    , unsigned int start_input_bits = 0
    , unsigned int nb_input_bits = 200
    , unsigned int start_registers = 0
    , unsigned int nb_registers = 200
    , unsigned int start_input_registers = 0
    , unsigned int nb_input_registers = 200);
    ~ModbusServer();

    void Start();
    void Stop();

    void SetBit(const int addr, unsigned char val);
    void SetInputBit(const int addr, unsigned char val);
    void SetRegister(const int addr, unsigned short val);
    void SetInputRegister(const int addr, unsigned short val);

    unsigned char GetBit(const int addr);
    unsigned char GetInputBit(const int addr);
    unsigned short GetRegister(const int addr);
    unsigned short GetInputRegister(const int addr);
private:
    void Init();
private:
    bool                    m_stop;
    bool                    m_stopped;

    std::string             m_ip;
    int                     m_port;
    int                     m_fd;
    modbus_t               *m_ctx;
    modbus_mapping_t       *m_mapping;

    unsigned int            m_start_bits;
    unsigned int            m_nb_bits;
    unsigned int            m_start_input_bits;
    unsigned int            m_nb_input_bits;
    unsigned int            m_start_registers;
    unsigned int            m_nb_registers;
    unsigned int            m_start_input_registers;
    unsigned int            m_nb_input_registers;

};
