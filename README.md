# masstorage-tester

Massstorage-tester is Linux utility for testing HDD's, SSD's, USB flash drives and TF cards. 
It can test both raw devices and filesystem directories, measure sequential and random read and write speed, verify reported capacity, perform long term data retention tests and total bytes written endurance.

# Compilation

`$ git clone https://github.com/tysch/masstorage-tester.git`
`$ cd masstorage-tester`
`$ make`

Tested on x86-64 Ubuntu 14.04, x86-64 Ubuntu 16.04 and Orange Pi Zero Debian 8

# Warning! 
Raw device tests overwrites all data and partitioning on target device, with no option to recover. Always double check the destination, and don't use the device being used. Filesystem tests have some overhead but do not require root privileges and are much safer, though it is still dangerous to use non-empty directiries. 


# How to
Use one of the following options to determine target device:

`$ sudo fdisk -l`
`$ df -h`
`$ lsblk`

Usage examples:

`$ sudo ./masstoragetester -d /dev/mmcblk0`
Tests mmcblk0 for sequential read and write speed and actual capacity.

`$ ./masstoragetester -d /media/username/flash_drive_name -f`
Tests USB flash without root privileges

`$ sudo ./masstoragetester -d /dev/mmcblk0 -x -u 4K`
Tests mmcblk0 for read and write speed and actual capacity with 4K-sized reads and writes at random non-repetitive positions. All positions are being tested exactly once, so it is not strictly random, though. 

`$ ./masstoragetester -d /media/username/flash_drive_name -x -u 4K -f -c 2`
Sequential + random read-write test for USB flash. First pass sequentially fills space to test, second one overwrites it in different pseudorandom order.

`$ sudo ./masstoragetester -d /dev/mmcblk0 -t -c 1000000 -l ./ -e 0 -k 0`
Total bytes written endurance test: cycle mmcblk0 1000000 times with different random data, log tests results, do not halt on errors and print short summary for data corruption pattern. 
This test can take a long time (from days to years) and it is unsafe and impractical to be done on PC. It is best performed via Raspberry Pi or similar SBC, backed up with power bank for uninterruptable power supply. Use -b option to launch test without being bound to open SSH session.

Silent data corruption test:
Write test pattern:

`$ sudo ./masstoragetester -d /dev/mmcblk0 -w`

perform some torture (long time storage, extreme temperature, magnetic field, radiation etc) and read data back:

`$ sudo ./masstoragetester -d /dev/mmcblk0 -r -t`

Use -i option for switching between different data patterns, for both write and read stage. 
 
