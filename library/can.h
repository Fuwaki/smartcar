#ifndef _can_h_
#define _can_h_

// can??????
typedef void (*can_receive_callback_t)(unsigned int canid, unsigned char *dat,
                                       unsigned char len);
void can_set_receive_callback(can_receive_callback_t callback);
int can_keep_alive();       //????????
void can_init(); // ≥ı ºªØcan
void can_set_filter(unsigned int cr, unsigned int mask);
int can_send_msg(unsigned int canid, unsigned char *dat, unsigned char len);
void can_debug();
#endif