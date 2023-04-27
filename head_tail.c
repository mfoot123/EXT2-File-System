#include "type.h"

int head(char *pathname) 
{
    // Open the file for read
    int fd = open_file(pathname, READ); // Opens file to read
    if (fd < 0) {
        perror("open");
        return -1;
    }

    char buf[BLKSIZE];
    // Read BLKSIZE bytes into buf
    int n = myread(fd, buf, BLKSIZE);
    if (n < 0) {
        perror("read");
        close_file(fd);
        return -1;
    }

    // Scan buf for newline characters and count the number of lines
    int lines = 0;
    char *cp = buf;
    while (*cp != '\0') {
        if (*cp == '\n') {
            lines++;
            if (lines >= 10) {
                *cp = '\0';
                break;
            }
        }
        cp++;
    }

    // Print buf as a string
    printf("=======================================================\n");
    printf("%s\n", buf);
    printf("=======================================================\n");

    close_file(fd);
    return 0;
}

int tail(char *pathname) 
{
    // 1. open file for READ;
    int fd = open_file(pathname, READ);
    if (fd < 0) {
        printf("ERROR: Failed to open file.\n");
        return -1;
    }

    // 2. get file_size (in its INODE.i_size)
    MINODE *mip = path2inode(pathname);
    int fileSize = mip->INODE.i_size;

    // 3. lssek to (file_size - BLKSIZE)      (OR to 0 if file_size < BLKSIZE)
    long offset = fileSize >= BLKSIZE ? fileSize - BLKSIZE : 0;
    myLSeek(fd, offset);

    /*
                                                                   n 	
    4. n = read BLKSIZE into buf[ ]=|............abc\n1234\nlast\n|0|
                                                                   |
                                                            char *cp
    */
    char buf[BLKSIZE + 1];
    int n = myread(fd, buf, BLKSIZE);
    if (n < 0) {
        printf("ERROR: Failed to read file.\n");
        return -1;
    }
    printf("%d", n);
    buf[n] = '\n';

    // 5. scan buf[ ] backwards for \n;  lines++  until lines=11
    int lines = 0, i = n - 1;
    while (i >= 0 && lines < 11) {
        if (buf[i] == '\n') {
            lines++;
        }
        i--;
    }

    // 6. print from cp+1 as %s
    printf("=======================================================\n");
    printf("%s", &buf[i + 1]);
    printf("=======================================================\n");

    // Clean up
    close_file(fd);
    iput(mip);

    return 0;
}

