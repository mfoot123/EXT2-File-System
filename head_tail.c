#include "type.h"

int head(char *pathname){
    // 1. Open the file for read
    int fd = open_file(pathname, READ); //Opens file to read
    if(fd != READ){ // Makes sure it is open and for
        return -1;
    }
    
    //2. read BLKSIZE into char buf[] = |abcd..\nnxyzw...\n...

    // 3. scan buf[] for \n; For each \n: linkes++ until 10; add a 0 after LAST \n

    //4. print buf as %s

    return 0;
}
