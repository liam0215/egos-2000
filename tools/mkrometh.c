/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: create the bootROM image files
 * The ROM on the Arty board has 16MB:
 *     the first 4MB holds the FE310 processor;
 *     the next  4MB holds the earth layer binary executable;
 *     the next  4MB holds the disk image produced by mkfs;
 *     the last  4MB is currently unused.
 * The output is in binary (bootROM.bin) and Intel MCS-86 (bootROM.mcs) format.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/stat.h>

#define SIZE_1MB  1 * 1024 * 1024
#define SIZE_4MB  4 * 1024 * 1024

char mem_fe310[SIZE_1MB];
int fe310_size;
char fe310_file[] = "fpga.bit";

void load_fe310();
void write_binary();
void write_intel_mcs();
int load_file(char* file_name, char* print_name, char* dst);

int main() {
    load_fe310();

    assert(fe310_size <= SIZE_1MB);

    write_binary();
    write_intel_mcs();
    return 0;
}

void write_binary() {
    freopen("bootROM.bin", "w", stdout);

    for (int i = 0; i < SIZE_1MB; i++) putchar(mem_fe310[i]);

    fclose(stdout);
    fprintf(stderr, "[INFO] Finish making the bootROM binary\n");
}

void write_mcs_section(char* mem, int base, int size);
void write_intel_mcs() {
    freopen("bootROM.mcs", "w", stdout);

    write_mcs_section(mem_fe310, 0x00, fe310_size);
    printf(":00000001FF\n");
    
    fclose(stdout);
    fprintf(stderr, "[INFO] Finish making the bootROM mcs image\n");
}

void write_mcs_section(char* mem, int base, int size) {
    for (int i = 0; i < (size >> 16) + 1; i++) {
        printf(":02000004%.4X%.2X\n", i + base, 0xFF);
        for (int j = 0; j < 0x10000; j += 16) {
            printf(":10%.4X00", j);
            for (int k = 0; k < 16; k++)
                printf("%.2X", mem[i * 0x10000 + j + k] & 0xFF);
            /* Use a dummy checksum */
            printf("%.2X\n", 0xFF);
            if (i * 0x10000 + j + 16 >= size) return;
        }
    }    
}

void load_fe310() {
    /* load the fe310 binary file */
    struct stat st;
    stat(fe310_file, &st);
    int len = (int)st.st_size;
    //printf("[INFO] FE310 binary file has %d bytes\n", len);

    /* load header */
    freopen(fe310_file, "r", stdin);
    int first, second;
    first = getchar();
    second = getchar();
    int length = (first << 8) + second;
    for (int i = 0, tmp; i < length; i++)
        tmp = getchar();

    /* load key length */
    first = getchar();
    second = getchar();
    int key_len = (first << 8) + second;
    assert(key_len == 1);

    while (1) {
        int key = getchar();
        first = getchar();
        second = getchar();
        if ((char)key == 'e')
            break;
        
        length = (first << 8) + second;
        for (int i = 0, tmp; i < length; i++)
            tmp = getchar();
    }

    int third, fourth;
    third = getchar();
    fourth = getchar();
    fe310_size = (first << 24) + (second << 16) + (third << 8) + fourth;
    printf("[INFO] FE310 binary has 0x%.6x bytes\n", fe310_size);

    for (int i = 0; i < fe310_size; i++)
        mem_fe310[i] = getchar();
    fclose(stdin);
}