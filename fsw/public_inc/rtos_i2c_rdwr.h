/*
** This header file contains definitions for the i2c_msg and i2c_rdwr_ioctl_data 
** structures to be used on non-linux implementations.
**
*/

/*
** Defines for struct i2c_msg - flags field
*/
#define I2C_M_RD            0x0001 /* read data, from slave to master */
#define I2C_M_TEN           0x0010 /* this is a ten bit chip address */
#define I2C_M_RECV_LEN      0x0400 /* length will be first received byte */
#define I2C_M_NO_RD_ACK     0x0800 /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_IGNORE_NAK    0x1000 /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_REV_DIR_ADDR  0x2000 /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_NOSTART       0x4000 /* if I2C_FUNC_NOSTART */
#define I2C_M_STOP          0x8000 /* if I2C_FUNC_PROTOCOL_MANGLING */


/**
 * @brief Performs a combined read/write transfer.
 *
 * Only one stop condition is signalled.
 *
 * The argument type is a pointer to struct i2c_rdwr_ioctl_data.
 */
#define I2C_RDWR            0x0707

struct i2c_msg {
   uint16_t  addr;    /* slave address */
   uint16_t  flags;   /* see above for flag definitions */
   uint16_t  len;     /* msg length */
   uint8_t  *buf;     /* pointer to msg data */
};

struct i2c_rdwr_ioctl_data {
   struct i2c_msg *msgs;    /* pointers to i2c_msgs */
   uint32_t        nmsgs;   /* number of i2c_msgs */
};
