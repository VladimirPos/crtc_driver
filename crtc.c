#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/time.h>
#include <linux/err.h>
#include <linux/rtc.h>
#include <linux/ktime.h>
#include <linux/proc_fs.h>

#define  DEVICE_NAME "crtc"
#define BUFSIZE  10

static struct platform_device *pdev = NULL;
struct rtc_device *rtc;
static struct timespec begtime;
static struct hrtimer hr_timer;
struct timespec now, diff;
unsigned long timer_interval_s = 1, timer_interval_ns = 0, time;
long counter = 0;
int mode = 2;
module_param(mode,int,0660);
int num,c,m,len;
char buf[BUFSIZE]; 
ktime_t ktime;


static inline void get_uptime(struct timespec* ts)
{
	getrawmonotonic(ts);
	get_monotonic_boottime(ts);
}

static int crtc_read_time(struct device *dev, struct rtc_time *tm)
{


	//get_uptime(&now);
	//diff = timespec_sub(now, begtime);
	if (mode==2)
		rtc_time_to_tm(time + counter, tm); 
	if (mode==1)
		rtc_time_to_tm(time + counter*2, tm); 
	if (mode==0)
		rtc_time_to_tm(time + counter/2, tm); 
	//rtc_time_to_tm(time + diff.tv_sec, tm); 
	return rtc_valid_tm(tm);
}

static int crtc_set_time(struct device *dev, struct rtc_time *tm)
{

	get_uptime(&begtime);
	rtc_tm_to_time(tm, &time);
	counter=0;


	return 0;
}

static const struct rtc_class_ops crtc_ops = {
	.read_time = crtc_read_time,
	.set_time = crtc_set_time
};

static int crtc_probe(struct platform_device *pdev)
{

	get_uptime(&begtime);

	rtc = rtc_device_register(pdev->name, &pdev->dev, &crtc_ops,
			THIS_MODULE);

	if (IS_ERR(rtc))
		return PTR_ERR(rtc);

	platform_set_drvdata(pdev, rtc);
	
	return 0;
}

static int crtc_remove(struct platform_device *pdev)
{
	rtc_device_unregister(platform_get_drvdata(pdev));
	return 0;
}

static struct platform_driver crtc_drv = {
	.probe = crtc_probe,
	.remove = crtc_remove,
	.driver = {
		.name = DEVICE_NAME,
		.owner = THIS_MODULE
	},
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
    interval = ktime_set(timer_interval_s,timer_interval_ns); 
    hrtimer_forward(timer_for_restart, currtime , interval);
    counter++;

  return HRTIMER_RESTART;
}

static int __init crtc_init(void)
{
	int err;
	ent=proc_create(DEVICE_NAME,0660,NULL,&proc_ops);
	err = platform_driver_register(&crtc_drv);
	pdev = platform_device_register_simple(DEVICE_NAME, -1, NULL, 0);
	ktime = ktime_set(timer_interval_s,timer_interval_ns );
    hrtimer_init( &hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
    hr_timer.function = &timer_callback;
    hrtimer_start( &hr_timer, ktime, HRTIMER_MODE_REL );
	time = ktime_get_real_seconds();
	return err;
}

static void __exit crtc_exit(void)
{
	platform_device_unregister(pdev);
	platform_driver_unregister(&crtc_drv);
	proc_remove(ent);
	hrtimer_cancel( &hr_timer );  
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vladimir Pososhnikov <v.pososhnikov@yandex.ru>");
MODULE_DESCRIPTION("A Linux char driver for emulation rtc-device");
MODULE_VERSION("0.2");

module_init(crtc_init);
module_exit(crtc_exit);