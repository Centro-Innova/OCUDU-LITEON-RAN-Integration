# Open5GS 5G Core

This directory contains the configuration and installation procedure for the **Open5GS 5G Core** used in the testbed.

The Open5GS Core is deployed **bare-metal**, directly on the Ubuntu Server host. No container or Docker installation is required for the Core.

## Operating System

The Open5GS Core was deployed using **Ubuntu Server**.

## Installation

Ubuntu makes it easy to install Open5GS using the official Open5GS PPA.

Add the Open5GS repository:

```bash
sudo add-apt-repository ppa:open5gs/latest
```

Update the package list:

```bash
sudo apt update
```

Install Open5GS:

```bash
sudo apt install open5gs
```

After the installation, the Open5GS services will be available through `systemd`.

## Configuration

After installing Open5GS, the configuration files must be adjusted according to the requirements of the private 5G network.

The main parameters that need to be configured include:

- MCC (Mobile Country Code)
- MNC (Mobile Network Code)
- TAC (Tracking Area Code)
- PLMN
- Network interfaces and IP addresses
- UPF configuration
- DNN
- Subscriber configuration

The exact values for MCC, MNC, TAC, DNN, IP addresses, and other network parameters depend on the network being deployed.

These values must be configured consistently across the entire 5G network, including the **Open5GS Core, OCUDU, and UE/SIM configuration**.

The same Open5GS Core configuration is used for both OCUDU deployment scenarios:

- [Without DPDK](../without-dpdk/)
- [With DPDK](../with-dpdk/)

## Service Verification

After installation and configuration, the Open5GS services can be checked using `systemctl`.

For example:

```bash
sudo systemctl status open5gs-amfd
```

The Open5GS AMF logs can be monitored using:

```bash
sudo journalctl -u open5gs-amfd -f
```

Other Open5GS services can be checked in the same way.

## Official Open5GS Documentation

For additional information about Open5GS installation and configuration, refer to the official Open5GS documentation:

https://open5gs.org/open5gs/docs/guide/01-quickstart/
