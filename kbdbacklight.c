#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>

#define EVENT       "/dev/input/event3"
#define KBFD        "/sys/devices/platform/thinkpad_acpi/leds/tpacpi::kbd_backlight/brightness"

#ifndef IDLE_MSEC
#define IDLE_MSEC   5000
#endif

#ifndef BRGHT_OFF
#define BRGHT_OFF	0x30
#endif

#ifndef BRGHT_MED
#define BRGHT_MED   0x31
#endif

#ifndef BRGHT_MAX
#define BRGHT_MAX   0x32
#endif

static char dummybuf[8192];
volatile sig_atomic_t running = 1;

void term(int signum)
{
    running = 0;
}

int main(int argc, char *argv[])
{
    struct sigaction action;
    struct pollfd pfd;
    int rc, blfd;
    int timeout, prev = -1;

    printf("kbd_backlight started\n");

    char bm[1];
    bm[0] = BRGHT_MAX;

    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = term;
    sigaction(SIGTERM, &action, NULL);

    blfd = open(KBFD, O_WRONLY);

    // needs the event bound to the keyboard
    // for Xorg ie find using
    //  cat /var/log/Xorg.0.log | grep "keyboard.*event"
    pfd.fd = open(EVENT, O_RDONLY);
    pfd.events = POLLIN;
    timeout = IDLE_MSEC;

    while (running) {
        rc = poll(&pfd, 1, timeout);

        if (rc) {
            /* got keyboard input, flush it all and
             * wait for the next event.
             */
            read(pfd.fd, dummybuf, sizeof(dummybuf));
            timeout = IDLE_MSEC;
            bm[0] = BRGHT_MAX;
        } else {
            /* once we've gotten a timeout, turn off
             * kbd backlight and wait forever for
             * the next keypress
             */
            timeout = -1;
            bm[0] = BRGHT_OFF;
        }

        if (bm[0] == prev) continue;
        lseek(blfd, 1, SEEK_SET);
        write(blfd, bm, 1);

        prev = bm[0];
    }

    // clean up after sigterm
    bm[0] = BRGHT_OFF;

    lseek(blfd, 1, SEEK_SET);
    write(blfd, bm, 1);
}
