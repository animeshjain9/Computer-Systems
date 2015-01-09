#!/bin/sh

rm -f $PWD/disk1.img.copy
rm -f $PWD/disk2.img.copy

rm -f $PWD/disk11.img.copy
rm -f $PWD/disk21.img.copy

rm -f $PWD/disk12.img.copy
rm -f $PWD/disk22.img.copy

rm -f $PWD/disk13.img.copy
rm -f $PWD/disk23.img.copy

cp disk1.img disk1.copy.img
cp disk1.img disk2.copy.img

cp disk1.img disk11.copy.img
cp disk1.img disk21.copy.img

cp disk1.img disk12.copy.img
cp disk1.img disk22.copy.img

cp disk1.img disk13.copy.img
cp disk1.img disk23.copy.img

disk1=disk1.copy.img
disk2=disk2.copy.img
disk11=disk11.copy.img
disk21=disk21.copy.img
disk12=disk12.copy.img
disk22=disk22.copy.img
disk13=disk13.copy.img
disk23=disk23.copy.img




yes 'testing read write' | head -5 | fmt > /tmp/shortfile
yes 'testing read write' | head -5000 | fmt > /tmp/longfile

echo "Testing q2 Homework 4 Read-Write access\n"

echo "Testing following functionalities:\n
 ls\n mkdir\n put\n cd\n cd ..\n pwd\n
Test Begin:
"


./homework --cmdline $disk1 > /tmp/q2out <<EOF
mkdir test123
cd test123
put /tmp/shortfile testfile1.txt
show testfile1.txt
rm testfile1.txt
ls
cd ..
ls
rmdir test123
ls
mkdir nest1
cd nest1
mkdir nest2
cd nest2
mkdir tmp
put /tmp/longfile
show tmp/longfile
ls
rm tmp/longfile
ls
cd ..
cd ..
rm nest1
ls
EOF

sed -i -e '$a\' /tmp/q2out

./homework --cmdline $disk11 > /tmp/q2out1 <<EOF
blksiz 17
mkdir test123
cd test123
put /tmp/shortfile testfile1.txt
show testfile1.txt
rm testfile1.txt
ls
cd ..
ls
rmdir test123
ls
mkdir nest1
cd nest1
mkdir nest2
cd nest2
mkdir tmp
put /tmp/longfile
show tmp/longfile
ls
rm tmp/longfile
ls
cd ..
cd ..
rm nest1
ls
EOF


sed -i -e '$a\' /tmp/q2out1

./homework --cmdline $disk12 > /tmp/q2out2 <<EOF
blksiz 1024
mkdir test123
cd test123
put /tmp/shortfile testfile1.txt
show testfile1.txt
rm testfile1.txt
ls
cd ..
ls
rmdir test123
ls
mkdir nest1
cd nest1
mkdir nest2
cd nest2
mkdir tmp
put /tmp/longfile
show tmp/longfile
ls
rm tmp/longfile
ls
cd ..
cd ..
rm nest1
ls
EOF

sed -i -e '$a\' /tmp/q2out2

./homework --cmdline $disk13 > /tmp/q2out3 <<EOF
blksiz 4000
mkdir test123
cd test123
put /tmp/shortfile testfile1.txt
show testfile1.txt
rm testfile1.txt
ls
cd ..
ls
rmdir test123
ls
mkdir nest1
cd nest1
mkdir nest2
cd nest2
mkdir tmp
put /tmp/longfile
show tmp/longfile
ls
rm tmp/longfile
ls
cd ..
cd ..
rm nest1
ls
EOF

sed -i -e '$a\' /tmp/q2out3


./q1-soln --cmdline $disk2 > /tmp/q2-soln <<EOF
mkdir test123
cd test123
put /tmp/shortfile testfile1.txt
show testfile1.txt
rm testfile1.txt
ls
cd ..
ls
rmdir test123
ls
mkdir nest1
cd nest1
mkdir nest2
cd nest2
mkdir tmp
put /tmp/longfile
show tmp/longfile
ls
rm tmp/longfile
ls
cd ..
cd ..
rm nest1
ls
EOF

sed -i -e '$a\' /tmp/q2-soln

./q1-soln --cmdline $disk21 > /tmp/q2-soln1 <<EOF
blksiz 17
mkdir test123
cd test123
put /tmp/shortfile testfile1.txt
show testfile1.txt
rm testfile1.txt
ls
cd ..
ls
rmdir test123
ls
mkdir nest1
cd nest1
mkdir nest2
cd nest2
mkdir tmp
put /tmp/longfile
show tmp/longfile
ls
rm tmp/longfile
ls
cd ..
cd ..
rm nest1
ls
EOF

sed -i -e '$a\' /tmp/q2-soln1

./q1-soln --cmdline $disk22 > /tmp/q2-soln2 <<EOF
blksiz 1024
mkdir test123
cd test123
put /tmp/shortfile testfile1.txt
show testfile1.txt
rm testfile1.txt
ls
cd ..
ls
rmdir test123
ls
mkdir nest1
cd nest1
mkdir nest2
cd nest2
mkdir tmp
put /tmp/longfile
show tmp/longfile
ls
rm tmp/longfile
ls
cd ..
cd ..
rm nest1
ls
EOF

sed -i -e '$a\' /tmp/q2-soln2


./q1-soln --cmdline $disk23 > /tmp/q2-soln3 <<EOF
blksiz 4000
mkdir test123
cd test123
put /tmp/shortfile testfile1.txt
show testfile1.txt
rm testfile1.txt
ls
cd ..
ls
rmdir test123
ls
mkdir nest1
cd nest1
mkdir nest2
cd nest2
mkdir tmp
put /tmp/longfile
show tmp/longfile
ls
rm tmp/longfile
ls
cd ..
cd ..
rm nest1
ls
EOF

sed -i -e '$a\' /tmp/q2-soln3

diff /tmp/q2out /tmp/q2-soln

if [ $? != 0 ]; then
    echo 'Q2 Read-Write test failed - Please check diff result above'
else
    echo 'Q2 Read-Write test passed'
fi



diff /tmp/q2out1 /tmp/q2-soln1

if [ $? != 0 ]; then
    echo 'Q2 Read-Write test failed - Please check diff result above-1'
else
    echo 'Q2 Read-Write test passed-1'
fi



diff /tmp/q2out2 /tmp/q2-soln2

if [ $? != 0 ]; then
    echo 'Q2 Read-Write test failed - Please check diff result above-2'
else
    echo 'Q2 Read-Write test passed-2'
fi



diff /tmp/q2out3 /tmp/q2-soln3

if [ $? != 0 ]; then
    echo 'Q2 Read-Write test failed - Please check diff result above-3'
else
    echo 'Q2 Read-Write test passed-3'
fi


#rm -f /tmp/q2out
#rm -f /tmp/q2-soln
rm -f disk1.copy.img
rm -f disk2.copy.img
rm -f disk11.copy.img
rm -f disk21.copy.img
rm -f disk12.copy.img
rm -f disk22.copy.img
rm -f disk13.copy.img
rm -f disk23.copy.img
#rm -f /test/shortfile
#rm -f /tmp/longfile
