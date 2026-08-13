# OCUDU With DPDK

This directory contains the installation, build, configuration, and execution procedure for running **OCUDU with DPDK**.

This setup is based on the baseline OCUDU testbed configuration, with the additional DPDK and SR-IOV configuration required for packet processing through a Virtual Function (VF).

> **Important:** Before starting this tutorial, complete the following configurations:
>
> - [Open5GS Core](../core/)
> - [Synchronization (PTP/GPS)](../synchronization/)
> - [System and Hardware Configuration](../configs/)
>
> The CPU isolation, hugepages, realtime kernel, common network configuration, Open5GS Core, and PTP/GPS synchronization are shared between the DPDK and non-DPDK deployments.

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

OCUDU requires a Linux system with realtime kernel support.

The CPU, hugepages, and realtime configuration used in this testbed are documented in:

[System and Hardware Configuration](../configs/)

---

## DPDK

## Build OCUDU With DPDK

Enter the OCUDU repository:

```bash
cd ~/ocudu
```

Create the build directory:

```bash
mkdir -p build
cd build
```

Configure OCUDU with DPDK enabled:

```bash
cmake ../ -DENABLE_DPDK=ON
```

The CMake output should indicate that DPDK was found.

Build OCUDU:

```bash
make -j$(nproc)
```

After the build completes, the gNB binary will be available at:

```text
~/ocudu/build/apps/gnb/gnb
```
---

## DPDK and SR-IOV Configuration

The DPDK deployment uses SR-IOV to create a **Virtual Function (VF)** from the Intel Fronthaul NIC.

The physical interface used in this testbed is:

```text
ens4f0np0
```

Create one VF:

```bash
sudo echo 1 | sudo tee /sys/class/net/ens4f0np0/device/sriov_numvfs
```

Verify that the VF was created:

```bash
ip link show ens4f0np0
```

---

## VFIO

Load the `vfio-pci` driver:

```bash
sudo modprobe vfio-pci
```

Bind the VF to `vfio-pci`:

```bash
sudo dpdk-devbind.py --bind=vfio-pci 0000:ac:01.0
```

> **Note:** The PCI address `0000:ac:01.0` is the VF used in this testbed. On another system, the PCI address may be different.

Verify the DPDK device binding:

```bash
sudo dpdk-devbind.py --status
```

The VF should appear under the devices using a DPDK-compatible driver.

---

## VF Configuration

Configure the VF from the physical function:

```bash
sudo ip link set ens4f0np0 vf 0 vlan 0
```

Enable trust mode:

```bash
sudo ip link set ens4f0np0 vf 0 trust on
```

Disable spoof checking:

```bash
sudo ip link set ens4f0np0 vf 0 spoofchk off
```

Verify the configuration:

```bash
ip -d link show ens4f0np0
```

---

## VF MAC Address

> **Important:** The testbed does not use a network switch. The LITEON FlexFi O-RU is connected directly to the Intel NIC.
>
> Because the Fronthaul is using a Virtual Function, the MAC address configured on the LITEON FlexFi and the MAC address configured in the OCUDU gNB configuration must correspond to the **VF MAC address**.

Do not use the physical function (PF) MAC address for the OCUDU configuration.

Check the interface and VF information:

```bash
ip link show ens4f0np0
```

The required configuration is:

```text
LITEON FlexFi O-RU
        |
        | O-RAN Fronthaul
        v
    VF MAC Address
        |
        v
OCUDU gNB Configuration
```

The same VF MAC address must therefore be configured on both sides:

```text
LITEON FlexFi RU MAC = VF MAC = OCUDU DU MAC
```

Before starting OCUDU:

1. Identify the VF MAC address.
2. Configure this MAC address on the LITEON FlexFi.
3. Configure the same MAC address in the DPDK gNB configuration file.

---

## CPU Configuration

The CPU configuration is shared with the non-DPDK deployment.

Before starting the gNB, follow the CPU and hugepages configuration documented in:

[System and Hardware Configuration](../configs/)

The CPU affinity used when starting the gNB must correspond to the CPUs configured for OCUDU.

---

## Start OCUDU

Enter the gNB directory:

```bash
cd ~/ocudu/build/apps/gnb
```

Start the gNB using the DPDK configuration:

```bash
sudo taskset -c 0-15 ./gnb -c ../../../configs/gnb_ru_liteon_tdd_n78_100mhz.yml
```

---

## Pre-Execution Validation

Before starting the gNB, verify the VFIO binding:

```bash
sudo dpdk-devbind.py --status
```

Verify the hugepages:

```bash
grep -i huge /proc/meminfo
```

Verify that the DPDK VF is available and correctly bound before starting OCUDU.

---

## gNB Logs

Monitor the OCUDU gNB logs:

```bash
tail -f /tmp/gnb.log
```

During startup, verify that OCUDU successfully initializes:

- DPDK;
- the VF;
- Open Fronthaul;
- the LITEON FlexFi;
- the gNB;
- the connection to the Open5GS Core.

---

## Open5GS Core

The Open5GS Core is the same Core used by the non-DPDK deployment.

Verify the AMF:

```bash
sudo systemctl status open5gs-amfd
```

Monitor the AMF logs:

```bash
sudo journalctl -u open5gs-amfd -f
```

No DPDK-specific configuration is required in Open5GS.

---

## LITEON FlexFi O-RU

The LITEON FlexFi can be accessed through its serial console.

Install `minicom`:

```bash
sudo apt update
sudo apt install minicom
```

Access the RU:

```bash
sudo minicom -D /dev/ttyUSB0 -b 115200
```

The serial device may be different depending on the USB port used.

Check the available serial devices:

```bash
ls /dev/ttyUSB*
```

Verify the RU network configuration and confirm that the MAC address corresponds to the VF MAC address configured in OCUDU.

---

## Adding UEs and SIM Cards

After the Core and gNB are operational, subscriber information must be added to the Open5GS database for each UE/SIM that will connect to the network.

The subscriber configuration includes parameters such as:

- IMSI;
- authentication keys;
- OP/OPc;
- AMF;
- DNN/APN;
- subscriber profile.

For the detailed procedure for registering subscribers and configuring the SIM/APN, refer to the official Open5GS and OCUDU documentation.

### Open5GS Subscriber Configuration

The official Open5GS Quickstart contains the procedure for configuring the Core and subscriber database:

https://open5gs.org/open5gs/docs/guide/01-quickstart/

### OCUDU COTS UE Tutorial

The OCUDU COTS UE tutorial provides a detailed procedure for configuring a commercial UE, including SIM and APN configuration:

https://docs.ocudu.org/tutorials/cots_ue/

---

## DPDK Deployment Overview

The packet-processing path used in this testbed is:

```text
LITEON FlexFi O-RU
        |
        | O-RAN Fronthaul
        v
    Intel NIC PF
        |
        v
    Virtual Function
        |
        | VFIO-PCI
        v
       DPDK
        |
        v
      OCUDU
        |
        v
     Open5GS
```

The Open5GS Core, PTP/GPS synchronization, CPU configuration, hugepages, and common network configuration are shared with the non-DPDK deployment.

The DPDK-specific part consists of the DPDK-enabled OCUDU build, SR-IOV Virtual Function, VFIO binding, VF configuration, and DPDK-specific gNB configuration.

---

## Official Documentation

For additional information about OCUDU and its deployment procedures, refer to the official documentation:

https://docs.ocudu.org/

OCUDU documentation:

https://docs.ocudu.org/user_manual/installation/

OCUDU tutorials:

https://docs.ocudu.org/tutorials/

COTS UE tutorial:

https://docs.ocudu.org/tutorials/cots_ue/
