#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/workqueue.h>

// character device
#include <linux/fs.h>
#include <linux/cdev.h>


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("I2C master driver for Raspberry Pi with arduino uno as slave");
MODULE_AUTHOR("Mark-Kitur");   
static const char *module_name = "raspi_master";

#define BUFFER_SIZE 13

// Structure to hold driver data
struct module_data {
    dev_t dev_num;
    struct cdev cdev;
    struct class *class;

    u8 *dev_buffer;
    size_t data_len;

    struct i2c_client *client;
    struct delayed_work poll_work;
};

//FOPS this will expose data to user space through /dev/raspi_master
static int master_open(struct inode *inode, 
                       struct file *file)
{
    struct module_data *data = container_of(inode->i_cdev, struct module_data, cdev);
    file->private_data = data;
    pr_info("%s: Device opened\n", module_name);
    return 0;
}

static int master_release(struct inode *inode,
                          struct file *file)
{
    pr_info("%s: Device closed\n", module_name);
    return 0;
}

static ssize_t master_read(struct file *file,
                           char __user *user_buf,
                           size_t count,
                           loff_t *offset)
{
    struct module_data *data = file->private_data;
    char output[64];
    int len = 0;
    int i;

    if (*offset > 0)
        return 0;

    for (i = 0; i < data->data_len; i++) {
        len += scnprintf(output + len,
                         sizeof(output) - len,
                         "%u ",
                         data->dev_buffer[i]);
    }

    len += scnprintf(output + len,
                     sizeof(output) - len,
                     "\n");

    if (count < len)
        return -EINVAL;

    if (copy_to_user(user_buf, output, len))
        return -EFAULT;

    *offset += len;

    return len;
}
        
// Define file operations for the character device
static struct file_operations fops ={
    .owner = THIS_MODULE,
    .open = master_open,
    .release = master_release,
    .read = master_read,
};

// read data from the I2C slave device
static int master_read_from_slave(struct i2c_client *client, 
                                  u8 *buffer){
    int ret ;
    //smbus read block data
    ret = i2c_master_recv(client, buffer, BUFFER_SIZE);
    if(ret < 0){
        dev_err(&client->dev, "%s: Failed to read from slave device\n", module_name);
        return ret;
    }
    return ret;
} 

// periodic work function to read data from the I2C slave device and store it in the device buffer
static void master_periodic_work(struct work_struct *work)
{
    struct module_data *data =container_of(to_delayed_work(work),struct module_data,poll_work);

    int ret;
    int i;
    u8 buffer[BUFFER_SIZE];

    ret = master_read_from_slave(data->client,
                                 buffer);

    if (ret < 0) {
        dev_err(&data->client->dev,
                "%s: Failed to read from slave device\n",
                module_name);
        goto reschedule;
    }

    memcpy(data->dev_buffer, buffer, ret);
    data->data_len = ret;

    dev_info(&data->client->dev,
             "%s: Received %d bytes:\n",
             module_name,
             ret);

    // for (i = 0; i < ret; i++) {
    //     dev_info(&data->client->dev,
    //              "%s: Buffer[%d] = %u\n",
    //              module_name,
    //              i,
    //              buffer[i]);
    // }

reschedule:
    schedule_delayed_work(&data->poll_work,
                          msecs_to_jiffies(1000));
}

static char *master_devnode(const struct device *dev, umode_t *mode)
{
    if (mode)
        *mode = 0666;
    return NULL;
}   


static int master_probe(struct i2c_client *client)
{
    struct module_data *data;
    int ret;

    dev_info(&client->dev,
             "%s: I2C device probed, address = 0x%02x\n",
             module_name, client->addr);

    /* Allocate driver data */
    data = devm_kzalloc(&client->dev,
                        sizeof(*data),
                        GFP_KERNEL);

    if (!data)
        return -ENOMEM;

    i2c_set_clientdata(client, data);
    data->client = client;


    /* Allocate device number */
    ret = alloc_chrdev_region(&data->dev_num,
                              0,
                              1,
                              module_name);

    if (ret) {
        dev_err(&client->dev,
                "%s: Failed to allocate char device region\n",
                module_name);
        return ret;
    }
    /* Initialize character device */
    cdev_init(&data->cdev, &fops);
    data->cdev.owner = THIS_MODULE;


    /* Add character device */
    ret = cdev_add(&data->cdev,
                   data->dev_num,
                   1);

    if (ret) {
        dev_err(&client->dev,
                "%s: Failed to add cdev\n",
                module_name);

        goto err_unregister;
    }


    /* Create class */
    data->class = class_create(module_name);

    if (IS_ERR(data->class)) {
        ret = PTR_ERR(data->class);

        dev_err(&client->dev,
                "%s: Failed to create class\n",
                module_name);

        goto err_cdev;
    }

    // Set the devnode function to set permissions for the device node
    data->class->devnode = master_devnode;


    /* Create /dev/raspi_master */
    if (IS_ERR(device_create(data->class,
                             NULL,
                             data->dev_num,
                             NULL,
                             module_name))) {

        dev_err(&client->dev,
                "%s: Failed to create device\n",
                module_name);

        ret = -ENOMEM;

        goto err_class;
    }


    /* Allocate I2C data buffer */
    data->dev_buffer = devm_kzalloc(&client->dev,
                                    BUFFER_SIZE,
                                    GFP_KERNEL);

    if (!data->dev_buffer) {
        ret = -ENOMEM;
        goto err_device;
    }


    /* Initialize polling work */
    INIT_DELAYED_WORK(&data->poll_work,
                      master_periodic_work);

    schedule_delayed_work(&data->poll_work,
                          msecs_to_jiffies(500));


    dev_info(&client->dev,
             "%s: Character device /dev/%s created\n",
             module_name,
             module_name);

    return 0;


err_device:
    device_destroy(data->class, data->dev_num);

err_class:
    class_destroy(data->class);

err_cdev:
    cdev_del(&data->cdev);

err_unregister:
    unregister_chrdev_region(data->dev_num, 1);

    return ret;
}

static void master_remove(struct i2c_client *client)
{
    struct module_data *data;
    data = i2c_get_clientdata(client);
    cancel_delayed_work_sync(&data->poll_work);
    device_destroy(data->class, data->dev_num);
    class_destroy(data->class);
    cdev_del(&data->cdev);
    unregister_chrdev_region(data->dev_num, 1);
    dev_info(&client->dev,
             "%s: I2C device removed\n",
             module_name);
}




//set I2C device tree match table
static const struct of_device_id master_of_match[] = {
    { .compatible = "arduino,uno"},
    {},
};
MODULE_DEVICE_TABLE(of, master_of_match);

//driver struct
static struct i2c_driver master_driver = {
    .probe = master_probe,
    .remove = master_remove,
    .driver = {
        .name = "raspi_master",
        .of_match_table = master_of_match,
    },
};

module_i2c_driver(master_driver);

