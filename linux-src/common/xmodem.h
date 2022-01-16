#if !defined(MMB4L_XMODEM_H)
#define MMB4L_XMODEM_H

#define xmodem_send  xmodem_transmit

void xmodem_transmit(int file_fnbr, int serial_fnbr);
void xmodem_receive(int file_fnbr, int serial_fnbr);

#endif
