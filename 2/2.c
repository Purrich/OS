#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>

const int BUFFER_SIZE = 1000;

char* fileName;

void unzip()
{
    int fd = open(fileName, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    char readBuffer[BUFFER_SIZE];
    char writeBuffer[BUFFER_SIZE];

    int i = 0;
    int readCount;
    int charCounter = 0;
    int zeroCounter = 0;

    while (readCount = read(0, readBuffer, BUFFER_SIZE)) {
        while (i < readCount) {
            while (i < readCount && readBuffer[i] != 0) {
                writeBuffer[charCounter] = readBuffer[i];
                charCounter++;
                i++;
            }
            while (i < readCount && readBuffer[i] == 0) {
                zeroCounter++;
                i++;
            }

            if (charCounter > 0) {
                write(fd, writeBuffer, charCounter);
                charCounter = 0;
            }
            if (zeroCounter > 0) {
                lseek(fd, zeroCounter, SEEK_CUR);
                zeroCounter = 0;
            }
        }
    }

    close(fd);
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("Error: invalid arguments!\n");
        printf("To start, print 'gzip -cd <sparse-file.gz> | ./2 <newsparsefile>'\n");
        return 1;
    }

    fileName = argv[1];
    unzip();

    return 0;
}