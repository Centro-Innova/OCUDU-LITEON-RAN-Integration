# O-RAN Synchronization Configuration (PTP and GPS)

This directory contains the configuration files and `systemd` services required to establish the synchronization infrastructure for the 5G O-RAN testbed.

Accurate time and phase synchronization is a strict requirement for the operation of the O-RAN Fronthaul interface between the Central/Distributed Unit (OCUDU) and the Radio Unit (LITEON FlexFi O-RU).

In this testbed, the GPS antenna is used as the time reference for the hardware clock of the Intel network interface card (NIC). The NIC operates as the **Grandmaster Clock**, using the Telecom Profile G.8275.1. The system clock is also synchronized to the NIC hardware clock to minimize clock drift during gNB operation.

---
## LinuxPTP Installation

The synchronization configuration in this repository uses **LinuxPTP**. If LinuxPTP is not already installed, clone the official source repository and compile it before applying the configuration files.

### Download and Build LinuxPTP

Clone the LinuxPTP repository:

```bash
git clone git://git.code.sf.net/p/linuxptp/code linuxptp
```

Enter the source directory:

```bash
cd linuxptp/
```

Build LinuxPTP:

```bash
make
```

Install the compiled binaries:

```bash
sudo make install
```
---
## Directory Files 

### Configuration Files (`/etc/linuxptp/`)

#### `ptp4l.cfg`

- **Purpose:** Main configuration file for the Precision Time Protocol (PTP).
- **Function:** Defines the PTP protocol parameters using the G.8275.1 profile.
- The network interface used in this testbed is `ens4f0np0`.
- The interface is configured to operate exclusively as the Grandmaster using `serverOnly 1`.
- The configuration also defines the PTP message intervals and the multicast destination MAC address required by the O-RU:

```text
01:1B:19:00:00:00
```

#### `ts2phc.conf`

- **Purpose:** Configuration file for `ts2phc` (Time Stamp to Physical Hardware Clock).
- **Function:** Defines how timing information obtained from the GPS antenna is processed and transferred to the NIC hardware clock.
- The configuration uses NMEA data received through the GPS serial interface and maps the timing reference to the Intel NIC.

---

### Systemd Services (`/etc/systemd/system/`)

#### `ts2phc.service`

- **Purpose:** GPS timing ingestion service.
- **Function:** Executes `ts2phc` using the `ts2phc.conf` configuration file.
- This is the first step in the synchronization chain, providing the NIC hardware clock with timing information obtained from the GPS antenna.

#### `ptp4l-gm.service`

- **Purpose:** PTP Grandmaster service.
- **Function:** Executes `ptp4l` using the `ptp4l.cfg` configuration file.
- The service is configured with CPU affinity and real-time scheduling to provide more deterministic PTP packet processing.

#### `phc2sys.service`

- **Purpose:** System clock synchronization service.
- **Function:** Synchronizes the Linux system clock (`CLOCK_REALTIME`) with the Physical Hardware Clock (PHC) of the Intel NIC.
- This ensures that applications running in userspace, including OCUDU, operate using a clock synchronized with the NIC hardware clock.
- The service can also be configured with CPU affinity and real-time scheduling.

---

## Deployment

> **Note:** Before applying these files, verify that the network interface used by the server is `ens4f0np0`. If a different interface is used, update `ptp4l.cfg`, `ts2phc.conf`, and the corresponding systemd services accordingly.
>
> The CPU affinity configuration must also be adjusted according to the CPU topology of the server.

### Step 1: Copy the Configuration Files

Copy the `.cfg` and `.conf` files to the `linuxptp` configuration directory:

```bash
sudo cp ptp4l.cfg /etc/linuxptp/
sudo cp ts2phc.conf /etc/linuxptp/
```

### Step 2: Copy the Systemd Services

Copy the service files to the systemd service directory:

```bash
sudo cp ts2phc.service /etc/systemd/system/
sudo cp ptp4l-gm.service /etc/systemd/system/
sudo cp phc2sys.service /etc/systemd/system/
```

### Step 3: Reload Systemd

Reload the systemd configuration so that the new services are recognized:

```bash
sudo systemctl daemon-reload
```

### Step 4: Enable the Services

Enable the services so that they start automatically during system boot:

```bash
sudo systemctl enable ts2phc.service
sudo systemctl enable ptp4l-gm.service
sudo systemctl enable phc2sys.service
```

### Step 5: Start the Synchronization Services

Start the services in the following order:

```bash
sudo systemctl restart ts2phc.service
sudo systemctl restart ptp4l-gm.service
sudo systemctl restart phc2sys.service
```

The synchronization chain is:

```text
GPS Antenna
     |
     | NMEA / PPS
     v
   ts2phc
     |
     v
Intel NIC PHC
     |
     v
   ptp4l
     |
     v
PTP Grandmaster
     |
     v
O-RAN Network

Intel NIC PHC
     |
     v
  phc2sys
     |
     v
CLOCK_REALTIME
```

---

## Validation and Status

After starting the synchronization services, verify that each component is operating correctly.

### 1. GPS Synchronization (`ts2phc`)

Check the service status:

```bash
sudo systemctl status ts2phc.service
```

To monitor the service logs in real time:

```bash
sudo journalctl -u ts2phc.service -f
```

The logs should indicate that timing information is being received from the GPS source and applied to the NIC hardware clock.

> `[INSERT SCREENSHOT HERE - ts2phc]`

---

### 2. PTP Grandmaster (`ptp4l-gm`)

Check the service status:

```bash
sudo systemctl status ptp4l-gm.service
```

Monitor the PTP logs:

```bash
sudo journalctl -u ptp4l-gm.service -f
```

The PTP port state can also be checked using:

```bash
sudo pmc -u -b 0 'GET PORT_DATA_SET'
```

The port should report the following state:

```text
MASTER
```

> `[INSERT SCREENSHOT HERE - ptp4l-gm]`

---

### 3. System Clock Synchronization (`phc2sys`)

Check the service status:

```bash
sudo systemctl status phc2sys.service
```

Monitor the synchronization process:

```bash
sudo journalctl -u phc2sys.service -f
```

The `phc2sys` logs should show the offset between the NIC hardware clock and the system clock converging toward zero.

> `[INSERT SCREENSHOT HERE - phc2sys]`

---

## Synchronization Architecture

The complete synchronization architecture used in the testbed is:

```text
                       GPS Antenna
                            |
                     NMEA / PPS Signal
                            |
                            v
                        ts2phc
                            |
                            v
                  Intel NIC Hardware Clock
                         (PHC)
                            |
                            +------------------+
                            |                  |
                            v                  v
                          ptp4l             phc2sys
                            |                  |
                            v                  v
                    PTP Grandmaster       CLOCK_REALTIME
                            |                  |
                            |                  |
                            +--------+---------+
                                     |
                                     v
                                   OCUDU
                                     |
                              O-RAN Fronthaul
                                     |
                                     v
                              LITEON FlexFi
```
## Validation

After completing the synchronization configuration, the following steps can be used to verify that the services are running correctly and that the LITEON FlexFi O-RU can be accessed.

### Step 1: Reload Systemd

Reload the systemd configuration:

```bash
sudo systemctl daemon-reload
```

### Step 2: Start the Synchronization Services

Start `ts2phc` and `ptp4l`:

```bash
sudo systemctl start ts2phc
sudo systemctl start ptp4l
```

If `phc2sys` is configured as a systemd service, start it as well:

```bash
sudo systemctl start phc2sys
```

### Step 3: Verify `ts2phc`

Check the service status:

```bash
sudo systemctl status ts2phc
```

To monitor the logs in real time:

```bash
sudo journalctl -u ts2phc -f
```

The service should be running without errors and receiving the timing information from the GPS source.

### Step 4: Verify `ptp4l`

Check the service status:

```bash
sudo systemctl status ptp4l
```

Monitor the PTP logs:

```bash
sudo journalctl -u ptp4l -f
```

The PTP interface should successfully initialize and operate as the configured Grandmaster.

### Step 5: Verify `phc2sys`

Monitor the system clock synchronization:

```bash
sudo journalctl -u phc2sys -f
```

The offset between the NIC hardware clock and the system clock should converge toward zero.

---

## Accessing the LITEON FlexFi O-RU

The LITEON FlexFi O-RU can be accessed through its serial console.

First, install `minicom`:

```bash
sudo apt update
sudo apt install minicom
```

Identify the USB serial device connected to the O-RU. In this testbed, the device is:

```text
/dev/ttyUSB0
```

The device name may be different depending on which USB port is used and how the operating system enumerates the device.

Start `minicom` with the following parameters:

```bash
sudo minicom -D /dev/ttyUSB0 -b 115200
```

After connecting, the LITEON FlexFi console should be available.

The expected console output and access procedure are shown below:

> Sync status/state: SYNCHRONIZED/SYNCHRONIZING

If the serial device is not `/dev/ttyUSB0`, check the available devices with:

```bash
ls /dev/ttyUSB*
```

Then replace `/dev/ttyUSB0` in the `minicom` command with the corresponding device.

---
This synchronization configuration is shared by both testbed configurations:

- [Without DPDK](../without-dpdk/)
- [With DPDK](../with-dpdk/)

The synchronization infrastructure remains the same regardless of whether OCUDU uses the standard Linux networking stack or DPDK.
