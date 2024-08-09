#ifndef mares_gpio_defs_h_
#define mares_gpio_defs_h_

/* The following can be checked by `drvmgr info` in the shell */

/* RTEMS /dev/console */
#define RKI_APBUART_0     0x80100000

/* RTEMS /dev/console_b */
#define RKI_APBUART_1     0x80100100

/* RTEMS /dev/console_c */
#define RKI_APBUART_2     0x80100200

/* RTEMS /dev/console_d */
#define RKI_APBUART_3     0x80100300

/* RTEMS /dev/console_e*/
#define RKI_APBUART_4     0x80100400

/* RTEMS /dev/console_f*/
#define RKI_APBUART_5     0x80100500

/* APB UART */
struct rki_apbuart_regs
{
  volatile unsigned int data;
  volatile unsigned int status;
  volatile unsigned int ctrl;
  volatile unsigned int scaler;
};

#endif
