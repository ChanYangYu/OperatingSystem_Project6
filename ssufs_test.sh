#!/bin/bash

gcc -o ssufs_test ssufs_Dtest.c ssufs-ops.c ssufs-disk.c
./ssufs_test

rm -f ssufs
rm -f ssufs_test

