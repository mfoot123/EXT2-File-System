#include "type.h"

int head(char *pathname){
    // 1. Open the file for read
    int fd = open_file(pathname, READ); // Opens file to read
    if(fd != READ){ // Makes sure it is open and for
        return -1;
    }

    char buf[BLKSIZE];
    int links = 0;
    char firstTenLines[BLKSIZE];

    // 2. read BLKSIZE into char buf[] = |abcd..\nnxyzw...\n...
    bzero(buf, BLKSIZE);
    myread(fd, buf, BLKSIZE);

    // 3. scan buf[] for \n; For each \n: linkes++ until 10; add a 0 after LAST \n
    int i = 0;
    while (links < 10 && i < BLKSIZE)
    {
        if (buf[i] == NULL)
        {
            break;
        }
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
    bzero(buf, BLKSIZE);
    close_file(fd);
    return 0;
}

int tail(char *pathname){
    
    // 1. open file fore READ;
    int fd = open_file(pathname, READ); // Opens file to read
    if(fd != READ){ // Makes sure it is open and for
        printf("ERROR: File Discriptor is not opened\n");
        return -1;
    }

    // 2. get file_size (in its INODE.i_size)
    MINODE *mip = path2inode(pathname);
    int fileSize = mip->INODE.i_size;
    int position;
    if (fileSize > BLKSIZE)
    {
        printf("ERROR: FileSize is less than the block's\n");
        position = myLSeek(fd, 0);
        // return -1;
    }

    // 3. lssek to (file_size - BLKSIZE)      (OR to 0 if file_size < BLKSIZE)
    position = myLSeek(fd, fileSize - BLKSIZE);

    //lseek(mip->dev, fileSize - BLKSIZE, SEEK_SET);

    //                                                                n
    // 4. n = read BLKSIZE into buf[ ]=|............abc\n1234\nlast\n|0|
    //                                                                |
    //                                                         char *cp
    char buf[BLKSIZE];
    int links = 0;
    char firstTenLines[BLKSIZE];
    bzero(buf, BLKSIZE);
    myread(fd, buf, BLKSIZE);

    // 5. scan buf[ ] backwards for \n;  lines++  until lines=11
    int i = fileSize;
    while (links < 11)
    {
        if(buf[i] == NULL)
        {
            break;
        }
        if (buf[i] == '\n')
        {
            links++;
        }
        if (links == 11)
        {
            strncpy(firstTenLines, buf, i + 1);
            break;
        }
        i--;
    }

    // 6. print from cp+1 as %s
    printf("=======================================================\n");
    printf("%s\n", firstTenLines);
    printf("=======================================================\n");

    bzero(buf, BLKSIZE);
    close_file(fd);
    return 0;
}