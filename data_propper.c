/*  Rpi4B GPIO propping tool
    data_propper.c v1.0 | 18/11/2024 @ 08:11
    Sebastian Lindau-Skands | slindauskands@gmail.com
*/

/*
    DEVELOPERS NOTE:
    malloc user input of bytes to an array of bools, to quickly "dump" meassured data in <10cc
    `sample_data()` loop should have E_time of <= 100CC to allow for 20MHz on 2.2GHz OC RPI4b
*/

#include <getopt.h>
#include <complex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define GPIO_BASE_ADDR_PI5 0xFE200000
#define GPIO_BASE_ADDR_PI4 0xFE200000
#define GPIO_BASE_ADDR_PI3 0x3F200000
#define GPIO_LEN  0xB4  // Length of the GPIO memory block
#define GPLEV0 0x34  // Offset for the GPIO Pin Level 0 register
#define MAX_FILENAME 256

typedef struct {
    int sample_freq;
    int sample_size;
    int sample_time;
    int pin_number;
    int generation;
    char name[MAX_FILENAME];
    bool oc_ack;
} SamplingConfig;

void sample_amount(int generation, int pin_number, int sample_freq, int sample_size, bool **data);
void sample_over_time(int generation, int pin_number, int sample_freq, int sample_time, bool **data);
volatile unsigned int* mem_map();
bool sample_data(volatile unsigned int *gpio_mm, int generation, int pin_number);
void writeMatrixToFile(char *filename, bool **data, int rows, int cols);
void initialize_config(SamplingConfig *config);

void initialize_config(SamplingConfig *config) {
    config->sample_freq = 150000;
    config->sample_size = 15000;
    config->sample_time = 0;
    config->pin_number = 5;
    config->generation = 4;
    strncpy(config->name, "test", MAX_FILENAME-1);
    config->oc_ack = true;
}

int main(int argc, char *argv[]) {
    SamplingConfig config;
    initialize_config(&config);
    bool **data = NULL;

    if (config.sample_freq == 0 || (config.sample_time == 0 && config.sample_size == 0) ||
        config.pin_number == 0 || config.generation == 0) {
        printf("Usage: %s -a [-t <sampling time>/-s <sampling size>] -f <sampling frequency (KHz)> -n <sample name> -p <pin number> -g <pi generation>\n", argv[0]);
        printf("note that only -t or -s should be specified, not both\n");
        return -1;
    }

    if (config.sample_freq >= 13000000 && !config.oc_ack) {
        printf("A frequency above 13MHz which requires an overclocked raspberry pi running 2.2GHz. Please use the '-a' flag to acknowledge this warning, and proceed with execution");
        return -2;
    }

    data = malloc(sizeof(bool*));

    if (config.sample_time != 0) {
        config.sample_size = config.sample_freq * config.sample_time;
        data[0] = malloc(config.sample_size * sizeof(bool));
        sample_over_time(config.generation, config.pin_number, config.sample_freq, config.sample_time, data);
    } else {
        data[0] = malloc(config.sample_size * sizeof(bool));
        sample_amount(config.generation, config.pin_number, config.sample_freq, config.sample_size, data);
    }

    writeMatrixToFile(config.name, data, 1, config.sample_size);

    free(data[0]);
    free(data);

    return 0;
}

void sample_amount(int generation, int pin_number, int sample_freq, int sample_size, bool **data) {
    volatile unsigned int *gpio_mm = mem_map();
    for (int i = 0; i < sample_size; i++) {
        data[0][i] = sample_data(gpio_mm, generation, pin_number);
        usleep(1000000/sample_freq);
    }
    return;
}

void sample_over_time(int generation, int pin_number, int sample_freq, int sample_time, bool **data) {
    int start_time = time(NULL);
    int current_time;
    int i = 0;
    volatile unsigned int *gpio_mm = mem_map();

    while (true) {
        current_time = time(NULL);
        if ((current_time - start_time) >= sample_time) {
            return;
        }
        data[0][i] = sample_data(gpio_mm, generation, pin_number);
        usleep(1000000/sample_freq);
        i++;
    }
}

volatile unsigned int* mem_map() {
    int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    unsigned int gpio_base;
    if (getuid() != 0) {
        fprintf(stderr, "Program must be run as root\n");
        exit(EXIT_FAILURE);
    }

    gpio_base = GPIO_BASE_ADDR_PI4;

    void *gpio_map = mmap(NULL, GPIO_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, gpio_base);
    close(mem_fd);

    if (gpio_map == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    volatile unsigned int *gpio = (volatile unsigned int *)gpio_map;
    return gpio;
}

bool sample_data(volatile unsigned int *gpio_mm, int generation, int pin_number) {
    unsigned int value = gpio_mm[GPLEV0/4];
    return (value & (1 << pin_number)) != 0;
}

void writeMatrixToFile(char *filename, bool **data, int rows, int cols) {
    sprintf(filename, "%s%s", filename, ".csv");
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fprintf(file, "%d,", data[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}
