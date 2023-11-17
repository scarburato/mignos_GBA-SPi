/***************************************************************************//**
*  \file       driver.c
*
*  \details    Simple I2C driver explanation (SSD_1306 OLED Display Interface)
*
*  \author     EmbeTronicX
*
*  \Tested with Linux raspberrypi 5.4.51-v7l+
*
* *******************************************************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kernel.h>

#define I2C_BUS_AVAILABLE   (1)              // I2C Bus available in our Raspberry Pi
#define SLAVE_DEVICE_NAME   ("power-GBAPi")  // Device and Driver Name
#define power_GBAPi_SLAVE_ADDR  (0x8)            // ATMEGA Slave Address

static struct i2c_adapter *etx_i2c_adapter     = NULL;  // I2C Adapter Structure
static struct i2c_client  *etx_i2c_client_power_GBAPi = NULL;  // I2C Cient Structure (In our case it is OLED)

/*
** This function writes the data into the I2C client
**
**  Arguments:
**      buff -> buffer to be sent
**      len  -> Length of the data
**   
*/
static int I2C_Write(unsigned char *buf, unsigned int len)
{
    /*
    ** Sending Start condition, Slave address with R/W bit, 
    ** ACK/NACK and Stop condtions will be handled internally.
    */ 
    int ret = i2c_master_send(etx_i2c_client_power_GBAPi, buf, len);
    
    return ret;
}

/*
** This function reads one byte of the data from the I2C client
**
**  Arguments:
**      out_buff -> buffer wherer the data to be copied
**      len      -> Length of the data to be read
** 
*/
static int I2C_Read(unsigned char *out_buf, unsigned int len)
{
    /*
    ** Sending Start condition, Slave address with R/W bit, 
    ** ACK/NACK and Stop condtions will be handled internally.
    */ 
    int ret = i2c_master_recv(etx_i2c_client_power_GBAPi, out_buf, len);
    
    return ret;
}

/*
** This function is specific to the SSD_1306 OLED.
** This function sends the command/data to the OLED.
**
**  Arguments:
**      is_cmd -> true = command, flase = data
**      data   -> data to be written
** 
*/
static void power_GBAPi_Write(bool led)
{
    unsigned char buf[1] = {led};
    int ret;
    
    ret = I2C_Write(buf, 1);
}


/*
** This function getting called when the slave has been found
** Note : This will be called only once when we load the driver.
*/
static int etx_power_GBAPi_probe(struct i2c_client *client,
                         const struct i2c_device_id *id)
{
    pr_info("ATMEGA Probed!!!\n");
    return 0;
}

/*
** This function getting called when the slave has been removed
** Note : This will be called only once when we unload the driver.
*/
static void etx_power_GBAPi_remove(struct i2c_client *client)
{   
    pr_info("ATMEGA Removed!!!\n");
}

/*
** Structure that has slave device id
*/
static const struct i2c_device_id etx_power_GBAPi_id[] = {
        { SLAVE_DEVICE_NAME, 0 },
        { }
};
MODULE_DEVICE_TABLE(i2c, etx_power_GBAPi_id);

/*
** I2C driver Structure that has to be added to linux
*/
static struct i2c_driver etx_power_GBAPi_driver = {
        .driver = {
            .name   = SLAVE_DEVICE_NAME,
            .owner  = THIS_MODULE,
        },
        .probe          = etx_power_GBAPi_probe,
        .remove         = etx_power_GBAPi_remove,
        .id_table       = etx_power_GBAPi_id
};

/*
** I2C Board Info strucutre
*/
static struct i2c_board_info power_GBAPi_i2c_board_info = {
        I2C_BOARD_INFO(SLAVE_DEVICE_NAME, power_GBAPi_SLAVE_ADDR)
    };

static void power_GBAPi_poweroff_do_poweroff(void)
{
    power_GBAPi_Write(false);
    mdelay(100);
}

/*
** Module Init function
*/
static int __init etx_driver_init(void)
{
    int ret = -1;
    etx_i2c_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);
    
    if( etx_i2c_adapter == NULL )
    {
        printk(KERN_ERR "No i2c bus\n");
        return -ENODEV;
    }
    
    etx_i2c_client_power_GBAPi = i2c_new_client_device(etx_i2c_adapter, &power_GBAPi_i2c_board_info);
    
    if( etx_i2c_client_power_GBAPi != NULL )
    {
        i2c_add_driver(&etx_power_GBAPi_driver);
        ret = 0;
    }else{
        printk(KERN_ERR "Unable to init device\n");
        return -ENODEV;
    }
    
    i2c_put_adapter(etx_i2c_adapter);
    
    pr_info("Driver Added!!!\n");
    // if (pm_power_off != NULL) {
    //     printk(KERN_ERR "Porca miseria non sono solo\n");
	// 	return -EBUSY;
	// }

    pm_power_off = &power_GBAPi_poweroff_do_poweroff;
    power_GBAPi_Write(true);
    return ret;
}

/*
** Module Exit function
*/
static void __exit etx_driver_exit(void)
{
    i2c_unregister_device(etx_i2c_client_power_GBAPi);
    i2c_del_driver(&etx_power_GBAPi_driver);
    if (pm_power_off == &power_GBAPi_poweroff_do_poweroff)
		pm_power_off = NULL;

    pr_info("Driver Removed!!!\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("GBASPi <GBASPi@nintendo.fuck>");
MODULE_DESCRIPTION("GBASPi power handler");
MODULE_VERSION("0.1");