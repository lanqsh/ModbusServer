#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <exception>
#include "ModbusServer.h"

using std::string;

#define PrintBuf(buf, bufLength) do { \
        char pBuf[1024] = {0}; \
        if (1 > bufLength || NULL == buf || bufLength >= 256) { \
            snprintf(pBuf, 1024, "buf is %p, length is %d\n", buf, bufLength); \
        } else { \
            for (unsigned int i = 0; i < bufLength; ++i) { \
                snprintf(pBuf+i*3, 4, "%02x ", buf[i]); \
            } \
        } \
        printf("[%s %s %d]%s\n",__FILE__,__func__,__LINE__, pBuf); \
    } while (0)

ModbusServer::ModbusServer(const std::string &ip
                                , const int port
                                , unsigned int start_bits
                                , unsigned int nb_bits
                                , unsigned int start_input_bits
                                , unsigned int nb_input_bits
                                , unsigned int start_registers
                                , unsigned int nb_registers
                                , unsigned int start_input_registers
                                , unsigned int nb_input_registers)
                                : m_ip(ip)
                                , m_port(port)
                                , m_start_bits(start_bits)
                                , m_nb_bits(nb_bits)
                                , m_start_input_bits(start_input_bits)
                                , m_nb_input_bits(nb_input_bits)
                                , m_start_registers(start_registers)
                                , m_nb_registers(nb_registers)
                                , m_start_input_registers(start_input_registers)
                                , m_nb_input_registers(nb_input_registers)
{
    Init();
}

ModbusServer::~ModbusServer()
{
    if (!m_stop) Stop();
}

void ModbusServer::Start()
{
    int master_socket;
    int rc;
    fd_set refset;
    fd_set rdset;

    /* Maximum file descriptor number */
    int fdmax;

    /* Clear the reference set of socket */
    FD_ZERO(&refset);
    /* Add the server socket */
    FD_SET(m_fd, &refset);

    /* Keep track of the max file descriptor */
    fdmax = m_fd;

    while (!m_stop)
    {
        rdset = refset;
        if (select(fdmax+1, &rdset, NULL, NULL, NULL) == -1)
        {
            perror("Server select() failure.");
            continue;
        }

        /* Run through the existing connections looking for data to be
         * read */
        for (master_socket = 0; master_socket <= fdmax; master_socket++)
        {

            if (FD_ISSET(master_socket, &rdset))
            {
                if (master_socket == m_fd)
                {
                    /* A client is asking a new connection */
                    socklen_t addrlen;
                    struct sockaddr_in clientaddr;
                    int newfd;

                    /* Handle new connections */
                    addrlen = sizeof(clientaddr);
                    memset(&clientaddr, 0, sizeof(clientaddr));
                    newfd = accept(m_fd, (struct sockaddr *)&clientaddr, &addrlen);
                    if (newfd == -1)
                    {
                        perror("Server accept() error");
                    }
                    else
                    {
                        FD_SET(newfd, &refset);
                        if (newfd > fdmax)
                        {
                            /* Keep track of the maximum */
                            fdmax = newfd;
                        }
                        printf("New connection from %s:%d on socket %d\n", inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port, newfd);
                    }
                }
                else
                {
                    /* An already connected master has sent a new query */
                    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
                    int data = 0;
                    int address=0;

                    modbus_set_socket(m_ctx, master_socket);
                    rc = modbus_receive(m_ctx, query);
                    printf("modbus_receive:%d\n", rc);

                    if (rc != -1)
                    {
                        modbus_reply(m_ctx, query, rc, m_mapping);
                        PrintBuf(query, rc);
                    }
                    else
                    {
                        /* Connection closed by the client, end of server */
                        printf("Connection closed on socket %d\n", master_socket);
                        close(master_socket);

                        /* Remove from reference set */
                        FD_CLR(master_socket, &refset);

                        if (master_socket == fdmax)
                        {
                            fdmax--;
                        }
                    }
                }
            }
        }
    }
    m_stopped = true;
}

void ModbusServer::Stop()
{
    m_stop = true;
    while (!m_stopped) usleep(100);
    modbus_mapping_free(m_mapping);
    modbus_close(m_ctx);
    modbus_free(m_ctx);
}

void ModbusServer::SetBit(const int addr, unsigned char val)
{
    m_mapping->tab_bits[addr] = val;
}

void ModbusServer::SetInputBit(const int addr, unsigned char val)
{
    m_mapping->tab_input_bits[addr] = val;
}

void ModbusServer::SetRegister(const int addr, unsigned short val)
{
    m_mapping->tab_registers[addr] = val;
}

void ModbusServer::SetInputRegister(const int addr, unsigned short val)
{
    m_mapping->tab_input_registers[addr] = val;
}

unsigned char ModbusServer::GetBit(const int addr)
{
    return m_mapping->tab_bits[addr];
}

unsigned char ModbusServer::GetInputBit(const int addr)
{
    return m_mapping->tab_input_bits[addr];
}

unsigned short ModbusServer::GetRegister(const int addr)
{
    return m_mapping->tab_registers[addr];
}

unsigned short ModbusServer::GetInputRegister(const int addr)
{
    return m_mapping->tab_input_registers[addr];
}

void ModbusServer::Init()
{
    m_stop = false;
    m_stopped = false;
    m_ctx = modbus_new_tcp(m_ip.c_str(), m_port);
    m_mapping = modbus_mapping_new_start_address(m_start_bits
                                                    , m_nb_bits
                                                    , m_start_input_bits
                                                    , m_nb_input_bits
                                                    , m_start_registers
                                                    , m_nb_registers
                                                    , m_start_input_registers
                                                    , m_nb_input_registers);
    if (m_mapping == NULL)
    {
        Stop();
        printf("ModbusMapping error\n");
        throw;
    }
    m_fd = modbus_tcp_listen(m_ctx, 1);

}

