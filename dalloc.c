#include "type.h"

// src: textbook
// tst_bit, set_bit functions
int tst_bit(char *buf, int bit){
return buf[bit/8] & (1 << (bit % 8));
}

// src: textbook
int set_bit(char *buf, int bit){
buf[bit/8] |= (1 << (bit % 8));
}

// src: textbook
int clr_bit(char *buf, int bit) // clear bit in char buf[BLKSIZE]
{ buf[bit/8] &= ~(1 << (bit%8)); }

// src: website
int incFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // inc free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count++;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count++;
  put_block(dev, 2, buf);
}

// src: website
int idalloc(int dev, int ino)  // deallocate an ino number
{
  int i;  
  char buf[BLKSIZE];

  // return 0 if ino < 0 OR > ninodes

  // get inode bitmap block
  get_block(dev, bmap, buf);
  clr_bit(buf, ino-1);

  // write buf back
  put_block(dev, imap, buf);

  // update free inode count in SUPER and GD
  incFreeInodes(dev);
}

// src: website
int bdalloc(int dev, int blk) // deallocate a blk number
{
  // WRITE YOUR OWN CODE to deallocate a block number blk
}