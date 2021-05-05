#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev_t.h>
#include<linux/uaccess.h>
#define DEV_MEM_SIZE 512
//#undef pr_fmt
//#define pr_fmt(fmt) "%s :" ,fmt __func__
 
char dev_buff[DEV_MEM_SIZE];

dev_t device_number;

struct class *class_pcd;

struct device *device_pcd;

struct cdev pcd_cdev;

loff_t pcd_lseek (struct file *filp, loff_t offset, int whence)
{
	loff_t temp;
	pr_info("lseek requested\n");
	pr_info("current value file position =%lld\n",filp->f_pos);
	switch(whence)
	{
	case SEEK_SET:
		if((offset > DEV_MEM_SIZE) || (offset < 0) )
		return -EINVAL;
		filp->f_pos=offset;
		break;
	case SEEK_CUR: 
		temp= filp->f_pos + offset;
		if((temp > DEV_MEM_SIZE) || (temp <0))
		return -EINVAL;
		filp->f_pos=temp;
		break;
	case SEEK_END:
		temp= DEV_MEM_SIZE + offset;
                if((temp > DEV_MEM_SIZE) || (temp <0))
                return -EINVAL;
		filp->f_pos=DEV_MEM_SIZE+offset;
		break;
	default: 	
		return -EINVAL;
	}
        pr_info("new value of the  file position =%lld\n",filp->f_pos);

        return filp->f_pos;
}

ssize_t pcd_read (struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{	
	pr_info("read requested for %zu bytes \n",count);
        pr_info("current f_pos =%lld\n",*f_pos);

	/* adust the count */
	if((*f_pos + count) > DEV_MEM_SIZE)
		count = DEV_MEM_SIZE - *f_pos;

	/*copy to user*/
	if(copy_to_user(buff,&dev_buff[*f_pos],count)){
 	return -EFAULT;
	}
        
	/*update the current file position*/
	*f_pos+= count;
	pr_info("number of bytes successfully read =%zu\n",count);
	pr_info("updated f_pos =%lld\n",*f_pos);
	/* return number of bytes which have been succesfully read*/
	return count;
}

ssize_t pcd_write (struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
        pr_info("write requested for %zu bytes \n",count);
        pr_info("current f_pos =%lld\n",*f_pos);

        /* adust the count */
        if((*f_pos + count) > DEV_MEM_SIZE)
                count = DEV_MEM_SIZE - *f_pos;

	if(!count)
	{	
		pr_err("no space left on device\n");	
		return -ENOMEM;

	}	
        /*copy from user*/
        if(copy_from_user(&dev_buff[*f_pos],buff,count)){
        return -EFAULT;
        }

        /*update the current file position*/
        *f_pos+= count;
        pr_info("number of bytes successfully written =%zu\n",count);
        pr_info("updated f_pos =%lld\n",*f_pos);
        /* return number of bytes which have been succesfully written*/
        return count;

	
}


int pcd_open (struct inode *inode, struct file *filp)
{
	pr_info("open was successful\n");
        
	return 0;

}

int pcd_release (struct inode *inode, struct file *filp)
{
	pr_info("close was successful\n");
        
	return 0;
}

static struct file_operations pcd_fops=
{
	.open=pcd_open,
	.write=pcd_write,
	.read=pcd_read,
	.llseek=pcd_lseek,
	.release=pcd_release,
	.owner=THIS_MODULE
};


static int __init pcd_init(void)
{
	alloc_chrdev_region(&device_number,0,1,"pcd_devices");
	
	pr_info("pcd_init: in init function\n");
	pr_info("Device number <major>:<minor> =%d:%d\n",MAJOR(device_number),MINOR(device_number));
	cdev_init(&pcd_cdev,&pcd_fops);	
	
	pcd_cdev.owner=THIS_MODULE;
	cdev_add(&pcd_cdev,device_number,1);	
	
	class_pcd = class_create(THIS_MODULE,"pcd_class");
	
	device_pcd=device_create(class_pcd,NULL,device_number,NULL,"pcd");

	pr_info("module init was successfull\n");
	return 0;
}

static void __exit pcd_exit(void)
{
	device_destroy(class_pcd,device_number);
	class_destroy(class_pcd);
	cdev_del(&pcd_cdev);
	unregister_chrdev_region(device_number,1);
	pr_info("pcd_Exit: in cleanup module");
	pr_info("module unloaded\n");

}

module_init(pcd_init);
module_exit(pcd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("chandan");
MODULE_DESCRIPTION("A simple pcd char driver");
