#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define BUFFERSIZE 64

void checkArgs(int argc, char **argv) {
    if (argc <= 1 || argc > 3) {
        perror("Missing arguments or too many arguments! Please check your input again!");\
        exit(1);
    }

    // TODO: Check more stuffs
}

void printToFile(char *wrappedString, int stringSize) {
    for (int x = 0; x < stringSize; x++) {
        printf("%c", wrappedString[x]);
    }
    printf("\n");
}

void readFile(int fd, int colSize) {
    char buffer[BUFFERSIZE];

    ssize_t numBytesRead;
    char *wrappedString = calloc(sizeof(char), colSize);
    size_t wrappedStringSize = 0;
    // wrappedString = [101 46 10 84 104 97 116 32 32 32 32 32 32 32 32 32 32 32 32 32 105 115 10 103 111 111 100 46]
    /*
     *
     */
    while ((numBytesRead = read(fd, buffer, BUFFERSIZE)) > 0) {
        for (int x = 0; x < numBytesRead; x++) {
            if (buffer[x] != ' ') {
                wrappedString[wrappedStringSize++] = buffer[x];
            } else if (wrappedStringSize == 0 && buffer[x] == 32) {
                continue;
            } else if (buffer[x] == 32 && wrappedString[wrappedStringSize - 1] != 32) {
                wrappedString[wrappedStringSize++] = buffer[x];
            }

            if (wrappedStringSize == colSize || x + 1 == numBytesRead) {
                printToFile(wrappedString, wrappedStringSize);
                memset(wrappedString, 0, wrappedStringSize);
                wrappedStringSize = 0;
            }
        }
    }
}

int main(int argc, char **argv) {
    // checkArgs(argc, argv);
    readFile(open(argv[2], O_RDONLY), atoi(argv[1]));

	return 0;
}

/*
 * [lorem ispum menny sucks]
 * Size 8 -> [lorem is]
 *
 *
 *
 */