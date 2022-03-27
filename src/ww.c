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
        }else{
            perror("Second argument should be a regular file or a directory! Please check your input!");
            exit(1);
        }
    }else if(argc<3){ //if there is only one arg
        return 'e'; //e for empty
    }

    perror("How did you even get here?");
    exit(1); //if it reaches here for some reason exit

    // TODO: Check more stuffs
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

void printToFile(struct wrappedString *ws, int fd, char mode) {
    //method probably gonna be tweaked bc of readFile change 

    if(mode == 'f' || mode == 'e'){ //if there is no present filename or second arg is file then print out to stdout
        for (int x = 0; x < ws->size; x++) {
            printf("%c", ws->string[x]);
        }
        printf("\n");
    }else if (mode =='d'){//if second arg is directory write to wrap. files
        write(fd, ws->string , ws->size);
        write(fd, "\n", 1); 
    }

}

void checkIfMemoryAllocationFailed(void * ptr) {
    if (ptr == NULL) {
        perror("Memory allocation failed! Exiting!");
        exit(2); // Exit code zero is for if the word is larger than the column width
    }
}

int wrapFile(int fd, size_t colSize, int wfd, char mode) {
    int status = 0;

    char buffer[BUFFERSIZE];

    ssize_t numBytesRead;
    struct wrappedString ws;
    ws.string = calloc(colSize, sizeof(colSize));
    checkIfMemoryAllocationFailed(ws.string);
    ws.size = 0;

    char *currentWord = NULL;
    int wordSize = 0, newLineCount = 0;

    while ((numBytesRead = read(fd, buffer, BUFFERSIZE)) > 0) {
        for (int x = 0; x < numBytesRead; x++) {
            if (!isspace(buffer[x])) {
                wordSize++;

                currentWord = realloc(currentWord, wordSize + 1);

                checkIfMemoryAllocationFailed(currentWord);

                currentWord[wordSize - 1] = buffer[x];
                currentWord[wordSize] = '\0';

            } else if (buffer[x] == ' ' || buffer[x] == '\n') {
                if (wordSize > colSize) {
                    status = 1;
                }
                if (wordSize > 0) {
                    if (ws.size + wordSize + 1 < colSize) {
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
                        if(mode == 'f'){
                            printToFile(&ws, wfd, 'f');
                        }else if (mode =='d'){
                            printToFile(&ws, wfd, 'd');
                        }
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
    if(mode == 'f'){
        printToFile(&ws, wfd, 'f');
    }else if (mode =='d'){
        printToFile(&ws, wfd, 'd');
    }

    free(ws.string);
    free(currentWord);

    return status;
}

void wrapDirectory(DIR *dir, char* dirName, int colSize){

    struct dirent *de;
    struct stat sb;
    de = readdir(dir);

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

            wrapFile(open(rpath, O_RDONLY), colSize, wfd, 'd');
            free(wpath);
            
        }
        de = readdir(dir);
        free(rpath);

    }
    closedir(dir); //close dir

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
        wrapFile(open(argv[2], O_RDONLY), atoi(argv[1]),0, 'f');
    }else if (mode == 'd'){
        //printDirEntry(opendir(argv[2]));
        wrapDirectory(opendir(argv[2]), argv[2], atoi(argv[1]));
    }else if (mode =='e'){
        puts("owo");
        //uhhh idk how to do this ngl we'll tweak the code to adjust for this input later
    }
    
	return 0;
}

/*
 * [lorem ispum menny sucks]
 * Size 8 -> [lorem is]
 *
 *
 *
 */