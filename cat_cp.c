#include "type.h"

int my_cat(char *filename) 
{
    char mybuf[1024], dummy = 0;
    int n;

    // 1. int fd = open filename for READ;
    int fd = open_file(filename, 0);

    // test case
    if (fd < 0) {
        perror("Error: unable to open file");
        return -1;
    }

    /*
        2. while( n = read(fd, mybuf[1024], 1024)){
        mybuf[n] = 0;             // as a null terminated string
        // printf("%s", mybuf);   <=== THIS works but not good
        spit out chars from mybuf[ ] but handle \n properly;
        } 
    */
    while (n = myread(fd, mybuf, 1024)) {
        for (int i = 0; i < n; i++) {
            putchar(mybuf[i]);
        }
        
    }

    // 3. close(fd);
    close_file(fd);
    return 0;
}


int my_cp(char *src, char *dest) 
{
    int n = 0;
    char buf[BLKSIZE];

    // 1. fd = open src for READ;
    int fd = open_file(src, O_RDONLY);

    // 2. gd = open dst for WR|CREAT; 
    int gd = open_file(dest, READ_WRITE);

    if (fd == -1 || gd == -1) {
        if (gd != -1) close_file(gd);
        if (fd != -1) close_file(fd);
        return -1;
    }

    /*
        3. while( n=read(fd, buf[ ], BLKSIZE) ){
            write(gd, buf, n);  // notice the n in write()
        }
    */
    while (n = myread(fd, buf, BLKSIZE)) {
        mywrite(gd, buf, n);
    }

    // close the directories
    close_file(fd);
    close_file(gd);
    return 0;
}