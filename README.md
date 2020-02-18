# ckvfs
This repo is for the C implementation of the python kvfs - https://github.com/qznc/kvfs

# Running instructions
1. Build

    a. ```$ make```

    b. ```$ gcc -Wall main.c `pkg-config fuse3 --cflags --libs` -o main```

2. ```$ ./main -d -s -f /tmp/hellofs2```

