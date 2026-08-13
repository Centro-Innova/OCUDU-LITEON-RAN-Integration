# OCUDU Without DPDK

This directory contains the installation, build, configuration, and execution procedure for running **OCUDU without DPDK**.

This configuration uses the standard Linux networking stack and serves as the baseline setup for the testbed.

> **Important:** Before starting this tutorial, make sure that the following components have already been configured:
>
> - [Open5GS Core](../core/)
> - [Synchronization (PTP/GPS)](../synchronization/)
> - [Common Configuration](../configs/)
>
> These configurations are shared between the DPDK and non-DPDK setups.

---

## OCUDU Installation

Clone the OCUDU repository:

```bash
git clone https://gitlab.com/ocudu/ocudu.git
```

Enter the repository:

```bash
cd ocudu
```

Checkout the `dev` branch:

```bash
git checkout dev
```

Update the repository:

```bash
git pull
```

---

## Dependencies

Install the required dependencies:

```bash
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    git \
    gcc \
    g++ \
    python3 \
    python3-pip \
    libyaml-cpp-dev
```

---

## Build OCUDU

Create the build directory:

```bash
mkdir -p build
cd build
```

Configure the project:

```bash
cmake ..
```

Build OCUDU:

```bash
make -j$(nproc)
```

After the build completes, the gNB application will be available in:

```text
build/apps/gnb/
```

---

## System Configuration

Before starting OCUDU, complete the configurations described in the `config/` directory.

The following configurations must be completed:

- Network interface configuration;
- MTU;
- VLAN configuration;
- XDP;
- OCUDU performance configuration;
- CPU configuration.

For the CPU configuration, follow the procedure documented in:

[Common Configuration](../configs/)

---

## OCUDU Configuration

The configuration file used in this testbed is:

```text
configs/gnb_ru_liteon_tdd_n78_100mhz.yml
```

Make sure that the OCUDU configuration matches the configuration used by the **Open5GS Core** and the **LITEON FlexFi O-RU**.

The main network parameters, such as PLMN, TAC, IP addresses, and radio configuration, must be configured according to the deployed network.

---

## Start the gNB

Enter the gNB directory:

```bash
cd ~/ocudu/build/apps/gnb
```

Start the gNB:

```bash
sudo taskset -c 0-15 ./gnb -c ../../../configs/gnb_ru_liteon_tdd_n78_100mhz.yml 
```

The CPU range used by `taskset` must be adapted to the CPU configuration of the server.

The procedure for configuring CPU affinity and the other performance-related settings is documented in the `config/` directory.

---

## gNB Logs

The gNB logs can be monitored using:

```bash
tail -f /tmp/gnb.log
```

---

## Open5GS Verification

Before starting the gNB, verify that the Open5GS Core is running:

```bash
sudo systemctl status open5gs-amfd
```

To monitor the AMF logs:

```bash
sudo journalctl -u open5gs-amfd -f
```

---

## LITEON FlexFi O-RU

The LITEON FlexFi can be accessed through its serial console.

Install `minicom`:

```bash
sudo apt update
sudo apt install minicom
```

Connect to the RU:

```bash
sudo minicom -D /dev/ttyUSB0 -b 115200
```

The serial device may be different depending on the USB port used by the RU.

To identify available serial devices:

```bash
ls /dev/ttyUSB*
```

---

## Network Validation

After starting OCUDU and the LITEON FlexFi, verify that the UE can connect to the 5G network.

The following components must be operational:

```text
Open5GS
   |
   v
OCUDU
   |
   v
LITEON FlexFi
   |
   v
5G UE
```

Verify that the UE successfully registers with the network and receives an IP address.

---

## Internet Connectivity

If Internet connectivity is required, configure IP forwarding and NAT on the server.

Enable IP forwarding:

```bash
sudo sysctl -w net.ipv4.ip_forward=1
```

Make the configuration persistent:

```bash
echo "net.ipv4.ip_forward=1" | sudo tee -a /etc/sysctl.conf
sudo sysctl -p
```

Configure NAT:

```bash
sudo iptables -t nat -A POSTROUTING -s 10.45.0.0/16 ! -o ogstun -j MASQUERADE
```

---

## Official OCUDU Documentation

For additional information, refer to the official OCUDU documentation:

https://docs.ocudu.org/tutorials/cots_ue/
