/*
 * Copyright © Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <modbus.h>

/* The goal of this program is to check all major functions of
   libmodbus:
   - write_coil
   - read_bits
   - write_coils
   - write_register
   - read_registers
   - write_registers
   - read_registers

   All these functions are called with random values on a address
   range defined by the following defines.
*/
#define LOOP          100
#define SERVER_ID     17
#define ADDRESS_START 0
#define ADDRESS_END   99

#define PrintBuf(buf, bufLength) do { \
        char pBuf[1024] = {0}; \
        char *p = (char*)buf; \
        if (1 > bufLength || NULL == p || bufLength >= 256) { \
            snprintf(pBuf, 1024, "buf is %p, length is %d\n", p, bufLength); \
        } else { \
            for (unsigned int i = 0; i < bufLength; ++i) { \
                snprintf(pBuf+i*3, 4, "%.2X ", p[i]); \
            } \
        } \
        printf("[%s %d]%s\n",__func__,__LINE__, pBuf); \
    } while (0)

/* At each loop, the program works in the range ADDRESS_START to
 * ADDRESS_END then ADDRESS_START + 1 to ADDRESS_END and so on.
 */
int main(void)
{
    modbus_t *ctx;
    int rc;
    int nb_fail;
    int nb_loop;
    int addr = ADDRESS_START;
    short val = ADDRESS_START;
    int nb;
    uint8_t *tab_rq_bits;
    uint8_t *tab_rp_bits;
    uint16_t *tab_rq_registers;
    uint16_t *tab_rw_rq_registers;
    uint16_t *tab_rp_registers;

    /* RTU */
    /*
        ctx = modbus_new_rtu("/dev/ttyUSB0", 19200, 'N', 8, 1);
        modbus_set_slave(ctx, SERVER_ID);
    */

    /* TCP */
    ctx = modbus_new_tcp("0.0.0.0", 1502);
    //modbus_set_debug(ctx, TRUE);

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    /* Allocate and initialize the different memory spaces */
    nb = ADDRESS_END - ADDRESS_START + 1;

    tab_rq_bits = (uint8_t *) malloc(nb * sizeof(uint8_t));
    memset(tab_rq_bits, 0, nb * sizeof(uint8_t));

    tab_rp_bits = (uint8_t *) malloc(nb * sizeof(uint8_t));
    memset(tab_rp_bits, 0, nb * sizeof(uint8_t));

    tab_rq_registers = (uint16_t *) malloc(nb * sizeof(uint16_t));
    memset(tab_rq_registers, 0, nb * sizeof(uint16_t));

    tab_rp_registers = (uint16_t *) malloc(nb * sizeof(uint16_t));
    memset(tab_rp_registers, 0, nb * sizeof(uint16_t));

    tab_rw_rq_registers = (uint16_t *) malloc(nb * sizeof(uint16_t));
    memset(tab_rw_rq_registers, 0, nb * sizeof(uint16_t));

    /* WRITE BIT */
    tab_rq_bits[0] = 0xFF;
    tab_rq_bits[1] = 0xFF;
    rc = modbus_write_bits(ctx, addr, nb, tab_rq_bits);
    rc = modbus_read_bits(ctx, addr, nb, tab_rp_bits);
    printf("modbus_read_bits:%d\n", rc);
    PrintBuf(tab_rp_bits, nb);

    int read_cnt = 100;
    rc = modbus_read_registers(ctx, 0, read_cnt, tab_rp_registers);
    if (rc < 0) {
        printf("ERROR modbus_read_registers single (%d)\n", rc);
    }
    PrintBuf(tab_rp_registers, nb*2);

    for (int i = 0; i < read_cnt; ++i)
    {
        unsigned short result = tab_rp_registers[i];
        printf("modbus_read_registers:%d result:%d\n", rc, result);
    }

    memset(tab_rp_registers, 0, nb * sizeof(uint16_t));
    modbus_read_input_registers(ctx, 0, read_cnt, tab_rp_registers);
    for (int i = 0; i < read_cnt; ++i)
    {
        unsigned short result = tab_rp_registers[i];
        printf("modbus_read_input_registers:%d result:%d\n", rc, result);
    }

    nb_loop = nb_fail = 0;
    while (nb_loop++ < LOOP) {
        /* SINGLE REGISTER */
        rc = modbus_write_register(ctx, addr, val);

        printf("modbus_write_register rc:%d addr:%d val:%d\n", rc, addr, val);
        if (rc != 1) {
            printf("ERROR modbus_write_register (%d)\n", rc);
            printf("Address = %d, value = %d (0x%X)\n",
                   addr,
                   tab_rq_registers[0],
                   tab_rq_registers[0]);
            nb_fail++;
        } else {
            rc = modbus_read_registers(ctx, 0, nb, tab_rp_registers);
            printf("modbus_read_registers:%d\n", rc);
            //PrintBuf(tab_rp_registers, nb*2);
            if (rc < 0) {
                printf("ERROR modbus_read_registers single (%d)\n", rc);
                nb_fail++;
            }
        }
        addr++;
        val++;

        printf("Test: ");
        if (nb_fail)
            printf("%d FAILS\n", nb_fail);
        else
            printf("SUCCESS\n");
        sleep(1);
    }

    /* Free the memory */
    free(tab_rq_bits);
    free(tab_rp_bits);
    free(tab_rq_registers);
    free(tab_rp_registers);
    free(tab_rw_rq_registers);

    /* Close the connection */
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}

