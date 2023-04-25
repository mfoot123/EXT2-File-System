#include "type.h"

int my_cat(char *filename) {
    char mybuf[1024], dummy = 0;
    int n;

    // 1. int fd = open filename for READ;
    int fd = open_file(filename, READ);
    printf("Maybe works: %d", fd);
    // check that the file is open
    if (fd < 0) {
        printf("Failed to open file: %s\n", filename);
        return -1;
    }
    
    // 2. while( n = read(fd, mybuf[1024], 1024)){
    while ((n = read(fd, mybuf, 1024)) > 0) {
        // mybuf[n] = 0;             // as a null terminated string
        mybuf[n] = 0;
        for (int i = 0; i < n; i++) {
            // handle \n properly
            if (mybuf[i] == '\n') {
                putchar('\n');
            } else {
                // spit out chars from mybuf[ ]
                putchar(mybuf[i]);
            }
        }
    }

    // 3. close(fd);
    close(fd);
    return 0;
}

int my_cp(char *src, char *dest)
{
    // variables:
    int fd, gd, n;
    char buf[BLKSIZE];

    // 1. fd = open src for READ;
    if ((fd = open(src, O_RDONLY)) == -1) {
        printf("Error: fd O_RDONLY == -1\n");
        return -1;
    }

    // 2. gd = open dst for WR|CREAT; 
    if ((gd = open(dest, O_WRONLY | O_CREAT, 0644)) == -1) {
        printf("Error: gd O_WRONLY | O_CREAT == -1\n");
        close(fd);
        return -1;
    }

    /* 3. while( n=read(fd, buf[ ], BLKSIZE) ){
          write(gd, buf, n);  // notice the n in write()
          }
    */
    while ((n = read(fd, buf, BLKSIZE)) > 0) 
    {
        if (write(gd, buf, n) != n) 
        {
            printf("Error: write != n\n");
            close(fd);
            close(gd);
            return -1;
        }
        else
        write(gd, buf, n);
    }

    // Check for errors during reading
    if (n < 0) {
        printf("Error: n < 0\n");
        close(fd);
        close(gd);
        return -1;
    }

    // Close files and return success
    close(fd);
    close(gd);
    return 0;
}
