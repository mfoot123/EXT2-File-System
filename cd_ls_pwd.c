#include "cd_ls_pwd.h"

int cd()
{
  printf("cd: Under Construction\n");

  // write YOUR code for cd
}

int ls_file(MINODE *mip, char *name)
{
  printf("ls_file: under construction\n");

  // use mip->INODE to ls_file
}

int ls_dir(MINODE *pip)
{
  char sbuf[BLKSIZE], name[256];
  DIR  *dp;
  char *cp;

  printf("simple ls_dir()\n");

  get_block(dev, pip->INODE.i_block[0], sbuf);
  dp = (DIR *)sbuf;
  cp = sbuf;

  while (cp < sbuf + BLKSIZE){
    strncpy(name, dp->name, dp->name_len);
    name[dp->name_len] = 0;
    printf("%s\n",  name);

    cp += dp->rec_len;
    dp = cp;
  }

}

int ls()
{
  MINODE *mip = running->cwd;

  ls_dir(mip);
  iput(mip);
}

int pwd()
{
  printf("CWD = %s\n", "/");
}