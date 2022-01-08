#if !defined(MMB4L_SERIAL_H)
#define MMB4L_SERIAL_H

void serial_open(const char *comspec, int fnbr);
void serial_close(int fnbr);
int serial_eof(int fnbr);
int serial_getc(int fnbr);
void serial_pump_input(int fnbr);
int serial_putc(int ch, int fnbr);
int serial_rx_queue_size(int fnbr);

#endif
