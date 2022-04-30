#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>

#include "mmb4l.h"
#include "cstring.h"
#include "error.h"
#include "file.h"
#include "interrupt.h"
#include "serial.h"
#include "utility.h"

#define	COM_DEFAULT_SPEED            B9600
#define	COM_DEFAULT_BUF_SIZE         4 * 1024
#define	COM_DEFAULT_INTERRUPT_COUNT  1

#define ERROR_COM_SPECIFICATION        error_throw_ex(kError, "COM specification")
#define ERROR_UNSUPPORTED_BAUDRATE(i)  error_throw_ex(kError, "Unsupported baudrate: %", i)

typedef enum { PARITY_NONE, PARITY_EVEN, PARITY_ODD } Parity;

typedef struct {
    char device[STRINGSIZE];
    speed_t speed;
    int bufsize;
    bool b7;
    Parity parity;
    bool rtscts;
    bool s2;
    bool xonxoff;
    char *rx_interrupt_addr;
    int64_t rx_interrupt_count;
} ComSpec;

static speed_t serial_int_to_speed(int64_t i) {
    switch (i) {
        case      50: return      B50;
        case      75: return      B75;
        case     110: return     B110;
        case     134: return     B134;
        case     150: return     B150;
        case     200: return     B200;
        case     300: return     B300;
        case     600: return     B600;
        case    1200: return    B1200;
        case    1800: return    B1800;
        case    2400: return    B2400;
        case    4800: return    B4800;
        case    9600: return    B9600;
        case   19200: return   B19200;
        case   38400: return   B38400;
        case   57600: return   B57600;
        case  115200: return  B115200;
        case  230400: return  B230400;
        case  460800: return  B460800;
        case  500000: return  B500000;
        case  576000: return  B576000;
        case  921600: return  B921600;
        case 1000000: return B1000000;
        case 1152000: return B1152000;
        case 1500000: return B1500000;
        case 2000000: return B2000000;
        case 2500000: return B2500000;
        case 3000000: return B3000000;
        case 3500000: return B3500000;
        case 4000000: return B4000000;
    }

    return 0;
}

static int32_t serial_speed_to_int(speed_t s) {
    switch (s) {
        case      B50: return      50;
        case      B75: return      75;
        case     B110: return     110;
        case     B134: return     134;
        case     B150: return     150;
        case     B200: return     200;
        case     B300: return     300;
        case     B600: return     600;
        case    B1200: return    1200;
        case    B1800: return    1800;
        case    B2400: return    2400;
        case    B4800: return    4800;
        case    B9600: return    9600;
        case   B19200: return   19200;
        case   B38400: return   38400;
        case   B57600: return   57600;
        case  B115200: return  115200;
        case  B230400: return  230400;
        case  B460800: return  460800;
        case  B500000: return  500000;
        case  B576000: return  576000;
        case  B921600: return  921600;
        case B1000000: return 1000000;
        case B1152000: return 1152000;
        case B1500000: return 1500000;
        case B2000000: return 2000000;
        case B2500000: return 2500000;
        case B3000000: return 3000000;
        case B3500000: return 3500000;
        case B4000000: return 4000000;
    }

    return 0;
}

static void serial_dump_spec(ComSpec *comspec) {
    printf("Device:             %s\n", comspec->device);
    printf("Speed:              %d\n", serial_speed_to_int(comspec->speed));
    printf("Bufsize:            %d\n", comspec->bufsize);
#if defined(ENV64BIT)
    printf("RX interrupt:       0x%8lx\n", (uintptr_t) comspec->rx_interrupt_addr);
    printf("RX interrupt count: %ld\n", comspec->rx_interrupt_count);
#else
    printf("RX interrupt:       0x%8ix\n", (uintptr_t) comspec->rx_interrupt_addr);
    printf("RX interrupt count: %lld\n", comspec->rx_interrupt_count);
#endif
    printf("B7:                 %s\n", comspec->b7 ? "true" : "false");
    printf("Parity:             %d\n", comspec->parity);
    printf("RTS/CTS:            %s\n", comspec->rtscts ? "true" : "false");
    printf("S2:                 %s\n", comspec->s2 ? "true" : "false");
    printf("XON/XOFF:           %s\n", comspec->xonxoff ? "true" : "false");
}

void serial_parse_comspec(const char* comspec_str, ComSpec *comspec) {
    getargs((char **) &comspec_str, 21, ":,");
    if (argc != 2 && (argc & 0x01) == 0) ERROR_COM_SPECIFICATION;

    memset(comspec, 0, sizeof(ComSpec));
    comspec->speed = COM_DEFAULT_SPEED;
    comspec->bufsize = COM_DEFAULT_BUF_SIZE;
    comspec->rx_interrupt_count = COM_DEFAULT_INTERRUPT_COUNT;
    strcpy(comspec->device, argv[0]);

    for (int i = 0; i < 6; i++) {
        if (strcasecmp(argv[argc - 1], "OC") == 0) { // Open collector option.
            ERROR_UNSUPPORTED_FLAG("OC");
        }

        else if (strcasecmp(argv[argc - 1], "DEP") == 0) { // Data enable option.
            ERROR_UNSUPPORTED_FLAG("DEP");
        }

        else if (strcasecmp(argv[argc - 1], "DEN") == 0) { // Data enable option.
            ERROR_UNSUPPORTED_FLAG("DEN");
        }

        else if (strcasecmp(argv[argc - 1], "EVEN") == 0) { // Even parity.
            if (comspec->parity != PARITY_NONE) ERROR_SYNTAX;
            comspec->parity = PARITY_EVEN;
            argc -= 2;
        }

        else if (strcasecmp(argv[argc - 1], "ODD") == 0) { // Odd parity.
            if (comspec->parity != PARITY_NONE) ERROR_SYNTAX;
            comspec->parity = PARITY_EVEN;
            argc -= 2;
        }

        else if (strcasecmp(argv[argc - 1], "S2") == 0) { // Two stop bit option.
            comspec->s2 = true;
            argc -= 2;
        }

        else if (strcasecmp(argv[argc - 1], "7BIT") == 0) { // 7 bit byte option.
            comspec->b7 = true;
            argc -= 2;
        }

        else if (strcasecmp(argv[argc - 1], "INV") == 0) { // Invert option.
            ERROR_UNSUPPORTED_FLAG("INV");
        }

        else if (strcasecmp(argv[argc - 1], "RTSCTS") == 0) { // Hardware flow control RTS/CTS option.
            comspec->rtscts = true;
            argc -= 2;
        }

        else if (strcasecmp(argv[argc - 1], "XONXOFF") == 0) { // Software flow control option.
            comspec->xonxoff = true;
            argc -= 2;
        }
    }

    if (argc < 1 || argc > 11) ERROR_COM_SPECIFICATION;

    // Speed/baud as a number.
    if (argc >= 3 && *argv[2]) {
        int64_t i = getinteger(argv[2]);
        comspec->speed = serial_int_to_speed(i);
        if (!comspec->speed) ERROR_UNSUPPORTED_BAUDRATE(i);
    }

    // Buffer size as a number.
    if (argc >= 5 && *argv[4]) {
        comspec->bufsize = getinteger(argv[4]);
    }

    // Received data interrupt location.
    if (argc >= 7) {
        // InterruptUsed = true;
        argv[6] = cstring_toupper(argv[6]); // TODO: is this needed ?
        comspec->rx_interrupt_addr = GetIntAddress(argv[6]);
    }

    // Buffer count for interrupt as a number.
    if (argc >= 9) {
        comspec->rx_interrupt_count = getinteger(argv[8]);
        if (comspec->rx_interrupt_count < 1 || comspec->rx_interrupt_count > comspec->bufsize) {
            ERROR_COM_SPECIFICATION;
        }
    }
}

void serial_open(const char *comspec_str, int fnbr) {
    if (fnbr < 1 || fnbr > MAXOPENFILES) ERROR_INVALID_FILE_NUMBER;
    FileEntry *entry = &(file_table[fnbr]);
    if (entry->type != fet_closed) ERROR_ALREADY_OPEN;

    ComSpec comspec = { 0 };
    serial_parse_comspec(comspec_str, &comspec);
    // serial_dump_spec(&comspec);

    errno = 0;
    int fd = open(comspec.device, O_RDWR | O_NOCTTY); //  | O_NDELAY);
    if (fd == -1) error_throw(errno);

    if (fcntl(fd, F_SETFL, 0) == -1) error_throw(errno);

    struct termios options;
    if (FAILED(tcgetattr(fd, &options))) error_throw(errno);
    cfmakeraw(&options);
    cfsetispeed(&options, comspec.speed);
    cfsetospeed(&options, comspec.speed);

    // Ignore modem lines and enable receiver.
    options.c_cflag |= (CLOCAL | CREAD);

    // Character size.
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= (comspec.b7 ? CS7 : CS8);

    // Parity.
    switch (comspec.parity) {
        case PARITY_NONE:
            options.c_cflag &= ~PARENB;
            break;
        case PARITY_EVEN:
            options.c_cflag |= PARENB;
            options.c_cflag &= ~PARODD;
            break;
        case PARITY_ODD:
            options.c_cflag |= PARENB;
            options.c_cflag |= PARODD;
            break;
        default:
            ERROR_INTERNAL_FAULT;
            break;
    }

    // No parity checking of input (for the moment).
    options.c_iflag &= ~INPCK;  // Disable parity checking.
    options.c_iflag &= ~IGNPAR; // Don't ignore parity errors - irrelevant since no checking.
    options.c_iflag &= ~PARMRK; // Don't 'mark' parity errors.
    options.c_iflag &= ~ISTRIP; // Don't strip parity bits.

    // Hardware flow control.
    if (comspec.rtscts) {
        options.c_cflag |= CRTSCTS;
    } else {
        options.c_cflag &= ~CRTSCTS;
    }

    // Stop bits.
    if (comspec.s2) {
        options.c_cflag |= CSTOPB;
    } else {
        options.c_cflag &= ~CSTOPB;
    }

    // Software flow control.
    if (comspec.xonxoff) {
        options.c_iflag |= (IXON | IXOFF | IXANY);
    } else {
        options.c_iflag &= ~(IXON | IXOFF | IXANY);
    }

    options.c_cc[VMIN]  = 0; // minimum number of characters to read.
    options.c_cc[VTIME] = 0; // time to wait for a character, 10ths of a second.

    // Apply changes after all output transmitted and discard all input.
    if (FAILED(tcsetattr(fd, TCSAFLUSH, &options))) error_throw(errno);

    // May be necessary, the jury is still out.
    // mmtime_sleep_ns(MILLISECONDS_TO_NANOSECONDS(1000));
    // if (FAILED(tcflush(fd, TCIOFLUSH))) error_throw(errno);

    entry->type = fet_serial;
    entry->serial_fd = fd;
    if (comspec.rx_interrupt_addr) {
        interrupt_enable_serial_rx(fnbr, comspec.rx_interrupt_count, comspec.rx_interrupt_addr);
    }

    char *data = GetMemory(comspec.bufsize); // Should already be zeroed.
    rx_buf_init(&(entry->rx_buf), data, comspec.bufsize);
}

void serial_close(int fnbr) {
    FileEntry *entry = &(file_table[fnbr]);
    assert(entry->type == fet_serial);
    close(entry->serial_fd);
    entry->type = fet_closed;
    entry->serial_fd = 0;
    FreeMemory(entry->rx_buf.data);
    interrupt_disable_serial_rx(fnbr);
}

void serial_pump_input(int fnbr) {
    assert(file_table[fnbr].type == fet_serial);
    
    char tmp[256];
    errno = 0;
    ssize_t count = read(file_table[fnbr].serial_fd, tmp, 256);
    if (count == -1) error_throw(errno);
    
    if (count > 0) {
        for (ssize_t i = 0; i < count; ++i) {
            rx_buf_put(&file_table[fnbr].rx_buf, tmp[i]);
        }
    }
}

int serial_eof(int fnbr) {
    if (rx_buf_size(&file_table[fnbr].rx_buf) > 0) return 0;
    serial_pump_input(fnbr);
    return (rx_buf_size(&file_table[fnbr].rx_buf) > 0) ? 0 : 1;

    // Alternative:
    // errno = 0;
    // int count;
    // if (ioctl(file_table[fnbr].serial_fd, FIONREAD, &count) == -1) error_throw(errno);
    // return count ? 0 : 1;
}

int serial_getc(int fnbr) {
    int ch = rx_buf_get(&file_table[fnbr].rx_buf);
    if (ch == -1) {
        serial_pump_input(fnbr);
        ch = rx_buf_get(&file_table[fnbr].rx_buf);
    }
    return ch;
}

int serial_putc(int ch, int fnbr) {
    assert(file_table[fnbr].type == fet_serial);
    errno = 0;
    ssize_t count = write(file_table[fnbr].serial_fd, &ch, 1);
    switch (count) {
        case -1:
            error_throw(errno);
            break;
        case 1:
            return 1;
            break;
        default:
            error_throw(EBADF);
            break;
    }

    assert(false);
    return -1;
}

int serial_rx_queue_size(int fnbr) {
    assert(file_table[fnbr].type == fet_serial);
    return rx_buf_size(&file_table[fnbr].rx_buf);
}
