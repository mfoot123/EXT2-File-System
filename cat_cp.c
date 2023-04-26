#include "type.h"

int my_cat(char *filename) 
{
    char mybuf[1024], dummy = 0;
    int n;

    // 1. int fd = open filename for READ;
    int fd = open(filename, O_RDONLY);

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
    while (n = read(fd, mybuf, 1024)) {
        mybuf[n] = 0;
        char *cp = mybuf;
        while (*cp != '\0') {
            // handle \n properly
            if (*cp == '\n') {
                putchar('\n');
            // spit out chars from mybuf[]
            } else {
                putchar(*cp);
            }
            cp++;
        }
    }

    // 3. close(fd);
    close(fd);
    return 0;
}


int my_cp(char *src, char *dest) 
{
    int n = 0;
    char buf[BLKSIZE];

    // 1. fd = open src for READ;
    int fd = open(src, O_RDONLY);

    // 2. gd = open dst for WR|CREAT; 
    int gd = open(dest, O_WRONLY | O_CREAT, 0666);

    if (fd == -1 || gd == -1) {
        if (gd != -1) close(gd);
        if (fd != -1) close(fd);
        return -1;
    }

    /*
        3. while( n=read(fd, buf[ ], BLKSIZE) ){
            write(gd, buf, n);  // notice the n in write()
        }
    */
    while (n = read(fd, buf, BLKSIZE)) {
        write(gd, buf, n);
    }

    // close the directories
    close(fd);
    close(gd);
    return 0;
}