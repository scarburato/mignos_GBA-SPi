#include <linux/module.h>
#include <linux/reboot.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/pm.h>

#define I2C_BUS_AVAILABLE (1)              // I2C Bus available in our Raspberry Pi
#define SLAVE_DEVICE_NAME ("ATMega") // Device and Driver Name
#define ATMega_GBASPi_SLAVE_ADDR (0x8)       // ATMEGA Slave Address

static struct i2c_adapter *etx_i2c_adapter = NULL;           // I2C Adapter Structure
static struct i2c_client *etx_i2c_client_ATMega_GBASPi = NULL; // I2C Cient Structure (In our case it is OLED)
static struct sys_off_handler *poweroff_handler = NULL;

//  Writes data into the I2C client
//   Arguments:
//       buff -> buffer to be sent
//       len  -> Length of the data
static int I2C_Write(unsigned char *buf, unsigned int len)
{
    return i2c_master_send(etx_i2c_client_ATMega_GBASPi, buf, len);
}

// Reads one byte of data from the I2C client
//  Arguments:
//      out_buff -> buffer wherer the data to be copied
//      len      -> Length of the data to be read
static int I2C_Read(unsigned char *out_buf, unsigned int len)
{
    return i2c_master_recv(etx_i2c_client_ATMega_GBASPi, out_buf, len);
}

static void ATMega_GBASPi_Write(bool led)
{
    unsigned char buff; 
    if (led) { 
        buff = 0x1; 
    } else { 
        buff = 0x0; 
    } 
    I2C_Write(&buff, sizeof(buff));
}

// poweroff handler
static int ATMega_GBASPi_poweroff(struct sys_off_data *data)
{
    // blinking the LED for testing
    ATMega_GBASPi_Write(0);
    mdelay(200);
    return 0;
}

// This function getting called when the slave has been found
// This will be called only once when we load the driver.
static int ATMega_GBASPi_probe(struct i2c_client *client,
                                 const struct i2c_device_id *id)
{
    pr_info("ATMEGA Probed!!!\n");
    return 0;
}

// This function getting called when the slave has been removed
// This will be called only once when we unload the driver.
static void ATMega_GBASPi_remove(struct i2c_client *client)
{

    pr_info("ATMEGA Removed!!!\n");
}


// Structure that has slave device id
static const struct i2c_device_id ATMega_GBASPi_id[] = {
    {SLAVE_DEVICE_NAME, 0},
    { },
};
MODULE_DEVICE_TABLE(i2c, ATMega_GBASPi_id);


// I2C driver Structure that has to be added to linux
static struct i2c_driver etx_ATMega_GBASPi_driver = {
    .driver = {
        .name = SLAVE_DEVICE_NAME,
        .owner = THIS_MODULE,
    },
    .probe = ATMega_GBASPi_probe,
    .remove = ATMega_GBASPi_remove,
    .id_table = ATMega_GBASPi_id
};

// I2C Board Info strucutre
static struct i2c_board_info ATMega_GBASPi_i2c_board_info = {
    I2C_BOARD_INFO(SLAVE_DEVICE_NAME, ATMega_GBASPi_SLAVE_ADDR)
};


// Module Init function
static int __init etx_driver_init(void)
{
    int ret = -1;
    etx_i2c_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);

    if (etx_i2c_adapter == NULL)
    {
        printk(KERN_ERR "No i2c bus\n");
        return -ENODEV;
    }

    etx_i2c_client_ATMega_GBASPi = i2c_new_client_device(etx_i2c_adapter, &ATMega_GBASPi_i2c_board_info);

    if (etx_i2c_client_ATMega_GBASPi != NULL)
    {
        i2c_add_driver(&etx_ATMega_GBASPi_driver);
        ret = 0;
    }
    else
    {
        printk(KERN_ERR "Unable to init device\n");
        return -ENODEV;
    }
    
    i2c_put_adapter(etx_i2c_adapter);

    pr_info("Driver Added!!!\n");

    poweroff_handler = register_sys_off_handler(SYS_OFF_MODE_POWER_OFF_PREPARE,0,ATMega_GBASPi_poweroff,NULL);
    ATMega_GBASPi_Write(true);
    return ret;
}

// Module Exit function
static void __exit etx_driver_exit(void)
{
    i2c_unregister_device(etx_i2c_client_ATMega_GBASPi);
    i2c_del_driver(&etx_ATMega_GBASPi_driver);

    unregister_sys_off_handler(poweroff_handler);
    
    ATMega_GBASPi_Write(false);
    pr_info("Driver Removed!!!\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);

// module_i2c_driver(etx_ATMega_GBASPi_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("GBASPi <GBASPi@free.me>");
MODULE_DESCRIPTION("GBASPi power handler");
MODULE_VERSION("0.1");
