# LKM
kernel module to store message queue


This is a kernel module. The programm keeps a queue of messages in memory, writes and reads messages into the queue by a request from the user space.


To create module:

$make


To load:

$sudo insmod lkm_task

Interaction from user program can be realized through a symbol device file:

$sudo mknod /dev/lkm_task c MAJOR 0

MAJOR - device number got during the module init (printed into kernel log).


To delete and unload:

$sudo rm /dev/lkm_task

$sudo rmmod lkm_task
