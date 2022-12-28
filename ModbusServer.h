#include <modbus.h>
#include <string>

enum class Backend_T {
    TCP,
    TCP_PI,
    RTU
};

class ModbusServer
{
public:
    ModbusServer(const std::string &ip = "0.0.0.0"
                    , const int port = 1502
                    , unsigned int start_bits = 0
                    , unsigned int nb_bits = 100
                    , unsigned int start_input_bits = 0
                    , unsigned int nb_input_bits = 100
                    , unsigned int start_registers = 0
                    , unsigned int nb_registers = 100
                    , unsigned int start_input_registers = 0
                    , unsigned int nb_input_registers = 100);

    ModbusServer(const std::string &device
                    , const int baud
                    , const char parity
                    , const int data_bit
                    , const int stop_bit
                    , unsigned int start_bits = 0
                    , unsigned int nb_bits = 100
                    , unsigned int start_input_bits = 0
                    , unsigned int nb_input_bits = 100
                    , unsigned int start_registers = 0
                    , unsigned int nb_registers = 100
                    , unsigned int start_input_registers = 0
                    , unsigned int nb_input_registers = 100);

    ~ModbusServer();

    void Start();
    void Stop();

    void SetBit(const int addr, unsigned char val);
    void SetBit(unsigned char *buf, unsigned char len);
    void SetInputBit(const int addr, unsigned char val);
    void SetInputBit(unsigned char *buf, unsigned char len);
    void SetRegister(const int addr, unsigned short val);
    void SetRegister(unsigned char *buf, unsigned char len);
    void SetInputRegister(const int addr, unsigned short val);
    void SetInputRegister(unsigned char *buf, unsigned char len);

    unsigned char GetBit(const int addr);
    unsigned char GetInputBit(const int addr);
    unsigned short GetRegister(const int addr);
    unsigned short GetInputRegister(const int addr);

    void SetSlave(const int slave);
private:
    void Init();
    void RunTCP();
    void RunRTU();
private:
    bool                    m_stop;
    bool                    m_stopped;

    unsigned char           m_start_bits;
    unsigned char           m_nb_bits;
    unsigned char           m_start_input_bits;
    unsigned char           m_nb_input_bits;
    unsigned char           m_start_registers;
    unsigned char           m_nb_registers;
    unsigned char           m_start_input_registers;
    unsigned char           m_nb_input_registers;

    int                     m_port;
    int                     m_fd;

    modbus_t               *m_ctx;
    modbus_mapping_t       *m_mapping;

    std::string             m_ip;
    std::string             m_device;
    Backend_T               m_mode;

    int                     m_baud;
    char                    m_parity;
    int                     m_data_bit;
    int                     m_stop_bit;
    int                     m_sid;
};
