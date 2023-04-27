#include "type.h"

/****************************************************HELPER FUNCTIONS******************************************************/

// src: textbook
int clr_bit(char *buf, int bit) // clear bit in char buf[BLKSIZE]
{ 
    int byte;
    byte = bit / 8;
    bit = bit % 8;
    if (buf[byte] &= ~(1 << bit))
    {
        return 1;
    }
    return 0;
}

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
  char buf[BLKSIZE];

  // get block bitmap block
  get_block(dev, bmap, buf);

  // clear bit for the given block
  clr_bit(buf, blk - 1);

  // write the block bitmap block back to disk
  put_block(dev, bmap, buf);

  // update free block count in superblock and group descriptor
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count++;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count++;
  put_block(dev, 2, buf);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////