# kbdbacklight

## Install
make -O2 -o kbdbacklight kbdbacklight.c
cp kbdbacklight /usr/local/bin/kbdbacklight
cp kbdbacklight.service /etc/systemd/system
systemctl daemon-reload

## Autostart
systemctl enable kbdbacklight.service
