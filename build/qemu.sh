#!/bin/sh

qemu-system-i386 -drive format=raw,media=disk,file=out/os.img -vga std \
-serial file:out/kernel.log -m 768 -d guest_errors -rtc base=utc \
-monitor stdio \
-smbios type=0,vendor="Puppy" -smbios type=1,manufacturer="Puppy",product="Puppy System",serial="P0PP1" \
-k en-us
