#!/bin/sh

error_count=0
projpath=$PWD

mount_dir=$PWD/mnt
rm -f disk1.fuse.img
rm -rf $mount_dir
mkdir mnt

cp disk1.img disk1.fuse.img
disk=$PWD/disk1.fuse.img

umask 022
echo "Mounting Drive"
./homework $disk $mount_dir
echo "Mount successful\nstarting Q3 Test\n\n"

#### TEST1
echo "\t>testing ls"
cd $mount_dir
rm -rf /tmp/tmp
ls > /tmp/tmp
rm -rf /tmp/orig
echo "another-file\ndir_other\nfile.txt\nhome\nwork" > /tmp/orig

cd $projpath
diff /tmp/tmp /tmp/orig
if [ $? != 0 ]; then
error_count=$((error_count + 1));
fi


#### TEST2
echo "\t>testing mkdir & rmdir"
mkdir $mount_dir/ameya
rm -f /tmp/tmp
ls $mount_dir > /tmp/tmp
rm -f /tmp/orig
touch /tmp/orig
echo "ameya\nanother-file\ndir_other\nfile.txt\nhome\nwork" > /tmp/orig
diff /tmp/tmp /tmp/orig
if [ $? != 0 ]; then
error_count=$((error_count + 1));
fi


rmdir $mount_dir/ameya
if [ $? != 0 ]; then
error_count=$((error_count + 1));
fi




#### TEST3
echo "\t>testing nested mkdir"
mkdir $mount_dir/ameya
mkdir $mount_dir/ameya/animesh
mkdir $mount_dir/ameya/animesh/peter
cd $mount_dir/ameya/animesh/peter
rm -f /tmp/tmp
pwd > /tmp/tmp
rm -f /tmp/orig
touch /tmp/orig
echo "$projpath/mnt/ameya/animesh/peter" > /tmp/orig
diff /tmp/tmp /tmp/orig
if [ $? != 0 ]; then
error_count=$((error_count + 1));
fi



#### TEST4
echo "\t>testing show/cat"
cd $mount_dir
rm -f /tmp/orig
cat file.txt > /tmp/orig
rm -f /tmp/tmp
echo "file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt
file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt
file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt
file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt
file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt
file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt
file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt
file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt
file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt
file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt
file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt
file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt
file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt
file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt
file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt
file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt
file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt
file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt
file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt
file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt
file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt
file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt
file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt
file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt
file.txt file.txt file.txt file.txt file.txt file.txt file.txt file.txt">/tmp/tmp
cd $projpath
diff /tmp/tmp /tmp/orig
if [ $? != 0 ]; then
error_count=$((error_count + 1));
fi


#### TEST5
echo "\t>testing chmod <777>, see report below\n"
rm -f /tmp/orig
ls -al > /tmp/orig
rm -f cstest.txt
echo "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.">cstest.txt

cd $mount_dir
cp -f "$projpath/cstest.txt" .

chmod 744 cstest.txt
ls -al "cstest.txt" | awk {'print $1'} > /tmp/perm1
chmod 777 cstest.txt 
ls -al "cstest.txt" | awk {'print $1'} > /tmp/perm2



diff /tmp/perm1 /tmp/perm2
if [ $? != 0 ]; then
echo "permissions are changed successfully\n\n"
else
echo "permissions not changed, operation failed\n\n"
error_count=$((error_count + 1));
fi

rm cstest.txt


#### TEST6
echo "\t>testing timestamp of file.txt\n"
time_start=`ls -al "file.txt" | awk {'print $6" "$7" "$8'}`
mv file.txt new_file.txt
time_end=`ls -al "new_file.txt" | awk {'print $6" "$7" "$8'}`

echo "Original timestamp of file.txt: $time_start"
echo "Modified timestamp of file.txt: $time_end"

cd $projpath


#### TEST7
echo "\n\t>testing truncate on tfile.txt <500 bytes>"

rm -f $projpath/tfile.txt
echo "Quis lacus fusce. Sit id commodo tristique arcu tempor proin. Posuere duis Netus cras placerat est eros consequat. Litora curabitur nascetur dignissim nibh. Curae; vulputate urna vivamus ultricies, non. Eros. Quisque. Convallis est tristique porta vitae. Est eu nisi viverra hendrerit amet sociis sociis nunc vulputate lobortis aliquam tincidunt est fusce habitasse arcu facilisi hymenaeos hac congue fames parturient class dolor feugiat felis senectus metus integer tristique odio hac aptent lorem " > $projpath/tfile.txt 

cd $mount_dir
cp $projpath/tfile.txt .
init_wc=`wc -c tfile.txt`
truncate --size 0 tfile.txt
final_wc=`wc -c tfile.txt | awk {'print $1'}`


if [ $final_wc != 0 ]; then
error_count=$((error_count + 1));
fi

#echo "Initial word count: $init_wc\nFinal word count: $final_wc\n\n"



if [ $error_count != 0 ]; then
    echo "ERRORS FOUND!\n\n Please check diff results above\n"
    echo "ERROR COUNT : $error_count\n";
else
    echo "\n !!! End of Test !!!\n"
    echo "\n!!! All tests passed in FUSE mode !!!\n"
fi

cd $projpath
echo "Unmounting the drive"
rm -rf tfile.txt
rm -rf cstest.txt
rm -rf disk1.fuse.img
sleep 6s
fusermount -u $mount_dir
rm -rf mnt

