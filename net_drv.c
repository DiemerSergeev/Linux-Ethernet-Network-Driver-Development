/*
ТЕКСТ ЗАДАНИЯ ПО КУРСОВОМУ ПРОЕКТИРОВАНИЮ НА ТЕМУ "РАЗРАБОТКА СЕТЕВОГО ДРАЙВЕРА ДЛЯ LINUX"

1) Разработать драйвер сетевого устройства, реализующего следующие операции (структура net_device_ops):
    ndo_init
    ndo_uninit
    ndo_open
    ndo_stop
    ndo_get_stats
    ndo_start_xmit
    ndo_set_mac_address
    ndo_change_mtu

2) Название устройства должно включать фамилию студента, выполняющего курсовую работу

3) Реализовать два потока, осуществляющих изменение статистики передачи данных сетевого интерфейса: 
    * Первый поток осуществляет изменение (в сторону увеличения) параметров, содержащих информацию о переданных данных (количество переданных байт, количество переданных пакетов и т.д.) 
    * Второй - параметров, содержащих информацию о полученных данных (количество полученных байт, количество полученных пакетов и т.д.)
    * Потоки должны инкрементировать количество переданных / полученных байт на текущее значение MTU

4) Потоки должны создаваться при запуске устройства (ndo_open) и завершаться при остановке устройства (ndo_stop)

5) Начальные значения параметров статистики сетевого интерфейса должны передаваться в драйвер на этапе его загрузки как параметр

6) При вызове операции start_xmit должно осуществляться инкрементирование количества принятых и полученных пакетов, а также увеличение количества принятых и полученных байт на величину 2 MTU

7) Изменение и получение текущих параметров статистики должно осуществляться потокобезопасно и быть защищено мьютексом

8) Исполняемый файл драйвера должен содержать информацию об авторе - ФИО студента, выполнившего работу. 
   Данная информация должна быть доступна для отображения при помощи утилиты modinfo. Версия модуля должна соответствовать варианту курсовой работы

9) Осуществить отображение информации о созданном сетевом интерфейсе и проверку всех реализованных операций созданного сетевого интерфейса при помощи команд, выполняемых в терминале
*/
#include "net_drv.h"
/////////////////////////////////////////
MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Sergeev DA");
MODULE_VERSION("11.0");
MODULE_DESCRIPTION("Kernel network driver");
/////////////////////////////////////////
module_init(net_drv_init);
module_exit(net_drv_exit);
/////////////////////////////////////////
DEFINE_MUTEX(mut1);
DEFINE_MUTEX(mut2);
/////////////////////////////////////////
static long send_bytes = 0;
static long send_packets = 0;
static long receive_bytes = 0;
static long receive_packets = 0;
/////////////////////////////////////////
module_param(send_bytes, ulong, 0); // первый аргумент - имя переменной
module_param(send_packets, ulong, 0); // второй аргумент - тип переменной
module_param(receive_bytes, ulong, 0); // третий аргумент - параметр доступа
module_param(receive_packets, ulong, 0);
/////////////////////////////////////////
struct task_struct* th1 = NULL;
struct task_struct* th2 = NULL;

/*****************************************************************/
static int __init net_drv_init(void)
{
    DBGMSG("%s has been called\n", __FUNCTION__);
    
    printk("send_bytes = %ld\n", send_bytes); // входные данные с загрузкой драйвера
    printk("send_packets = %ld\n", send_packets); // входные данные с загрузкой драйвера
    printk("receive_bytes = %ld\n", receive_bytes); // входные данные с загрузкой драйвера
    printk("receive_packets = %ld\n", receive_packets); // входные данные с загрузкой драйвера
    
    netDev = alloc_netdev(0, DEVICE_NAME, NET_NAME_UNKNOWN, net_dev_setup); // описание сетевого устройства
    if(!netDev) // проверка ошибки создания описания сетевого устройства
    {
        DBGMSG("Failed to allocate netdevice\n");
        return -ENOMEM;
    }
    int err = register_netdev(netDev); // регистрация сетевого устройства
    if (err) // проверка ошибки регистрации сетевого устройства
    {
        DBGMSG("MyDriver: Failed to register netdevice, error %d\n", err);
        free_netdev(netDev); // удаление структуры net_device
        return err; // возвращение номера ошибки
    }
    DBGMSG("MyDriver: Netdevice %s registered successfully\n", netDev->name);
    DBGMSG("%s has been completed\n", __FUNCTION__);
    return 0;
}
/*****************************************************************/
static void __exit net_drv_exit(void)
{
    DBGMSG("%s has been called\n", __FUNCTION__);
    unregister_netdev(netDev); // удаление регистрации сетевого устройства
    free_netdev(netDev); // удаление структуры net_device
    DBGMSG("%s has been completed\n", __FUNCTION__);
}
/*****************************************************************/
int ThreadProcMut1(void* Arg)
{
    struct net_device* dev = (struct net_device*)Arg;

    DBGMSG("ThreadProcMut1 start\n");

    while (!kthread_should_stop())
    {
        msleep(1000);

        mutex_lock(&mut1); // блокировка Мютексом
        dev->stats.tx_bytes += dev->mtu; // инкремент переданных байт на значение MTU
        dev->stats.tx_packets++; // инкремент переданных пакетов
        mutex_unlock(&mut1); // разблокировка Мютекса
    }
    DBGMSG("ThreadProcMut1 stop\n");
    return 0;
}
/*****************************************************************/
int ThreadProcMut2(void* Arg)
{
    struct net_device* dev = (struct net_device*)Arg;

    DBGMSG("ThreadProcMut2 start\n");

    while (!kthread_should_stop())
    {
        msleep(1000);

        mutex_lock(&mut2); // блокировка Мютексом
        dev->stats.rx_bytes += dev->mtu; // инкремент полученных байт на значение MTU
        dev->stats.rx_packets++; // инкремент полученных пакетов
        mutex_unlock(&mut2);  // разблокировка Мютекса
    }
    DBGMSG("ThreadProcMut2 stop\n");
    return 0;
}
/*****************************************************************/
static int net_dev_open(struct net_device* dev)
{
    DBGMSG("%s has been called\n", __FUNCTION__);

    th1 = kthread_run(ThreadProcMut1, dev, "SendStats_%s", dev->name);
    if (IS_ERR(th1)) 
    {
        pr_err("MyNetDev: Failed to create SendStats thread: %ld\n", PTR_ERR(th1));
        th1 = NULL;
        return PTR_ERR(th1);
    }
    th2 = kthread_run(ThreadProcMut2, dev, "ReceiveStats_%s", dev->name);
    if (IS_ERR(th1)) 
    {
        pr_err("MyNetDev: Failed to create SendStats thread: %ld\n", PTR_ERR(th2));
        th2 = NULL;
        return PTR_ERR(th2);
    }
    netif_start_queue(dev);
    DBGMSG("%s has been completed\n", __FUNCTION__);
    return 0;
}
/*****************************************************************/
static int net_dev_stop(struct net_device* dev)
{
    DBGMSG("%s has been called\n", __FUNCTION__);
    netif_stop_queue(dev);

    if(th1 != ERR_PTR(-ENOMEM)) // проверка, если поток был запущен и уже завершен, то закрываем
    { 
       kthread_stop(th1);
       th1 = NULL;
    }
    if (th2 != ERR_PTR(-ENOMEM)) // проверка, если поток был запущен и уже завершен, то закрываем
    {
        kthread_stop(th2);
        th2 = NULL;
    }
    DBGMSG("%s has been completed\n", __FUNCTION__);
    return 0;
}
/*****************************************************************/
static int net_dev_start_xmit(struct sk_buff* skb, struct net_device* dev)
{
    DBGMSG("%s has been called\n", __FUNCTION__);

    mutex_lock(&mut1); // блокировка Мютексом
    dev->stats.tx_packets++; // инкремент переданных пакетов
    dev->stats.tx_bytes += (2 * dev->mtu); // инкремент переданных байт на значение 2MTU
    mutex_unlock(&mut1);  // разблокировка Мютекса
    
    mutex_lock(&mut2); // блокировка Мютексом  
    dev->stats.rx_packets++; // инкремент полученных пакетов
    dev->stats.rx_bytes += (2 * dev->mtu); // инкремент полученных байт на значение 2MTU
    mutex_unlock(&mut2);  // разблокировка Мютекса

    DBGMSG("%s has been completed\n", __FUNCTION__);

    dev_kfree_skb(skb); // освобождение сокетного буфера

    return NETDEV_TX_OK;
}
/*****************************************************************/
static void net_dev_setup(struct net_device* dev)
{
    DBGMSG("%s has been called\n", __FUNCTION__);

    ether_setup(dev); // установка настроек для ETHERNET
    
    dev->netdev_ops = &dev_ops; // загрузка всех команд
    dev->addr_len = ETH_ALEN; // установка длины MAC-адреса

    char srcMac[MAX_ADDR_LEN] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 };
    dev_addr_set(dev, srcMac); // установка MAC адреса по дефолту

    dev->stats.tx_bytes = send_bytes; // присваиваем входные данные в поля структуры
    dev->stats.tx_packets = send_packets; // присваиваем входные данные в поля структуры
    dev->stats.rx_bytes = receive_bytes; // присваиваем входные данные в поля структуры
    dev->stats.rx_packets = receive_packets; // присваиваем входные данные в поля структуры
    
    DBGMSG("%s has been completed\n", __FUNCTION__);
}
/*****************************************************************/
static struct net_device_stats* net_dev_stats(struct net_device* dev)
{
    DBGMSG("%s has been called\n", __FUNCTION__);
    DBGMSG("%s has been completed\n", __FUNCTION__);
    return &dev->stats; // возвращаем поле статистики из структуры dev
}
/*****************************************************************/
static int net_dev_set_mac_address(struct net_device* dev, void* addr)
{ 
    DBGMSG("%s has been called for %s\n", __FUNCTION__, dev->name);

    struct sockaddr *sa = (struct sockaddr *)addr; // структура адреса сокетов
    
    DBGMSG("%s: called for %s. Current flags: 0x%04X\n", __FUNCTION__, dev->name, dev->flags); 
     
    if (netif_running(dev)) // условие возможности поменять MAC-адрес
    {
        pr_warn("%s: Cannot set MAC address on running interface %s\n", DEVICE_NAME, dev->name);
        return -EBUSY;
    }
    if (!is_valid_ether_addr(sa->sa_data)) // условие возможности поменять MAC-адрес
    {
        pr_warn("%s: Invalid MAC address provided for %s: %pM\n", DEVICE_NAME, dev->name, sa->sa_data);
        return -EADDRNOTAVAIL;
    }
    dev_addr_set(dev, sa->sa_data); // присвоение MAC-адреса

    DBGMSG("MAC address for %s set to %pM\n", dev->name, dev->dev_addr);
    DBGMSG("%s has been completed\n", __FUNCTION__);
    return 0;
}
/*****************************************************************/
static int net_dev_init(struct net_device* dev)
{
    DBGMSG("%s has been called\n", __FUNCTION__);
    DBGMSG("%s has been completed\n", __FUNCTION__);
    return 0;
}
/*****************************************************************/
static void net_dev_uninit(struct net_device* dev)
{
    DBGMSG("%s has been called\n", __FUNCTION__);
    DBGMSG("%s has been completed\n", __FUNCTION__);
}
/*****************************************************************/
static int net_dev_change_mtu(struct net_device* dev, int new_mtu)
{
    DBGMSG("%s has been called\n", __FUNCTION__);
    DBGMSG("old mtu = %ld\n", dev->mtu);
    DBGMSG("min mtu = %ld\n", dev->min_mtu);
    DBGMSG("max mtu = %ld\n", dev->max_mtu);
    dev->mtu = new_mtu;
    DBGMSG("new mtu = %ld\n", dev->mtu);
    DBGMSG("%s has been completed\n", __FUNCTION__);
    return 0;
}

