sudo rmmod chords
make clean
make all
sudo insmod chords.ko
sudo mknod /dev/chords c 239 0
./chords_test
dmesg | tail -n 4