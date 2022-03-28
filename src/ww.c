#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>

#define BUFFERSIZE 64

struct wrappedString {
    char *string;
    size_t size;
};

void checkWriteSuccess(ssize_t writeValue) {
    if (writeValue == -1) {
        perror("Write failed!");
        exit(2);
    }
}

char checkArgs(int argc, char **argv) {

    struct stat sb;

    if (argc <= 1 || argc > 3) {
        perror("Missing arguments or too many arguments! Please check your input again!");\
        exit(1);
    }

    //do we need to check is argv[1] is an integer as well or is that assumed?
    if(atoi(argv[1])<1){//if argv is negative
        perror("Column size argument should be a positive integer! Please check your input again!");
        exit(1);
    } 

    if(argc==3){ //if there are 2 args
        stat(argv[2], &sb);
        if(S_ISREG(sb.st_mode)){ //second arg is a reg file
            return 'f'; //f for file
        }else if (S_ISDIR(sb.st_mode)){ //second arg is a directory
            return 'd'; //d for directory
        }else{ //checks if file or directory exists
            perror("Second argument should be an existing regular file or a directory! Please check your input!");
            exit(1);
        }
    }else if(argc<3){ //if there is only one arg
        return 'e'; //e for empty
    }

    perror("How did you even get here?");
    exit(1); //if it reaches here for some reason exit
}

char* readPathName(char* dir, char* de){ //function that creates pathname so i can go through files in dir
    int sizeOfString = strlen(dir) + strlen(de) + 2; //add 2 bc terminator and backslash
    char* newString = malloc(sizeOfString*sizeof(char)); // allocate mem

    //make string by strcat
    newString[0]='\0';
    strcat(newString, dir);
    strcat(newString, "/");
    strcat(newString, de);
    return newString;
}

char* writePathName(char* dir, char* de){
    char *prefix = "wrap.";
    int sizeOfString = strlen(dir) + strlen(de) + strlen(prefix) + 2; //add 2 bc terminator and backslash
    char* newString = malloc(sizeOfString*sizeof(char)); // allocate mem

    //make string by strcat
    newString[0]='\0';
    strcat(newString, dir);
    strcat(newString, "/");
    strcat(newString,prefix);
    strcat(newString, de);
    return newString;
}

void checkIfMemoryAllocationFailed(void * ptr) {
    if (ptr == NULL) {
        perror("Memory allocation failed! Exiting!");
        exit(2); // Exit code zero is for if the word is larger than the column width
    }
}

int wrapFile(int fd, size_t colSize, int wfd) {
    int status = 0;

    char buffer[BUFFERSIZE];

    ssize_t numBytesRead;
    struct wrappedString ws;
    ws.string = calloc(colSize, sizeof(colSize));
    checkIfMemoryAllocationFailed(ws.string);
    ws.size = 0;

    char *currentWord = NULL;
    size_t wordSize = 0;
    int consecutiveNewLines = 0;

    while ((numBytesRead = read(fd, buffer, BUFFERSIZE)) > 0) {
        for (int x = 0; x < numBytesRead; x++) {
            if (buffer[x] == '\n') {
                consecutiveNewLines++;

                if (consecutiveNewLines == 2) {
                    if (ws.size > 0) {
                        checkWriteSuccess(write(wfd, ws.string , ws.size));
                        checkWriteSuccess(write(wfd, "\n", 1));
                        ws.string = memset(ws.string, 0, colSize);
                        ws.size = 0;
                    }

                    checkWriteSuccess(write(wfd, "\n", 1));
                }
            }

            if (!isspace(buffer[x])) {
                wordSize++;

                currentWord = realloc(currentWord, wordSize + 1);

                checkIfMemoryAllocationFailed(currentWord);

                currentWord[wordSize - 1] = buffer[x];
                currentWord[wordSize] = '\0';

                consecutiveNewLines = 0;
            } else if (buffer[x] == ' ' || buffer[x] == '\n') {
                if (wordSize + 1 > colSize) {
                    status = 1;

                    if (ws.size > 0) {
                        checkWriteSuccess(write(wfd, ws.string , ws.size));
                        checkWriteSuccess(write(wfd, "\n", 1));
                        ws.string = memset(ws.string, 0, colSize);
                        ws.size = 0;
                    }

                    checkWriteSuccess(write(wfd, currentWord , wordSize));
                    checkWriteSuccess(write(wfd, "\n", 1));
                    currentWord = memset(currentWord, 0, wordSize);
                    wordSize = 0;
                }


                if (wordSize > 0) { // What if we focused on here?
                    if (ws.size + wordSize + 1 <= colSize) {
                        if (ws.size > 0) {
                            ws.string[ws.size] = ' ';
                        }
                        ws.string = strcat(ws.string, currentWord);

                        if (ws.size == 0) {
                            ws.size += wordSize;
                        } else {
                            ws.size += wordSize + 1;
                        }

                    } else {
                        checkWriteSuccess(write(wfd, ws.string , ws.size));
                        checkWriteSuccess(write(wfd, "\n", 1));

                        ws.string = memset(ws.string, 0, colSize);
                        ws.string = memcpy(ws.string, currentWord, wordSize);
                        ws.size = wordSize;
                    }

                    currentWord = memset(currentWord, 0, wordSize);
                    wordSize = 0;
                }
            }
        }
    }

    if (status == 1){
        checkWriteSuccess(write(wfd, ws.string , ws.size));
    }else{
        checkWriteSuccess(write(wfd, ws.string , ws.size));
        checkWriteSuccess(write(wfd, "\n", 1));
    }

    free(ws.string);
    free(currentWord);
    close(fd);

    return status;
}

int wrapDirectory(DIR *dir, char* dirName, int colSize){

    struct dirent *de;
    struct stat sb;
    de = readdir(dir);
    int status = 0; //this is put here so we know to return 1 or not if one of the files contains a word size larger than colsize

    while (de!=NULL) { //while have not read last entry
    
        //skips files that start with . and start with wrap.
        while(de!=NULL&&(strncmp (de->d_name, ".",1)==0||strncmp(de->d_name, "wrap.", 5)==0)){
            de=readdir(dir);
        }

        //if directory is null bc last entry starts with . or wrap. break out of loop
        if (de==NULL){
            break;
        }

        //get path to files in dir and file info
        char *rpath = readPathName(dirName, de->d_name);
        stat(rpath, &sb);

        //checks if file entry is regular file and only lists and read reg files
        if(S_ISREG(sb.st_mode)){

            //get path for new wrap. file in directory and open them
            char *wpath = writePathName(dirName, de->d_name);    
            int wfd = open(wpath, O_WRONLY|O_CREAT|O_APPEND|O_TRUNC,S_IRWXU); 
            
            //wraps file and if file contains a word larger than colsize then we return 1
            int tempstatus = wrapFile(open(rpath, O_RDONLY), colSize, wfd);
            if(tempstatus==1){
                status = 1;
            }
            free(wpath);
            
        }
        de = readdir(dir);
        free(rpath);

    }
    closedir(dir); //close dir
    
    return status;

}

void printDirEntry(DIR *dir){ //function that i just use to check the contents of my directory 
    struct dirent* de;
    de = readdir(dir);
    while (de != NULL) {
        puts(de->d_name);
        de = readdir(dir);
        }
    puts("\n");
    closedir(dir);
}

int main(int argc, char **argv) {

    //this is just an if on how ww will execute depending on the type of argument
    char mode = checkArgs(argc, argv);
    if (mode == 'f'){
        int wfd = open("/dev/stdout", O_WRONLY|O_APPEND|O_TRUNC);
        return wrapFile(open(argv[2], O_RDONLY), atoi(argv[1]),wfd);
    } else if (mode == 'd'){
        //printDirEntry(opendir(argv[2]));
        return wrapDirectory(opendir(argv[2]), argv[2], atoi(argv[1]));
    } else if (mode =='e'){
        int wfd = open("/dev/stdout", O_WRONLY|O_APPEND|O_TRUNC);
        return wrapFile(open("/dev/stdin", O_RDONLY), atoi(argv[1]),wfd);
    }
}