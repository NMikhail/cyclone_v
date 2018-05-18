#include <linux/init.h>   // init(), exit()
#include <linux/module.h> // THIS_MODULE
#include <linux/errno.h>  //error code
#include <linux/version.h>//Linux version
#include <linux/kernel.h> //printk()
#include <linux/fs.h>     //alloc_chrdev_region(), struct file_operations
#include <linux/cdev.h>		//struct cdev
#include <linux/device.h> //class_create() device_create()
#include <linux/types.h>	//dev_t
#include <linux/spi/spi.h>  // spi_driver() spi_device()
#include <linux/of.h>   //of_match_ptr()
#include <linux/platform_device.h> // struct platform_driver platform_device() platform_get_resource()
#include <linux/uaccess.h> // copy_to_user() copy_from_user()

#include "spi-srz.h" //Фнукции и регистры контроллера SPI Designware 
#include "fifo.h"

#define SUCCESS 0
#define SPI_MAJOR 159
#define SPI_MINOR 0
#define SRZ_SPI_NAME "spisrz"
#define buf_depth 2048;

//********************************************************************************
struct srz_spi_mmio {
    struct srz_spi srz;
    struct clk *clk;
};
static struct device *dev;
static struct cdev spi_cdev;
static dev_t spi_num;
static struct class *dev_class;
static struct file_operations spisrz_fops;
static struct srz_spi *srz;
//********************************************************************************
static inline u32 srz_reg_read(struct srz_spi *srz, u32 offset){
    return readl_relaxed(srz->regs+offset);
}
static inline void srz_reg_write(struct srz_spi *srz, u32 offset, u32 val){
    writel_relaxed(val, srz->regs + offset);
}
static inline u8 srz_data_read(struct srz_spi *srz, u32 offset){
    return readb_relaxed(srz->regs+offset);
}
static inline void srz_data_write(struct srz_spi *srz, u32 offset, u8 val){
    writeb_relaxed(val, srz->regs + offset);
}
//***********************************MODULE SPI SLAVE*****************************
static int spisrz_open(struct inode *inode, struct file *spi_filp){
    FIFO(2048) *rx_buf, *tx_buf;
    pdata = kmalloc(sizeof(struct data_buffer),GFP_KERNEL);
    if (!pdata)
        return -ENOMEM; 
    pdata->rx_rd = 0;
    pdata->rx_wr = 0;
    pdata->tx_rd = 0;
    pdata->tx_wr = 0;
    pdata->rx_empty = 1;
    pdata->tx_empty = 1;
    pdata->rx_full  = 0;
    pdata->tx_full  = 0;

    spi_filp->private_data = pdata;
    srz_reg_write(srz, DW_SPI_SPIENR, 1);
	return SUCCESS;
}
static int spisrz_close(struct inode *inode, struct file *spi_filp){
    kfree(spi_filp->private_data);
	return SUCCESS;
}
static ssize_t spisrz_read(struct file *spi_filp, char __user *buff, size_t count,loff_t *offp){
    struct data_buffer *pdata = spi_filp->private_data;
    u16 addr_wr = pdata->rx_wr;
    u16 addr_rd = pdata->rx_rd;


    while (srz_reg_read(srz,DW_SPI_SR) & SR_RF_NOT_EMPT){        
        (pdata->buf_rd+addr_wr) = srz_data_read(srz, DW_SPI_DR);
        if (addr_wr!=buf_depth-1)
            addr_wr++;
        else
            addr_wr = 0;    
    }
    pdata->rx_wr = addr_wr;

    if (addr_rd+count<addr_wr){
        data->rx_rd = addr_rd+count;
        if (copy_to_user(buff, (pdata->buf_rd+addr_rd), count))
            return -EFAULT;
        return count;      
    }
    if (addr_rd<addr_wr && addr_rd+count>addr_wr){
        data->rx_rd = addr_wr;
        if (copy_to_user(buff, (pdata->buf_rd+addr_rd), addr_wr-addr_rd))
            return -EFAULT;
        return (addr_wr-addr_rd);
    }

    if (addr_rd>addr_wr && (addr_rd+count)%buf_depth>addr_wr)
        data->rx_rd = addr_rd+count;
        if (copy_to_user(buff, (pdata->buf_rd+addr_rd), count))
            return -EFAULT;
        return count;      
    }

    if (addr_rd>addr_wr && (addr_rd+count)%buf_depth<addr_wr)


        


    if (count>count_wr)
        return 0;
    else
        status=copy_to_user(buff, pdata->buf_rd, count);
    if (copy_to_user(buff, pdata->buf_rd, count))
        return -EFAULT;
    return 
    // for (count_wr = pdata->rx_wr;i<count;i++){
    //     if(srz_reg_read(srz,DW_SPI_SR) & SR_RF_NOT_EMPT)
    //         *(data+i) = srz_data_read(srz, DW_SPI_DR);
    //     else
    //         return 1;
    // }











    if (pdata->rx_empty)
        return 0;
    for (prd=0;prd<count;prd++){
        if (pdata->rx_rd=pdata->rx_wr)
            return(count-prd);
        if (pdata->rx_rd<pdata->rx_wr)

    }
    if (pdata->rx_rd<pdata->rx_wr){

    }


    u8 data[256];
    u8 i=0;
    ssize_t status;
    for (i=0;i<count;i++){
        if(srz_reg_read(srz,DW_SPI_SR) & SR_RF_NOT_EMPT)
            *(data+i) = srz_data_read(srz, DW_SPI_DR);
        else
            return 1;
    }
	status=copy_to_user(buff, data, count);
	return status;
}
static ssize_t spisrz_write(struct file *spi_filp, const char __user *buff, size_t count,loff_t *offp){
    u8 data[256];
    u8 i=0;
    ssize_t status;
	status = copy_from_user(data, buff, count);
    for (i=0;i<count;i++){
        if(srz_reg_read(srz,DW_SPI_SR) & SR_TF_NOT_FULL)
            srz_data_write(srz, DW_SPI_DR, data[i]);
        else
            return 1;
    }
	return status;
}
static long spisrz_ioctl(struct file *spi_filp, unsigned int cmd, unsigned long arg){
	printk(KERN_ALERT "+++ Driver: ioctl()\n");

    // if (_IOC_TYPE(cmd) != SRZ_IOC_MAGIC) return -ENOTTY;
    // switch (cmd){
    //     case SPI_ENABLE_IOC:
              
    //         printk(KERN_ALERT "spisrz slave enable\n");
    //         break;
    //     case SPI_CONFIG_IOC:
    //         srz_reg_write(srz, DW_SPI_CTRL0, (u32) *arg);
    //         srz_reg_write(srz, DW_SPI_SPIENR, (u32) *(arg+1);


    // }
	return SUCCESS;
}
//********************************************************************************
static struct file_operations spisrz_fops = {
	.owner = THIS_MODULE,
	.open = spisrz_open,
	.read = spisrz_read,
	.write = spisrz_write,
	.unlocked_ioctl = spisrz_ioctl,
	.release = spisrz_close,
};
//Определение устройства и его регистрация в файловой системе
static int spisrz_probe(struct platform_device *pdev){
    struct srz_spi_mmio *srzmmio;        
    struct resource *res;
    int ret;
    //Создаем класс
    dev_class = class_create(THIS_MODULE, SRZ_SPI_NAME);
    if (IS_ERR(dev_class))
        return 1;
    //Создаем структуру со старшим и младшим номером и регистрируем символьное устройство 
    spi_num=MKDEV(SPI_MAJOR,SPI_MINOR);
 	if(register_chrdev_region(spi_num,1,SRZ_SPI_NAME))  
        goto class_destroy;
    //Добавляем символьное устройство
    cdev_init(&spi_cdev,&spisrz_fops);
    spi_cdev.ops=&spisrz_fops;
    spi_cdev.owner=THIS_MODULE;
    if (cdev_add(&spi_cdev,spi_num,1)<0)
        goto unreg;          
    //Создаем устройство
    dev = device_create(dev_class, NULL, spi_num, NULL, SRZ_SPI_NAME);
    if (IS_ERR(dev))
        goto cdev_del;
    //Выделяем память под устройство
    srzmmio = devm_kzalloc(&pdev->dev, sizeof(struct srz_spi_mmio), GFP_KERNEL);
    if (!srzmmio)
        return -ENOMEM;    
    //Получаем адрес устройства из дерева устройств.
    srz = &srzmmio->srz;
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    srz->regs = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(srz->regs))
        return PTR_ERR(srz->regs);
    
    srz->irq = platform_get_irq(pdev,0);
    if (srz->irq < 0)
        return srz->irq;

    srzmmio->clk = devm_clk_get(&pdev->dev, NULL);
    if (IS_ERR(srzmmio->clk))
        return PTR_ERR(srzmmio->clk);
    
	if (clk_prepare_enable(srzmmio->clk));
		return ret;
    //Читаем узлы дерева устройств
    srz->slave = device_property_read_bool(&pdev->dev, "spi-slave");
    platform_set_drvdata(pdev, srzmmio);

    printk(KERN_ALERT "+++ spi-slave probe success\n");
	return SUCCESS;

	unreg:
		unregister_chrdev_region(spi_num,1);
	cdev_del:
		cdev_del(&spi_cdev);
	class_destroy:
		class_destroy(dev_class);		
	return 1;    
}
static int spisrz_remove(struct platform_device *pdev){
    struct srz_spi_mmio *srzmmio = platform_get_drvdata(pdev);
	clk_disable_unprepare(srzmmio->clk);
    device_destroy(dev_class,spi_num);
	cdev_del(&spi_cdev);
	unregister_chrdev_region(spi_num,1);
	class_destroy(dev_class);
    return 0;
}

static const struct of_device_id spisrz_of_match[] = {
	{ .compatible = "spisrz" },
	{},
};
MODULE_DEVICE_TABLE(of, spisrz_of_match);

static struct platform_driver spisrz_spi_driver = {
    .probe		= spisrz_probe,
	.remove 	= spisrz_remove,
	.driver		= {
		.name	= SRZ_SPI_NAME,
		.of_match_table = spisrz_of_match,
	},
};
module_platform_driver(spisrz_spi_driver);

MODULE_AUTHOR("Nikitin Mikhail, nikitin.mikhail@mail.ru");
MODULE_DESCRIPTION("SPI Designware Slave");
MODULE_LICENSE("GPL");