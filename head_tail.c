#include "type.h"

int head(char *pathname){
    // 1. Open the file for read
    int fd = open_file(pathname, READ); // Opens file to read
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
        if (links == 10)
        {
            strncpy(firstTenLines, buf, i+1);
            break;
        }
        i++;
    }

    //4. print buf as %s
    printf("=======================================================\n");
    printf("%s\n", firstTenLines);
    printf("=======================================================\n");
    return 0;
}

int tail(){
    /*
    1. open file fore READ;
    2. get file_size (in its INODE.i_size)
    3. lssek to (file_size - BLKSIZE)      (OR to 0 if file_size < BLKSIZE)

                                   n
4. n = read BLKSIZE into buf[ ]=|............abc\n1234\nlast\n|0|
                                                               |
                                                        char *cp

5. scan buf[ ] backwards for \n;  lines++  until lines=11
6. print from cp+1 as %s
    */
    return 0;
}