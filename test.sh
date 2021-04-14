#!/bin/bash
touch old.txt
echo "1a23b6df" >> old.txt

touch new.txt
echo "ab47c6dd" >> new.txt

touch diff.txt

gcc diff.c -o diff.o
chmod +x diff.o
./diff.o -d old.txt new.txt diff.txt

touch test_restore.txt
./diff.o -r old.txt diff.txt test_restore.txt

echo "New file given at the beginning and new file recovered from old file and difference:"

diff -s test_restore.txt new.txt

