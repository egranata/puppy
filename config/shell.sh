# This is the script that runs for an init shell (i.e. a shell run with --init)
# Such a shell is spawned by init as the last step of boot

env PATH=/system/apps:/initrd
cowspeak /system/config/motd
