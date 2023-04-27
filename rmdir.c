#include "type.h"

/******************************************************REMOVE DIR**********************************************************/

int rmdir(char *pathname) {

  char parent_dir[256], child_name[256];
  MINODE *pip; 
  DIR* dp;
  int pino;

  // 1. get minode of pathname: 
  MINODE *mip = path2inode(pathname);

  printf("mip ino: %d", mip->ino);

  // 2. check DIR type, not BUSY, is empty
  if (!S_ISDIR(mip->INODE.i_mode)) {
      printf("Error: not a dir\n");
      iput(mip);
      return -1;
  }
  if(mip->shareCount > 1)
  {
    printf("Error: Directory is busy\n");
    iput(mip);
    return -1;
  }

  // Check whether directory is empty
  // First, check link count (links_count > 2 means not empty);
  if (mip->INODE.i_links_count > 2) {
      printf("Error: Directory is not empty\n");
      iput(mip);
      return -1;
  }

  // However, links_count = 2 may still have FILEs, so go through its data
  // block(s) to see whether it has any entries in addition to . and ..
  char buf[BLKSIZE], *cp;
  get_block(mip->dev, mip->INODE.i_block[0], buf);
  dp = (DIR *) buf;
  cp = buf;
  while (cp < buf + BLKSIZE) {
    if (dp->inode > 0) {
        if (strcmp(dp->name, ".") != 0 && strcmp(dp->name, "..") != 0) {
          printf("Error: Directory is not empty\n");
          iput(mip);
          return -1;
          }
    }
    cp += dp->rec_len;
    dp = (DIR *) cp;
  }

  // 3. Deallocate the directory's blocks and inode
  for (int i = 0; i < 12; i++) {
    // get parent DIR pino (in mip->INODE.i_block[0])
    if (mip->INODE.i_block[i] != 0) {
      // Deallocate its block and inode
      bdalloc(mip->dev, mip->INODE.i_block[i]);
    }
  }

  idalloc(mip->dev, mip->ino);
  // clear mip->sharefCount to 0, still in cache
  iput(mip);

  // 4. Get parent minode
  strcpy(parent_dir, pathname);
  strcpy(child_name, basename(parent_dir));
  strcpy(parent_dir, dirname(parent_dir));
  pip = path2inode(parent_dir);
  pino = pip->ino;

  printf("We got here");

  //pip = iget(dev, pino);

  // 5. Remove child's entry from parent directory
  rm_child(pip, child_name);

  // 6. Decrement link count and update atime, mtime
  pip->INODE.i_links_count--;
  pip->INODE.i_atime = time(0L);
  pip->INODE.i_mtime = time(0L);
  pip->modified = 1;
  iput(pip);

  // return success
  return 0;
}

int rm_child(MINODE *parent, char *name) {
    int i;
    char buf[BLKSIZE], *cp;
    DIR *dp, *prev_dp;
    INODE *pip = &parent->INODE;
    int found_entry = 0;

    printf("parent ino: %d\n", parent->ino);

    // 1. Search parent INODE's data block(s) for the entry of name
    for (i = 0; i < 12 && pip->i_block[i]; i++) {
        get_block(parent->dev, pip->i_block[i], buf);
        cp = buf;
        dp = (DIR *)buf;
        prev_dp = NULL;
        while (cp < buf + BLKSIZE) {
            printf("dpname: %s\n", dp->name);
            if (strncmp(dp->name, name, dp->name_len) == 0) {
                // Entry with matching name found, set the flag to true
                found_entry = 1;
                break;
            }
            // Move to the next entry in the directory
            prev_dp = dp;
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
        // if found, break
        if (found_entry) {
            break; // Exit the outer loop as well
        }
    }
    // if not found, return an error
    if (found_entry == 0) {
        printf("Error: %s not found\n", name);
        return -1;
    }

    // 2. Erase name entry from parent directory by:
    if (prev_dp) {
        // 2.1. if has a predecessor entry:  add Rlen to predecessor's rlen
        prev_dp->rec_len += dp->rec_len;
    } else {
        // 2.2. First entry in data block
        // (1). if (ONLY entry in data block)
        if (dp->rec_len == BLKSIZE) {
            // deallocate the block
            bdalloc(parent->dev, pip->i_block[i]);
            pip->i_block[i] = 0;
            // reduce parent's file size by BLKSIZE;
            pip->i_size -= BLKSIZE;
        // has trailing entries
        } else {
            // Shift trailing entries left to cover the erased entry
            cp += dp->rec_len;
            prev_dp = dp;
            dp = (DIR *)cp;
            while (cp < buf + BLKSIZE) {
                prev_dp->inode = dp->inode;
                prev_dp->rec_len = dp->rec_len;
                prev_dp->name_len = dp->name_len;
                strncpy(prev_dp->name, dp->name, dp->name_len);
                cp += dp->rec_len;
                prev_dp = dp;
                dp = (DIR *)cp;
            }
            prev_dp->rec_len += dp->rec_len;
        }
        // Shift non-zero blocks upward to cover the hole
        while (i < 11 && pip->i_block[i+1]) {
            pip->i_block[i] = pip->i_block[i+1];
            i++;
        }
        pip->i_block[i] = 0;
    }
    // 3. Write the parent's data block back to disk;
    put_block(parent->dev, pip->i_block[i], buf);
    // mark parent minode MODIFIED for write-back
    parent->modified = 1;

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////