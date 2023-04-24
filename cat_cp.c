#include "type.h"

int my_cat(char *filename) {
    char mybuf[1024], dummy = 0;
    int n;

    // 1. int fd = open filename for READ;
    int fd = open(filename, O_RDONLY);

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