/*********** globals in main.c ***********/
extern PROC   proc[NPROC];
extern PROC   *running;

extern MINODE minode[NMINODE];   // minodes
extern MINODE *freeList;         // free minodes list
extern MINODE *cacheList;        // cacheCount minodes list

extern MINODE *root;

extern OFT    oft[NOFT];

extern char gline[256];   // global line hold token strings of pathname
extern char *name[64];    // token string pointers
extern int  n;            // number of token strings

extern int ninodes, nblocks;
extern int bmap, imap, inodes_start, iblk;  // bitmap, inodes block numbers

extern int  fd, dev;
extern char cmd[16], pathname[128], parameter[128];
extern int  requests, hits;

#define BLKSIZE 1024

char buf[BLKSIZE];

/*****************************************************************************************************************/

/***********************CD**************************/

// takes the path for the dir to change to
// return 0 on success -1 on failure
int cd(char *pathname)
{
    int ino;
    MINODE *mip;
    int dev = running->cwd->dev;

    // (1). Call path_to_inode to get the inode number and device
    ino = path2inode(pathname);

    // return error if ino=0
    if (ino == 0) {
        printf("%s: No such file or directory\n", pathname);
        return -1;
    }

    // (2). MINODE *mip = iget(dev, ino);
    // Get the inode of the directory
    mip = iget(dev, ino);

    // (3). Verify mip->INODE is a DIR // return error if not DIR
    // Verify that the inode is a directory
    if (!S_ISDIR(mip->INODE.i_mode)) {
        printf("%s: Not a directory\n", pathname);
        // release the mip
        iput(mip);
        return -1;
    }

    // Release the old cwd
    iput(running->cwd);

    // Change cwd to mip
    running->cwd = mip;

    return 0;
}

/***************************************************/

/**********************PWD**************************/

int pwd(MINODE *wd){
    // call rpwd
    rpwd(wd);

    return 0;
}

void rpwd(MINODE *wd)
{
    // variable definitions
    int ino;
    MINODE *pip;
    char temp[256];
    char myname[256];

    // (1). if (wd==root) return;
    if (wd == root) {
        // print the root
        printf("/\n");
        return;
    }

    //(2). from wd->INODE.i_block[0], get my_ino and parent_ino
    ino = search(wd, "..");

    // (3). pip = iget(dev, parent_ino);
    // Get the parent inode
    pip = iget(dev, ino);

    // (4). from pip->INODE.i_block[ ]: get my_name string by my_ino as LOCAL
    ino = search(pip, ".");
    get_block(dev, pip->INODE.i_block[0], buf);

    // dp is initialized to point to the start of buf
    DIR *dp = (DIR *)buf;
    // cp is initialized to point to the start of buf
    char *cp = buf;
    // searches through the directory block data in buf
    // to find the entry with the desired inode number ino
    // updates dp to point to that directory entry
    while (cp < buf + BLKSIZE && dp->inode != ino) {
        // update cp to point to the next directory entry
        cp += dp->rec_len;
        // update dp to point to the directory entry that cp now points to
        dp = (DIR *)cp;
    }

    // copy name to temp
    strncpy(myname, dp->name, dp->name_len);
    myname[dp->name_len] = '\0';

    // (5). rpwd(pip); // recursive call rpwd(pip) with parent minode
    // Recursively print the path to the parent directory
    rpwd(pip);

    // (6). print "/%s", my_name;
    // Print the name of the current directory
    printf("/%s", myname);

    iput(pip);
}

/**************************************************/

/***********************LS**************************/

int ls(char *pathname)
{
    
    MINODE *mip = running->cwd;
    if (!strcmp(pathname, "")){
        ls_dir(mip);
        return 0;
    }
    //    ls_dir(mip);
    // iput(mip);
    // MINODE *mip = running->cwd;
    int ino;

    // the inode number of the specified path
    ino = path2inode(pathname);

    // get the MINODE corresponding to the inode number returned
    mip = iget(dev, ino);
    // store inode number in inode var
    INODE inode = mip->INODE;

    // check if its a file or directory
    if (S_ISDIR(inode.i_mode))
    {
        // if its a directory call ls_dir
        ls_dir(mip);
    }
    else if(S_ISREG(inode.i_mode)){
        ls_file(mip, name);
    }
    else
    {
        printf("Error, not a file or directory");
        return 0;
    }

    iput(minode);
    return 0;
}

int ls_file(MINODE *mip, char *name)
{
    char ftime[64];
    struct stat fstat, *sp;
    sp = &fstat;
    // use mip->INODE to ls_file

    // Check if the inode is a regular file
    if (!S_ISREG(mip->INODE.i_mode))
        putchar('-');
    // Check if the inode is a directory
    else if (!S_ISDIR(mip->INODE.i_mode))
        putchar('d');
    // Check if the inode is a link
    else if (!S_ISLNK(mip->INODE.i_mode)){
        putchar('l');
    }

    if(mip->INODE.i_mode & 0x0001)
    {
        putchar('x');
    }
    else{
        putchar('-');
    }

    if(mip->INODE.i_mode & 0x0002){
        putchar('w');
    }
    else{
        putchar('-');
    }

    if(mip->INODE.i_mode & 0x0004){
        putchar('r');
    }
    else{
        putchar('-');
    }

    // Print the file attributes
    printf("%4d", mip->INODE.i_links_count);
    printf("%4d", mip->INODE.i_uid);
    printf("%4d", mip->INODE.i_gid);

    strcpy(ftime, ctime((time_t*)&(mip->INODE.i_mtime)));
    ftime[strlen(ftime) - 1] = 0;
    printf("%s  ", ftime);

    printf("%8d  ", mip->INODE.i_size);

    printf("%s  ", name);

    printf("[%d %d]\n", mip->dev, mip->ino);

    return 0;
}

int ls_dir(MINODE *pip)
{  
  char sbuf[BLKSIZE], name[256];
  DIR  *dp;
  char *cp;

  printf("ls ./\n");
  printf("tokensize: echo names: .\n");
  printf("search for %s in inode# %d\n", pathname, pip->id);

  get_block(dev, pip->INODE.i_block[0], sbuf);
  printf("i_block[0] = %d\n", pip->INODE.i_block[0]);
  dp = (DIR *)sbuf;
  cp = sbuf;

  show_dir(pip);
  printf("\n");
  printf("i_block[0] = %d\n", iblk);
  MINODE *pipAssist = pip;
  while (cp < sbuf + BLKSIZE){
        // printf("%d", dp->rec_len);
        strncpy(name, dp->name, dp->name_len);
        name[dp->name_len] = 0;
        pipAssist = iget(dev, dp->inode);
        // ls_file(pipAssist, name);
        cp += dp->rec_len;
        dp = cp;
  }
}

/***************************************************/
