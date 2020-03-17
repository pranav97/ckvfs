# ckvfs
This repo is for the C implementation of the python kvfs - https://github.com/qznc/kvfs

There is a paper that goes along this this that explains all the design decisions while building this prototype. 

# Running instructions
1. Clone the drivers in the parent directory of this repo (or somewhere else)

```git clone https://github.com/OpenMPDK/KVSSD ../KVSSD```

2. Move build_emul.bash to the right location in the KVSSD project.

```cp build_emul.bash ../KVSSD/PDK/core/``` 

3. Export the path to the shared library object so that main can see it 

```export LD_LIBRARY_PATH="/src/KVSSD/PDK/core/build"```

3. Build

```make clean; make```

4. Make a directory that will be the mount point of this fs

```mkdir /tmp/mountpt```

5. A absolute path of the *configuration file* is a required command line argument for this script. Please paste that after --conf. 

```./main -d -s -o auto_unmount /tmp/mountpt --config=/src/my_stuff/my-kvfs/kvssd_emul.conf``` 
6. In another terminal you can run the tests that are in this repo 
```bash create_dirs.bash
    bash create_files.bash```



