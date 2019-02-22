#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/moduleparam.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>
#include <linux/time.h>

#define  DEVICE_NAME "crtc"
#define  CLASS_NAME  "classcrtc"
#define BUFSIZE  100

struct tm  time;
struct timeval tv;
unsigned long timer_interval_ns = 1e7, counter = 0,intime, s_to_ns = 1e9;
static struct hrtimer hr_timer;
ktime_t ktime, outtime;
int mlt = 10;
int num,c,m,ret, len;
char buf[BUFSIZE]; 
static int    majorNumber;
static struct class*  crtcClass  = NULL;
static struct device* crtcDevice = NULL;
static int mode = 2;
module_param(mode,int,0660);
 
static int dev_open(struct inode *inodep, struct file *filep){
   return 0;
}

static ssize_t dev_read(struct file *file, char __user *ubuf,size_t count, loff_t *ppos) 
{
   tv = ns_to_timeval(outtime);
   time64_to_tm(tv.tv_sec,0, &time);
   len=0;
   if(*ppos > 0 || count < BUFSIZE)
      return 0;
   len += sprintf(buf,"%ld-%d-%d %d:%d:%d.%ld\n", time.tm_year+1900, time.tm_mon+1, time.tm_mday,
   time.tm_hour, time.tm_min, time.tm_sec, tv.tv_usec);
   
   if(copy_to_user(ubuf,buf,len))
      return -EFAULT;
   *ppos = len;
   return len;
}
static ssize_t dev_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos) 
{
   if(*ppos > 0 || count > BUFSIZE)
      return -EFAULT;
   if(copy_from_user(buf, ubuf, count))
      return -EFAULT;
   num = sscanf(buf,"%ld-%d-%d %d:%d:%d.%ld\n", &time.tm_year, &time.tm_mon, &time.tm_mday,
   &time.tm_hour, &time.tm_min, &time.tm_sec, &tv.tv_usec);
   intime = mktime(time.tm_year, time.tm_mon, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
   intime = ((intime*s_to_ns)+tv.tv_usec);
   counter=0;
   c = strlen(buf);
   *ppos = c;
   return c;
}

static int dev_release(struct inode *inodep, struct file *filep){
   return 0;
}

static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};
 
static struct proc_dir_entry *ent;

static ssize_t crtcwrite(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos) 
{
	if(*ppos > 0 || count > BUFSIZE)
		return -EFAULT;
	if(copy_from_user(buf, ubuf, count))
		return -EFAULT;
	num = sscanf(buf,"%d",&m);
	if(num != 1)
		return -EFAULT;
	mode = m;
	c = strlen(buf);
	*ppos = c;
   if(mode==1)
      mlt=2;
   if(mode==0)
      mlt=20;
   return c;
}
 
static ssize_t crtcread(struct file *file, char __user *ubuf,size_t count, loff_t *ppos) 
{
	len=0;
	if(*ppos > 0 || count < BUFSIZE)
		return 0;
	len += sprintf(buf,"mode = %d\n",mode);
	if(copy_to_user(ubuf,buf,len))
		return -EFAULT;
	*ppos = len;
	return len;
}
 
static struct file_operations proc_ops = 
{
	.owner = THIS_MODULE,
	.read = crtcread,
	.write = crtcwrite,
};
 
enum hrtimer_restart timer_callback( struct hrtimer *timer_for_restart )
{
    ktime_t currtime , interval;
    currtime  = ktime_get();
    interval = ktime_set(0,timer_interval_ns); 
    hrtimer_forward(timer_for_restart, currtime , interval);
    counter=counter+10;
    outtime = (intime+(counter/mlt*timer_interval_ns));

  return HRTIMER_RESTART;
}

static int __init timer_init(void) { 
      ent=proc_create(DEVICE_NAME,0660,NULL,&proc_ops);
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0){
      printk(KERN_ALERT "Failed to register a major number\n");
      return majorNumber;
   }
   crtcClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(crtcClass)){                
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(crtcClass);
   } 
   crtcDevice = device_create(crtcClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(crtcDevice)){
      class_destroy(crtcClass);
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(crtcDevice);
   }
  printk(KERN_ALERT "custom rtc driver loaded!\n");
  intime = ktime_get_real_ns();
  ktime = ktime_set( 0, timer_interval_ns );
  hrtimer_init( &hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
  hr_timer.function = &timer_callback;
  hrtimer_start( &hr_timer, ktime, HRTIMER_MODE_REL );
  return 0;
}

static void __exit timer_exit(void) {
  proc_remove(ent);
   device_destroy(crtcClass, MKDEV(majorNumber, 0));
   class_unregister(crtcClass);
   class_destroy(crtcClass);
   unregister_chrdev(majorNumber, DEVICE_NAME);
    ret = hrtimer_cancel( &hr_timer );    
    printk(KERN_ALERT "rtc module uninstalling");  
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vladimir Pososhnikov <v.pososhnikov@yandex.ru>");
MODULE_DESCRIPTION("A Linux char driver for emulation rtc-device");
MODULE_VERSION("0.1");

module_init(timer_init);
module_exit(timer_exit);