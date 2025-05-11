sudo rmmod chords
sudo dmesg -C
make clean
make all
sudo insmod chords.ko
# sudo dmesg | tail -n 4
sudo mknod /dev/chords210 c 240 0
sudo ./chords_test
# sudo dmesg | tail -n 4
sudo dmesg | grep CHORDS