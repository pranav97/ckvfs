# ckvfs
This repo is for the C implementation of the python kvfs - https://github.com/qznc/kvfs

There is a paper that goes along this this that explains all the design decisions while building this prototype. 

# Running instructions
1. Clone the drivers in the parent directory of this repo (or somewhere else)

```git clone https://github.com/OpenMPDK/KVSSD ../KVSSD```

2. Copy build_emul.bash to the right location in the KVSSD project.

```cp build_emul.bash ../KVSSD/PDK/core/``` 

3. Run the build_emul.bash script

```cd ../KVSSD/PDK/core/```

```bash build_emul.bash```

4. Export the path to the shared library object so that main can see it 

```export LD_LIBRARY_PATH="/src/KVSSD/PDK/core/build"```

5. Build

```make clean; make```

6. Make a directory that will be the mount point of this fs

```mkdir /tmp/mountpt```

7. A absolute path of the *configuration file* is a required command line argument for this script. Please paste that after --conf. 

```./main -d -s -o auto_unmount /tmp/mountpt --config=/src/my_stuff/my-kvfs/kvssd_emul.conf``` 

8. In another terminal you can run the tests that are in this repo 

```cp create*.bash /tmp/```

```cd /tmp/```

```bash create_dirs.bash```

```bash create_files.bash```



