/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: create the bootROM image file (egos_bootROM.mcs)
 * the bootROM has 16MB
 *     the first 4MB is reserved for the FE310 processor
 *     the next 4MB is reserved for the earth layer binary
 *     the next 6MB is reserved for the disk image
 *     the last 2MB is currently unused
 * the output file is in Intel MCS-86 object format
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/stat.h>

char fe310_file[] = "fpga.bit";

char output_bin[] = "bootROM.bin";
char output_mcs[] = "bootROM.mcs";

char mem_fe310[1024 * 1024];

void load_fe310();
void write_binary();
void write_intel_mcs();
void write_section(char* mem, int base, int size);

int fe310_size;

int main() {
    load_fe310();
    write_binary();
//    write_intel_mcs();
    
    return 0;
}

void write_binary() {
    freopen(output_bin, "w", stdout);

    for (int i = 0; i < 1024 * 1024; i++)
        putchar(mem_fe310[i]);

    fclose(stdout);
    fprintf(stderr, "[INFO] Finish making the bootROM binary\n");
}

void write_intel_mcs() {
    freopen(output_mcs, "w", stdout);

    write_section(mem_fe310, 0x00, fe310_size);
    printf(":00000001FF\n");
    
    fclose(stdout);
    fprintf(stderr, "[INFO] Finish making the bootROM mcs image\n");
}

void write_section(char* mem, int base, int size) {
    /* using a dummy checksum */
    char chk;

    int ngroups = (size >> 16) + 1;
    for (int i = 0; i < ngroups; i++) {
        printf(":02000004%.4X%.2X\n", i + base, chk & 0xff);
        for (int j = 0; j < 0x10000; j += 16) {
            printf(":10%.4X00", j);
            for (int k = 0; k < 16; k++)
                printf("%.2X", mem[i * 0x10000 + j + k] & 0xff);
            printf("%.2X\n", chk & 0xff);
            if (i * 0x10000 + j + 16 >= size)
                return;
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