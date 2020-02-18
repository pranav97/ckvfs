Main data structures in this file system are 
1. Blob - Each file is a blob. 
    
        Key - Hash of the file
        
        Data - is the real data that was loaded into the file

2. TreeBlob - Each dir is a TreeBlob

        Key - The name of the directory

        Data - A string version of a list of files and dirs in the TreeBlob.

Writing TreeBlob to disk - 
```
struct TreeBlob {
        char *names[];
        char *

}

```
1. On creating the dir we would create an empty treeblob with no data
2. Once it is a directory, it has to be 
    a. Loaded from disk
    b. Updated with any new content there

Loading TreeBlob from disk - On loading the tree blob from disk (on listdir for example) we will parse the string that was stored to get the list of files (and the metadata of each of them)


### Problems

1. If there are a lot of directories we might not be able to stringify the list of directories and files to store on disk. 



### Fuse functions that I have to build
1. Open - check if the path exists in the system  
	- returns -ENOENT if path does not exist
    - returns 0 otherwise
    - Checks permissions. But this file system doesn’t check this (sorry). If they are in access more and are requesting a read only path, we have to return -EACCES;


