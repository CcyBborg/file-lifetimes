#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/init.h> 
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <sys/socket.h>
#include <unistd.h>

#define RECORDS_SIZE 1000
#define WORK_INTERVAL 1000
#define SOCK_NAME "lifetimes.s"

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Golik Konstantin");

struct lifetimes_record {
    char filename[255];
    unsigned long long starting;
    unsigned long long lifetime;
};

struct lifetimes_record *records[RECORDS_SIZE];

int sock_fd;

void close_sock(int sock_fd, char *name)
{
    close(sock_fd);
    unlink(name);
}

static int commands_socket_listener(void *threaddata)
{
    sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock_fd < 0)
    {
        printk("Unable to create socket");
        return 0;
    }

    struct sockaddr srvr_name;
    srvr_name.sa_family = AF_UNIX;
    strcpy(srvr_name.sa_data, SOCK_NAME);
    if (bind(sock_fd, &srvr_name, strlen(srvr_name.sa_data) + sizeof(srvr_name.sa_family)) < 0)
    {
        printk("Unable to bind socket");
        return 0;
    }

    printk("Command server is listening");

    char buf[100];
    int bytes = 0;
    while (1)
    {
        bytes = recv(sock_fd, buf, sizeof(buf), 0);
        if (bytes <= 0)
        {
            printk("Unable to read data");
        }
        buf[bytes] = 0;
        printk("Received %s", buf);
    }

    return 0;
}

struct task_struct *command_listener_task = kthread_create(commands_socket_listener, NULL, "command-listener");

void handle(struct work_struct *);

DECLARE_DELAYED_WORK(work, handle);

void handle(struct work_struct * _)
{
    printk("Lifetimes work handler triggered");
    int i;
    for (i = 0; i < RECORDS_SIZE; i++)
    {
        if (records[i] != NULL)
        {
            printk("%s - %d", records[i]->filename, i);
        }
    }
    schedule_delayed_work(&work, WORK_INTERVAL);
}

static int __init md_init(void)
{ 
    printk("Init started\n");
    schedule_delayed_work(&work, WORK_INTERVAL);
    wake_up_process(command_listener_task);
    printk("Initialized successfully");
    return 0; 
}

static void __exit md_exit(void) 
{
    cancel_delayed_work_sync(&work);
    kthread_stop(command_listener_task);
    close_sock(sock_fd, SOCK_NAME);
    printk("Terminated\n"); 
} 

module_init(md_init); 
module_exit(md_exit);
