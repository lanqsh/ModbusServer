#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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

ModbusServer::ModbusServer(const string &ip, const int port)
        : m_ip(ip), m_port(port)
{
    Init();
}

ModbusServer::~ModbusServer()
{
    Stop();
}

void ModbusServer::ModbusMapping(int nb_bits,
                         int nb_input_bits,
                         int nb_registers,
                         int nb_input_registers)
{

}

void ModbusServer::Init()
{
    m_ctx = modbus_new_tcp(m_ip.c_str(), m_port);
    /* modbus_set_debug(m_ctx, TRUE); */

    m_mapping = modbus_mapping_new(200, 200, 200, 200);
/*
    unsigned char tmp[200] = {0};

    modbus_set_bits_from_bytes(m_mapping->tab_bits, 0, 100, tmp);
    modbus_set_bits_from_bytes(m_mapping->tab_input_bits, 0, 100, tmp);
    modbus_set_bits_from_bytes((unsigned char*)m_mapping->tab_registers, 0, 100*8*2, tmp);
    modbus_set_bits_from_bytes((unsigned char*)m_mapping->tab_input_registers, 0, 100*8*2, tmp);
*/
    if (m_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n", modbus_strerror(errno));
        modbus_free(m_ctx);
        return;
    }
    m_fd = modbus_tcp_listen(m_ctx, 1);

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

    for (;;)
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

}

void ModbusServer::Stop()
{
    modbus_mapping_free(m_mapping);
    modbus_close(m_ctx);
    modbus_free(m_ctx);
}

