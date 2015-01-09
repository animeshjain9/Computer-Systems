#!/bin/sh

rm -f $PWD/disk1.img.copy
rm -f $PWD/disk2.img.copy

cp disk1.img disk1.copy.img
cp disk1.img disk2.copy.img

disk1=disk1.copy.img
disk2=disk2.copy.img

echo "Testing q1 Homework 4 Read-only access\n"

echo "Testing following functionalities:\n
 ls\n ls-l\n show\n cd\n cd ..\n statfs\n pwd\n
Test Begin:
"


./homework --cmdline $disk1 > /tmp/q1out <<EOF
ls
ls-l
ls-l file.txt
show another-file
show file.txt
cd home
ls-l small-1
ls-l small-2
cd ..
cd work
cd dir-1
pwd
ls
show small-3
show small-4
show small-5
cd /
pwd
statfs
blksiz 17
ls
ls-l
ls-l file.txt
show another-file
show file.txt
cd home
ls-l small-1
ls-l small-2
cd ..
cd work
cd dir-1
pwd
ls
show small-3
show small-4
show small-5
cd /
pwd
statfs
blksiz 1024
ls
ls-l
ls-l file.txt
show another-file
show file.txt
cd home
ls-l small-1
ls-l small-2
cd ..
cd work
cd dir-1
pwd
ls
show small-3
show small-4
show small-5
cd /
pwd
statfs
blksiz 4000
ls
ls-l
ls-l file.txt
show another-file
show file.txt
cd home
ls-l small-1
ls-l small-2
cd ..
cd work
cd dir-1
pwd
ls
show small-3
show small-4
show small-5
cd /
pwd
statfs
EOF

sed -i -e '$a\' /tmp/q1out

./q1-soln --cmdline $disk2 > /tmp/q1-soln <<EOF
ls
ls-l
ls-l file.txt
show another-file
show file.txt
cd home
ls-l small-1
ls-l small-2
cd ..
cd work
cd dir-1
pwd
ls
show small-3
show small-4
show small-5
cd /
pwd
statfs
blksiz 17
ls
ls-l
ls-l file.txt
show another-file
show file.txt
cd home
ls-l small-1
ls-l small-2
cd ..
cd work
cd dir-1
pwd
ls
show small-3
show small-4
show small-5
cd /
pwd
statfs
blksiz 1024
ls
ls-l
ls-l file.txt
show another-file
show file.txt
cd home
ls-l small-1
ls-l small-2
cd ..
cd work
cd dir-1
pwd
ls
show small-3
show small-4
show small-5
cd /
pwd
statfs
blksiz 4000
ls
ls-l
ls-l file.txt
show another-file
show file.txt
cd home
ls-l small-1
ls-l small-2
cd ..
cd work
cd dir-1
pwd
ls
show small-3
show small-4
show small-5
cd /
pwd
statfs
EOF


sed -i -e '$a\' /tmp/q1-soln

diff /tmp/q1out /tmp/q1-soln

if [ $? != 0 ]; then
    echo 'Q1 Read-only test failed - Please check diff result above'
else
    echo 'Q1 Read-only test passed'
fi

rm -f /tmp/q1out
rm -f /tmp/q1-soln
rm -f disk1.copy.img
rm -f disk2.copy.img