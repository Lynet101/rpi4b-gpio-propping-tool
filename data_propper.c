/*  Rpi4B GPIO propping tool

    data_proper.c v1.0 | 16/11/2024 @ 15:37
    Sebastian Lindau-Skands | slindauskands@gmail.com
*/

/*
    DEVELOPERS NOTE:
    cc = clock cycle
    malloc user input of bytes to an array of bools, to quickly "dump" meassured data in <10cc
    `sample_data()` loop should have E_time of <= 100CC to allow for 20MHz on 2.2GHz OC RPI4b
    
    Peripheral base address: 0x7E000000
    
    return:
        -1 = Invalid amount of arguments
        -2 = abandoned by user (no oc on rpi)
*/

#include <complex.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define GPIO_BASE_ADDR 0x7E000000

void sample_amount(int pin_number, int sample_freq, int sample_size, bool **data);
void sample_over_time(int pin_number, int sample_freq, int sample_time, bool **data);
bool sample_data(int pin_number);
void sleep(int sample_freq);
void writeMatrixToFile(char *filename, bool **data, int rows, int cols);

int main(int argc, char *argv[]){
    char opt;
    int sample_freq = 0;
    int sample_size = 0;
    int sample_time = 0;
    int pin_number = 0;
    char *name = NULL;
    bool oc_ack = false;
    bool **data = NULL;

    while ((opt = getopt(argc, argv, "f:n:p:s:t:a")) != -1){
        switch(opt) {
            case 'f':
                sample_freq = atoi(optarg) * 1000;
                break;
            case 'n':
                name = optarg;
                break;
            case 'p':
                pin_number = atoi(optarg);
                break;
            case 's':
                sample_size = atoi(optarg);
                break;
            case 't':
                sample_time = atoi(optarg);
                break;
            case 'a':
                oc_ack = true;
                break;
            default:
                printf("Usage: %s -a [-t <sampling time>/-s <sampling size>] -f <sampling frequency (KHz)> -n <sample name> -p <pin number>\n", argv[0]);
                printf("note that only -t or -s should be specified, not both");
                return -1;
        }
    }

    if (name == NULL || sample_freq == 0 || (sample_time == 0 && sample_size == 0) || pin_number == 0) {
        printf("Usage: %s -a [-t <sampling time>/-s <sampling size>] -f <sampling frequency (KHz)> -n <sample name> -p <pin number>\n", argv[0]);
        printf("note that only -t or -s should be specified, not both");
        return -1;
    }

    if (sample_freq >= 13000000 && !oc_ack) {
        printf("A frequency above 13MHz which requires an overclocked raspberry pi running 2.2GHz. Please use the '-a' flag to acknowledge this warning, and proceed with execution");
        return -2;
    }

    //declare data as matrix of pins (only 1 pin is used currently, but for future this might change)
    //[[pin 1 data], [pin 2 data], ...]
    data = malloc(sizeof(bool*));

    if (sample_time != 0) {
        sample_size = sample_freq * sample_time;
        data[0] = malloc(sample_size * sizeof(bool));
        sample_over_time(pin_number, sample_freq, sample_time, data);
    }
    else {
        data[0] = malloc(sample_size * sizeof(bool));
        sample_amount(pin_number, sample_freq, sample_size, data);
    }

    writeMatrixToFile(name, data, 1, sample_size);

    free(data[0]);
    free(data);

    return 0;
}

void sample_amount(int pin_number, int sample_freq, int sample_size, bool **data) {
    for (int i = 0; i < sample_size; i++) {
        data[0][i] = sample_data(pin_number);
        //sleep(sample_freq);
    }
    return;
}

void sample_over_time(int pin_number, int sample_freq, int sample_time, bool **data) {
    int start_time = time(NULL);
    int current_time;
    int i = 0;

    while (true){
        current_time = time(NULL);
        if ((current_time - start_time) == sample_time) {return;}
        data[0][i] = sample_data(pin_number);
        i++;
        //sleep(sample_freq);
    }
}

bool sample_data(int pin_number) {
    // Placeholder for actual data sampling logic
    return true; // Example return value
}

void sleep(int sample_freq) { //this function is severely broken :shrug:
    #ifdef _WIN32
        Sleep(1000000/sample_freq);
    #else
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = (1000000000/sample_freq);
        nanosleep(&ts, NULL);
    #endif
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
