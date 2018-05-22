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
#include <linux/interrupt.h>

#include "spi-srz.h" //Фнукции и регистры контроллера SPI Designware 
#include "fifo.h"

#define SUCCESS 0
#define SPI_MAJOR 159
#define SPI_MINOR 0
#define SRZ_SPI_NAME "spisrz"

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
static struct fifo *pspififo;
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
    spi_filp->private_data = pspififo;
    srz_reg_write(srz, DW_SPI_SPIENR, 1);
	return SUCCESS;
}
static int spisrz_close(struct inode *inode, struct file *spi_filp){
	return SUCCESS;
}
static ssize_t spisrz_read(struct file *spi_filp, char __user *buff, size_t count,loff_t *offp){
    struct fifo *pdata = spi_filp->private_data;
    u8 data[FIFO_SIZE];
    u32 i;
    if (count>FIFO_SIZE){
        printk("SPI Slave rx count > FIFO_SIZE\n");
        return 0;
    }
    
    while (srz_reg_read(srz,DW_SPI_SR) & SR_RF_NOT_EMPT){
        if (!fifo_rx_is_full(pdata))
            fifo_rx_write(pdata, srz_data_read(srz, DW_SPI_DR));
        else{
            printk("SPI Slave FIFO RX is overflow.\n");
            return 0;
        }
    }

    for (i=0;i<count;i++){
        if (!fifo_rx_is_empty(pdata)) 
            *(data+i)=fifo_rx_read(pdata);
        else{
            printk("SPI Slave FIFO RX is empty.\n");
            break;
        }
    }
    if (copy_to_user(buff, data, i))
        return -EFAULT;
    return i;
}
static ssize_t spisrz_write(struct file *spi_filp, const char __user *buff, size_t count,loff_t *offp){
    struct fifo *pdata = spi_filp->private_data;
    u8 data[FIFO_SIZE];
    u32 i=0;
    if (count>FIFO_SIZE){
        printk("SPI Slave tx count > FIFO_SIZE\n");
        return 0;
    }
    printk("+++ spisrz write pointer data: %.8X", (u32)pdata);
    i=copy_from_user(data, buff, count);
    for (i=0;i<count;i++){
        if (!fifo_tx_is_full(pdata))
            fifo_tx_write(pdata, data[i]);
        else{
            printk("SPI Slave FIFO TX is overflow.\n");
            break;
        }
    }
    srz_reg_write(srz, DW_SPI_IMR, srz_reg_read(srz, DW_SPI_IMR) | SPI_INT_TXEI);
    return i;
}
static long spisrz_ioctl(struct file *spi_filp, unsigned int cmd, unsigned long arg){
	printk(KERN_ALERT "+++ Driver: ioctl()\n");
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
//*************************Обработчик прерывания**********************************
static irqreturn_t spisrz_irq(int irq, void *dev_id){
    struct fifo *pdata = dev_id;
    u32 flag;
    u8 master_on;
    flag = srz_reg_read(srz,DW_SPI_ISR);
    printk(KERN_ALERT "+++ Interrupt SPI slave, status interrupt: %.8X\n", flag);
    if (flag & SPI_INT_TXEI){//Если буфер передачи пуст
        printk(KERN_ALERT "+++ Interrupt TX empty\n");
        if (fifo_tx_is_empty(pdata)){
            printk(KERN_ALERT "+++ Interrupt fifo_tx_is_empty\n");
            master_on = 0;
            srz_reg_write(srz, DW_SPI_IMR, srz_reg_read(srz,DW_SPI_IMR) & ~SPI_INT_TXEI);
            return IRQ_HANDLED;
        }
        else{
            printk(KERN_ALERT "+++ Interrupt fifo_tx_not_empty\n");
            while(srz_reg_read(srz,DW_SPI_SR) & SR_TF_NOT_FULL){
                if (!fifo_tx_is_empty(pdata))
                    srz_data_write(srz, DW_SPI_DR, fifo_tx_read(pdata));
                else 
                    return IRQ_HANDLED;
            }
        }
    }
    if (flag & SPI_INT_RXFI){//Если буфер приема полон
        printk(KERN_ALERT "+++ Interrupt RX full\n");
        while (srz_reg_read(srz,DW_SPI_SR) & SR_RF_NOT_EMPT){
            if (!fifo_rx_is_full(pdata)){
                printk(KERN_ALERT "+++ Interrupt fifo_rx_not_full\n");
                fifo_rx_write(pdata, srz_data_read(srz, DW_SPI_DR));
            }
            else{
                printk("SPI Slave interrupt FIFO RX is overflow.\n");
                srz_reg_write(srz, DW_SPI_IMR, srz_reg_read(srz,DW_SPI_IMR) & ~SPI_INT_RXFI);
                return IRQ_HANDLED;
            }
        }
    }

    return IRQ_HANDLED;
}
// static int irq_counter;
// static irqreturn_t my(int irq, void *dev_id){
//     irq_counter++;
//     printk(KERN_ALERT "ISR: counter = %d\n",irq_counter);
//     return IRQ_NONE;
// }
//********************************************************************************
//Определение устройства и его регистрация в файловой системе
static int spisrz_probe(struct platform_device *pdev){
    struct srz_spi_mmio *srzmmio;        
    struct resource *res;
    int ret;
    //Создаем класс
    printk(KERN_ALERT "+++ spi-slave probe hello\n");
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
    
    ret = clk_prepare_enable(srzmmio->clk);
	if (ret)
		return ret;
    //Читаем узлы дерева устройств
    srz->slave = device_property_read_bool(&pdev->dev, "spi-slave");
    //Разрешаем прерывания
    printk(KERN_ALERT "+++ spi-slave probe success\n");


    pspififo = kmalloc(sizeof(struct fifo),GFP_KERNEL);
    if (!pspififo)
        return -ENOMEM;
    fifo_rx_erase(pspififo);
    fifo_tx_erase(pspififo);


    ret = request_irq(srz->irq, spisrz_irq ,IRQF_SHARED, "spisrz", pspififo);
    if (ret) {
        printk(KERN_ALERT "can not get IRQ\n");
    }
    
    printk("+++ irq num: %d", srz->irq);

    platform_set_drvdata(pdev, srzmmio);
    
  //  if (request_irq(40, my, IRQF_SHARED, "my_interrup", &ret))
    //    return -1;



    srz_reg_write(srz, DW_SPI_IMR, SPI_INT_RXFI);

    printk("+++ spisrz probe pointer data: %.8X, irq num: %d", (u32)pspififo, srz->irq);

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
    kfree(pspififo);
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