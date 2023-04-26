#include "type.h"

int head(char *pathname){
    // 1. Open the file for read
    int fd = open_file(pathname, READ); //Opens file to read
    if(fd != READ){ // Makes sure it is open and for
        return -1;
    }
    
    //2. read BLKSIZE into char buf[] = |abcd..\nnxyzw...\n...
    char buf[BLKSIZE];
    int links = 0;
    char firstTenLines[BLKSIZE];

    // 3. scan buf[] for \n; For each \n: linkes++ until 10; add a 0 after LAST \n
    bzero(buf, BLKSIZE);
    myread(fd, buf, BLKSIZE);
    int i = 0;
    while (links < 10 && i < BLKSIZE)
    {
        if(buf[i] == '\n'){
            links++;
        }
        if(links == 10){
            strncpy(firstTenLines, buf, i+1);
            return 0;
        }
    }
    
    //4. print buf as %s
    printf("%s", firstTenLines);
    return 0;
}

int tail(){
    return 0;
}