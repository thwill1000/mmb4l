#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>

#include "error.h"
#include "file.h"
#include "serial.h"
#include "utility.h"
#include "version.h"

#define	COM_DEFAULT_SPEED      B9600
#define	COM_DEFAULT_BUF_SIZE   1024
#define ERROR_COM_SPECIFICATION  error("COM specification")

typedef struct {
    char device[STRINGSIZE];
    speed_t speed;
    int bufsize;
    bool b7;
    int de;
    int ilevel;
    bool inv;
    bool oc;
    int parity;
    bool s2;
    char *rx_interrupt;
    char *tx_interrupt;
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

static void serial_parse_comspec(const char* comspec_str, ComSpec *comspec) {
    // int baud, i, inv, oc, s2, de, parity, b7, bufsize, ilevel;
    // char *interrupt, *TXinterrupt;
    //GPIO_InitTypeDef GPIO_InitStruct;

    getargs((char **) &comspec_str, 21, ":,");
    if (argc != 2 && (argc & 0x01) == 0) ERROR_COM_SPECIFICATION;

    memset(comspec, 0, sizeof(ComSpec));
    comspec->speed = COM_DEFAULT_SPEED;
    comspec->bufsize = COM_DEFAULT_BUF_SIZE;
    comspec->ilevel = 1;

    strcpy(comspec->device, argv[0]);

    for (int i = 0; i < 6; i++) {
        if (str_equal(argv[argc - 1], "OC")) { // Open collector option.
            comspec->oc = true;
            argc -= 2;
        }

        if (str_equal(argv[argc - 1], "DEP")) { // Data enable option.
            if (comspec->de) ERROR_SYNTAX;
            comspec->de = 1;
            argc -= 2;
        }

        if (str_equal(argv[argc - 1], "DEN")) { // Data enable option.
            if (comspec->de) ERROR_SYNTAX;
            comspec->de = 2;
            argc -= 2;
        }

        if (str_equal(argv[argc - 1], "EVEN")) { // Even parity.
            if (comspec->parity) ERROR_SYNTAX;
            comspec->parity = 1;
            argc -= 2;
        }

        if (str_equal(argv[argc - 1], "ODD")) { // Odd parity.
            if (comspec->parity) ERROR_SYNTAX;
            comspec->parity = 2;
            argc -= 2;
        }

        if (str_equal(argv[argc - 1], "S2")) { // Two stop bit option.
            comspec->s2 = true;
            argc -= 2;
        }

        if (str_equal(argv[argc - 1], "7BIT")) { // 7 bit byte option.
            comspec->b7 = true;
            argc -= 2;
        }

        if (str_equal(argv[argc - 1], "INV")) { // Invert option.
            comspec->inv = true;
            argc -= 2;
        }
    }

    if (argc < 1 || argc > 13) ERROR_COM_SPECIFICATION;

    // Speed/baud as a number.
    if (argc >= 3 && *argv[2]) {
        int64_t i = getinteger(argv[2]);
        comspec->speed = serial_int_to_speed(i);
        if (!comspec->speed) error("unsupported serial speed '%'", i);
    }

    // Buffer size as a number.
    if (argc >= 5 && *argv[4]) {
        comspec->bufsize = getinteger(argv[4]);
    }

    // Received data interrupt location.
    if (argc >= 7) {
        // InterruptUsed = true;
        argv[6] = strupr(argv[6]);
        comspec->rx_interrupt = GetIntAddress(argv[6]);
    }

    // Buffer level for interrupt as a number.
    if (argc >= 9) {
        comspec->ilevel = getinteger(argv[8]);
        if (comspec->ilevel < 1 || comspec->ilevel > comspec->bufsize) {
            ERROR_COM_SPECIFICATION;
        }
    }

    // Transmitted data interrupt location ???
    if (argc >= 11) {
        // InterruptUsed = true;
        argv[6] = strupr(argv[10]);
        comspec->tx_interrupt = GetIntAddress(argv[10]);
    }
}

static void serial_dump_spec(ComSpec *comspec) {
    printf("Device:       %s\n", comspec->device);
    printf("Speed:        %d\n", serial_speed_to_int(comspec->speed));
    printf("Bufsize:      %d\n", comspec->bufsize);
    printf("B7:           %s\n", comspec->b7 ? "true" : "false");
    printf("DE:           %d\n", comspec->de);
    printf("ILevel:       %d\n", comspec->ilevel);
    printf("INV:          %s\n", comspec->inv ? "true" : "false");
    printf("OC:           %s\n", comspec->oc ? "true" : "false");
    printf("Parity:       %d\n", comspec->parity);
    printf("S2:           %s\n", comspec->s2 ? "true" : "false");
    printf("RX interrupt: 0x%lx\n", (uintptr_t) comspec->rx_interrupt);
    printf("TX interrupt: 0x%lx\n", (uintptr_t) comspec->tx_interrupt);
}

void serial_open(const char *comspec_str, int fnbr) {
    if (fnbr < 1 || fnbr > MAXOPENFILES) ERROR_INVALID_FILE_NUMBER;
    if (file_table[fnbr].type != fet_closed) ERROR_ALREADY_OPEN;

    ComSpec comspec = { 0 };
    serial_parse_comspec(comspec_str, &comspec);
    //serial_dump_spec(&comspec);

    errno = 0;

    int fd = open(comspec.device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) error("could not open serial device '$'", comspec.device);
    fcntl(fd, F_SETFL, 0);
    struct termios options;
    tcgetattr(fd, &options);
    cfmakeraw(&options);
    cfsetispeed(&options, comspec.speed);
    cfsetospeed(&options, comspec.speed);
    options.c_cflag |= (CLOCAL | CREAD) ;
    options.c_cflag &= ~PARENB ;
    options.c_cflag &= ~CSTOPB ;
    options.c_cflag &= ~CSIZE ;
    options.c_cflag |= CS8 ;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG) ;
    options.c_oflag &= ~OPOST ;

    options.c_cc[VMIN]  = 0;
    options.c_cc[VTIME] = 1; //deci-seconds ?
    tcsetattr(fd, TCSANOW, &options);

    error_check();

    file_table[fnbr].type = fet_serial;
    file_table[fnbr].serial_fd = fd;
}

void serial_close(int fnbr) {
    assert(file_table[fnbr].type == fet_serial);
    close(file_table[fnbr].serial_fd);
    file_table[fnbr].type = fet_closed;
    file_table[fnbr].serial_fd = 0;
}

int serial_eof(int fnbr) {
    assert(file_table[fnbr].type == fet_serial);
    int count;
    if (FAILED(ioctl(file_table[fnbr].serial_fd, FIONREAD, &count))) {
        error_check();
    }
    return count ? 0 : 1;
}

int serial_getc(int fnbr) {
    assert(file_table[fnbr].type == fet_serial);
    char ch;
    ssize_t count = read(file_table[fnbr].serial_fd, &ch, 1);
    if (FAILED(count)) error_check();
    return count == 0 ? -1 : (int) ch;
}

int serial_putc(int ch, int fnbr) {
    assert(file_table[fnbr].type == fet_serial);
    errno = 0;
    ssize_t count = write(file_table[fnbr].serial_fd, &ch, 1);
    if (FAILED(count)) error_check();
    if (count != 1) error_system(EBADF);
    return count;
}

int serial_rx_queue_size(int fnbr) {
    assert(file_table[fnbr].type == fet_serial);
    return -1;
}
