#ifndef _TEST_UART_H
#define _TEST_UART_H

extern void clean_uart_buff(void);

extern int uart_put(char *send_buf, int data_len);
extern int uart_get(char *rcv_buf, int data_len);
extern int uart_receive(int fd, char *rcv_buf, int data_len, int timeout);


extern void intercom_uart_init(void);
extern void swiping_card_uart_init(void);

#endif
