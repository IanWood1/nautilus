qemu-system-x86_64 -enable-kvm -cpu Nehalem  -m 4G -vga virtio -display default -machine type=q35 -smp 2 -drive file=disk.qcow2,if=virtio -device virtio-net,netdev=mynet0 -netdev user,id=mynet0,net=192.168.76.0/24,dhcpstart=192.168.76.9


