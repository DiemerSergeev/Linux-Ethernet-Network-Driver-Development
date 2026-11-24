# Linux Network Device Driver

Linux Ethernet kernel driver written in C, built and tested as a loadable kernel module on a Linux virtual machine.

## Features

- Ethernet network device registered with the Linux networking subsystem
- Kernel threads for background processing
- Mutex synchronization for safe concurrent access to shared data
- Packet statistics (transmitted and received packets)
- MAC address and MTU configuration
- Configurable module parameters

## Tech Stack

C, Linux Kernel, kernel modules (Kbuild), Linux VM

## Requirements

- Linux with kernel headers installed for the running kernel
- GCC and Make

```bash
sudo apt install build-essential linux-headers-$(uname -r)
```

## Build

```bash
make
```

This produces the kernel module `<module_name>.ko`.

## Usage

Load the module:

```bash
sudo insmod <module_name>.ko
```

Load with module parameters:

```bash
sudo insmod <module_name>.ko <parameter>=<value>
```

Check that the device is registered and view kernel messages:

```bash
ip link show
dmesg | tail
```

Change the MAC address and MTU:

```bash
sudo ip link set dev <interface> address <MAC>
sudo ip link set dev <interface> mtu <MTU>
```

View packet statistics:

```bash
ip -s link show <interface>
```

Unload the module:

```bash
sudo rmmod <module_name>
```

## Module Parameters

| Parameter | Description | Default |
|---|---|---|
| `<parameter>` | `<description>` | `<default>` |

## Design Notes

- **Kernel threads** handle background work separately from the main driver path.
- **Mutexes** protect shared state (for example, packet statistics) from concurrent access by several threads.
- **Packet statistics** are collected by the driver and exposed through the standard network interface statistics.

## Testing

The driver was built, loaded, and tested as a kernel module on a Linux virtual machine. Testing covered module loading and unloading, interface registration, packet handling, and MAC/MTU configuration.

## Project Structure

```
.
├── <driver_source>.c
├── Makefile
└── README.md
```

## Author

Dmitrii Sergeev | [LinkedIn](https://www.linkedin.com/in/dmitrii-diemer-sergeev/) | [GitHub](https://github.com/DiemerSergeev)
