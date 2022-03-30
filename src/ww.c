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

void checkIfMemoryAllocationFailed(void * ptr) {
    if (ptr == NULL) {
        perror("Memory allocation failed! Exiting!");
        exit(2); // Exit code zero is for if the word is larger than the column width
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
        perror("Column size argument should be a positive integer greater than zero! Please check your input again!");
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
    checkIfMemoryAllocationFailed(newString);

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
    checkIfMemoryAllocationFailed(newString);

    //make string by strcat
    newString[0]='\0';
    strcat(newString, dir);
    strcat(newString, "/");
    strcat(newString,prefix);
    strcat(newString, de);
    return newString;
}

/*
 * Wrap file will take an input file descriptor, a column size, and a file descriptor to write to (in most cases, it will
 * just be stdout, but any file does work). We tokenize each word in the input file, see if that word with the appropriate white
 * whitespace will fit on the current line given the `colSize`. If it does not, it will go to the next line and print the appropriate word
 * on that line.
 */

int wrapFile(int fd, size_t colSize, int wfd) { // The function that actually wraps the file contents
    int status = 0; // Status indicates whether or not the word size is larger than the colSize. In that case, status becomes 1. By default, it is zero.

    char buffer[BUFFERSIZE]; // By default, it is 64 size.

    // String and wrapping setup.
    ssize_t numBytesRead;
    struct wrappedString ws;
    ws.string = calloc(colSize, sizeof(colSize));
    checkIfMemoryAllocationFailed(ws.string); // This function checks for malloc failures.
    ws.size = 0;

    char *currentWord = NULL;
    size_t wordSize = 0;
    int consecutiveNewLines = 0;

    while ((numBytesRead = read(fd, buffer, BUFFERSIZE)) > 0) { // We start reading the file here/
        for (int x = 0; x < numBytesRead; x++) {
            if (buffer[x] == '\n') {
                consecutiveNewLines++;

                if (consecutiveNewLines == 2) { // If we see two new lines in a row, that means we are at a paragraph and need to print out the paragraph.
                    if (ws.size > 0) { // This means we need to flush the current wrapped string before we can continue.
                        checkWriteSuccess(write(wfd, ws.string , ws.size));
                        checkWriteSuccess(write(wfd, "\n", 1));
                        ws.string = memset(ws.string, 0, colSize);
                        ws.size = 0;
                    }

                    checkWriteSuccess(write(wfd, "\n", 1)); // Write newline for next paragraph.
                }
            }

            if (!isspace(buffer[x])) { // This means we are reading a word.
                wordSize++;

                currentWord = realloc(currentWord, wordSize + 1); // For the null terminator... And later a space. Hence, we add 1.

                checkIfMemoryAllocationFailed(currentWord);

                currentWord[wordSize - 1] = buffer[x];
                currentWord[wordSize] = '\0';

                consecutiveNewLines = 0;
            } else if (buffer[x] == ' ' || buffer[x] == '\n') { // This means we are done reading a word and can start the wrapping process.
                if (wordSize + 1 > colSize) { // If the word is bigger than colSize...
                    status = 1;

                    if (ws.size > 0) { // We may need to flush the current wrapped string before we can deal with the current word.
                        checkWriteSuccess(write(wfd, ws.string , ws.size));
                        checkWriteSuccess(write(wfd, "\n", 1));
                        ws.string = memset(ws.string, 0, colSize);
                        ws.size = 0;
                    }

                    checkWriteSuccess(write(wfd, currentWord , wordSize)); // Print...
                    checkWriteSuccess(write(wfd, "\n", 1));
                    currentWord = memset(currentWord, 0, wordSize); // and reset
                    wordSize = 0; // Continue from here.
                }


                if (wordSize > 0) {
                    if (ws.size + wordSize + 1 <= colSize) { // Check to see if the current word will fit inside the wrapped string. If so, concat it into wrapped string.
                        if (ws.size > 0) {
                            ws.string[ws.size] = ' '; // Add a space for the word
                        }
                        ws.string = strcat(ws.string, currentWord);

                        if (ws.size == 0) {
                            ws.size += wordSize; // If wrapped string is empty, the size becomes wordSize.
                        } else {
                            ws.size += wordSize + 1; // Else, wrapped string needs to store the size of the space we added if it's not currently empty.
                        }

                    } else {
                        checkWriteSuccess(write(wfd, ws.string, ws.size)); // Else write string and new line character.
                        checkWriteSuccess(write(wfd, "\n", 1));

                        ws.string = memset(ws.string, 0, colSize); // Reset everything.
                        ws.string = memcpy(ws.string, currentWord, wordSize); // Store the next word we just loaded in...
                        ws.size = wordSize; // Store word size.
                    }

                    currentWord = memset(currentWord, 0, wordSize); // Reset current word once we get here regardless of what happened above.
                    wordSize = 0;
                }
            }
        }
    }

    if (ws.size == 0) {
        checkWriteSuccess(write(wfd, ws.string, ws.size)); // Write the final wrapped string and flush it out. Then write two new lines, one to end the final wrapped string, and one for the new paragraph.
        checkWriteSuccess(write(wfd, "\n", 1));
    } else {
        checkWriteSuccess(write(wfd, ws.string, ws.size)); // Write the final wrapped string and flush it out. Then write two new lines, one to end the final wrapped string, and one for the new paragraph.
        checkWriteSuccess(write(wfd, "\n\n", 2));
    }

    free(ws.string); // Free memory
    free(currentWord);
    close(fd);

    return status; // Was our process successful once we get here? Status indicates that.
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
            int wfd = open(wpath, O_WRONLY|O_CREAT|O_APPEND|O_TRUNC, S_IRWXU);
            
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
        int wfd = open("/dev/stdout", O_WRONLY|O_APPEND|O_TRUNC); // We use the full path names just to be specific and more clear. Hard coding in file descritor 0 or 1 felt weird to us. 
        return wrapFile(open(argv[2], O_RDONLY), atoi(argv[1]),wfd);
    } else if (mode == 'd'){
        //printDirEntry(opendir(argv[2]));
        return wrapDirectory(opendir(argv[2]), argv[2], atoi(argv[1]));
    } else if (mode =='e'){
        int wfd = open("/dev/stdout", O_WRONLY|O_APPEND|O_TRUNC);
        return wrapFile(open("/dev/stdin", O_RDONLY), atoi(argv[1]),wfd);
    }
}
