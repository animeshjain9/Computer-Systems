#!/bin/sh

# any scripting support you need for running the test. (e.g. creating
# image files, running the actual test executable)

# unique disk names
for i in 1 2 3; do
    disks="$disks /tmp/$USER-disk$i.img"
done

stripesize=32
for d in $disks; do
	count_total=`expr 128 \\* $stripesize \\* $i`
	dd if=/dev/zero bs=512 count=$count_total of=$d 2<&-
done

# run the test and clean up


./stripe-test $stripesize $disks

stripesize=7
./stripe-test $stripesize $disks

stripesize=4
./stripe-test $stripesize $disks

stripesize=2
./stripe-test $stripesize $disks

rm -f $disks



# 5 disks total
for i in 1 2 3 4 5; do
    drive="$drive /tmp/$USER-drive$i.img"
done

stripesize=32
for d in $drive; do
	count_total=`expr 128 \\* $stripesize \\* $i`
	dd if=/dev/zero bs=512 count=$count_total of=$d 2<&-
done

# run the test and clean up


./stripe-test $stripesize $drive

stripesize=7
./stripe-test $stripesize $drive

stripesize=4
./stripe-test $stripesize $drive

stripesize=2
./stripe-test $stripesize $drive

rm -f $drive