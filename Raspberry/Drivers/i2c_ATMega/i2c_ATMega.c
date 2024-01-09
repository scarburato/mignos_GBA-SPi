#include <linux/module.h>
#include <linux/reboot.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/input.h>

#define I2C_BUS_AVAILABLE (1)              // I2C Bus available in our Raspberry Pi
#define SLAVE_DEVICE_NAME ("ATMega") // Device and Driver Name
#define ATMega_GBASPi_SLAVE_ADDR (0x8)       // ATMEGA Slave Address

static struct i2c_adapter *etx_i2c_adapter = NULL;           // I2C Adapter Structure
static struct i2c_client *etx_i2c_client_ATMega_GBASPi = NULL; // I2C Cient Structure (In our case it is OLED)
static struct sys_off_handler *poweroff_handler = NULL;
static struct input_dev *gamepad_idev = NULL;
static struct input_dev *pwr_btn_idev = NULL;

//  Writes data into the I2C client
//   Arguments:
//       buff -> buffer to be sent
//       len  -> Length of the data
static int i2c_write(unsigned char *buf, unsigned int len)
{
    return i2c_master_send(etx_i2c_client_ATMega_GBASPi, buf, len);
}

// Reads one byte of data from the I2C client
//  Arguments:
//      out_buff -> buffer wherer the data to be copied
//      len      -> Length of the data to be read
static int i2c_read(unsigned char *out_buf, unsigned int len)
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
    i2c_write(&buff, sizeof(buff));
}

// poweroff handler
static int ATMega_GBASPi_poweroff(struct sys_off_data *data)
{
    // blinking the LED for testing
    ATMega_GBASPi_Write(0);
    mdelay(200);
    return 0;
}

/*
 * Threaded IRQ handler and this can (and will) sleep.
 */
static irqreturn_t atmega_gbaspi_irq_handler(int irq, void *dev_id)
{
    int error;
    uint16_t packet = 0; // this won't work on big endian machines

    error = i2c_read((void*)&packet, 2);
    if(error)
        return IRQ_HANDLED;

    // Update buttons
    input_report_key(gamepad_idev, BTN_A, packet & 0b10 << 0);
    input_report_key(gamepad_idev, BTN_B, packet & 0b10 << 1);
    input_report_key(gamepad_idev, BTN_TL, packet & 0b10 << 6);
    input_report_key(gamepad_idev, BTN_TR, packet & 0b10 << 7);

    // POV
    // @TODO check values
    input_report_abs(gamepad_idev, ABS_HAT0X, packet & 0b10 << 4 ? -1 : packet & 0b10 << 5 ? 1 : 0);
    input_report_abs(gamepad_idev, ABS_HAT0Y, packet & 0b10 << 2 ? -1 : packet & 0b10 << 3 ? 1 : 0);

    // Update pwr btn
    input_report_key(pwr_btn_idev, KEY_POWER, packet & 0x01);

    // report new state
    input_sync(gamepad_idev);
    input_sync(pwr_btn_idev);

    return IRQ_HANDLED;
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
    int ret = 0;
    etx_i2c_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);

    if (etx_i2c_adapter == NULL)
    {
        printk(KERN_ERR "No i2c bus\n");
        return -ENODEV;
    }

    etx_i2c_client_ATMega_GBASPi = i2c_new_client_device(etx_i2c_adapter, &ATMega_GBASPi_i2c_board_info);
    if (etx_i2c_client_ATMega_GBASPi == NULL)
    {
        printk(KERN_ERR "Unable to init device\n");
        return -ENODEV;
    }

    i2c_add_driver(&etx_ATMega_GBASPi_driver);    
    i2c_put_adapter(etx_i2c_adapter);

    ret = devm_request_threaded_irq(&etx_i2c_client_ATMega_GBASPi->dev, etx_i2c_client_ATMega_GBASPi->irq,
					  NULL, atmega_gbaspi_irq_handler,
					  IRQF_SHARED | IRQF_ONESHOT,
					  etx_i2c_client_ATMega_GBASPi->name, NULL);

    poweroff_handler = register_sys_off_handler(SYS_OFF_MODE_POWER_OFF_PREPARE, 0, ATMega_GBASPi_poweroff, NULL);
    if(IS_ERR_VALUE(poweroff_handler))
    {
        printk(KERN_WARNING "Unable to register poweroff handler, the system will not automatically cut power\n");
    }

    ATMega_GBASPi_Write(true);

    // Create the input device
    gamepad_idev = input_allocate_device();
    if(gamepad_idev == NULL)
    {
        ret = ENOMEM;
        goto err0;
    }

    // Set up its params
    gamepad_idev->name = "Gameboy controller";
    gamepad_idev->id.bustype = BUS_I2C;
    gamepad_idev->id.vendor = 0xcafe;
    gamepad_idev->id.product = 0xbabe; // @todo capire
    gamepad_idev->id.version = 1;
    gamepad_idev->dev.parent = &etx_i2c_client_ATMega_GBASPi->dev;

    // Set up POV-HAT
    input_set_abs_params(gamepad_idev, ABS_HAT0X, -1, 1, 0, 0);
    input_set_abs_params(gamepad_idev, ABS_HAT0Y, -1, 1, 0, 0);

    // Set up supported keys and axes
    gamepad_idev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
    set_bit(EV_ABS, gamepad_idev->evbit);
    set_bit(BTN_A, gamepad_idev->keybit);
    set_bit(BTN_B, gamepad_idev->keybit);
    set_bit(BTN_TL, gamepad_idev->keybit);
    set_bit(BTN_TR, gamepad_idev->keybit);

    // The power button
    pwr_btn_idev = input_allocate_device();
    if(pwr_btn_idev == NULL)
    {
        ret = ENOMEM;
        goto err1;
    }

    // Set up its params
    pwr_btn_idev->name = "Gameboy sys controls";
    pwr_btn_idev->id.bustype = BUS_I2C;
    pwr_btn_idev->id.vendor = 0xcafe;
    pwr_btn_idev->id.product = 0xbabe + 1; // @todo capire
    pwr_btn_idev->id.version = 1;
    pwr_btn_idev->dev.parent = &etx_i2c_client_ATMega_GBASPi->dev;

    pwr_btn_idev->evbit[0] = BIT_MASK(EV_KEY);
    set_bit(KEY_POWER, pwr_btn_idev->keybit);


    if(input_register_device(gamepad_idev))
    {
        ret = EINVAL;
        goto err2;
    }

    if(input_register_device(pwr_btn_idev))
    {
        ret = EINVAL;
        goto err3;
    }

    return 0;

err3:
    input_unregister_device(gamepad_idev);
err2:
    input_free_device(pwr_btn_idev);
err1:
    input_free_device(gamepad_idev);
err0:
    i2c_unregister_device(etx_i2c_client_ATMega_GBASPi);
    i2c_del_driver(&etx_ATMega_GBASPi_driver);

    if(! IS_ERR_OR_NULL(poweroff_handler))
        unregister_sys_off_handler(poweroff_handler);

    return ret;
}

// Module Exit function
static void __exit etx_driver_exit(void)
{
    input_unregister_device(pwr_btn_idev);
    input_unregister_device(gamepad_idev);
    input_free_device(pwr_btn_idev);
    input_free_device(gamepad_idev);

    i2c_unregister_device(etx_i2c_client_ATMega_GBASPi);
    i2c_del_driver(&etx_ATMega_GBASPi_driver);

    if(! IS_ERR_OR_NULL(poweroff_handler))
        unregister_sys_off_handler(poweroff_handler);
    
    pr_info("Driver Removed!!!\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);

// module_i2c_driver(etx_ATMega_GBASPi_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("GBASPi <GBASPi@free.me>");
MODULE_DESCRIPTION("GBASPi power & button handler");
MODULE_VERSION("0.1");
