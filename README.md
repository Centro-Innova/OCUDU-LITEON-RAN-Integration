# OCUDU + LITEON FlexFi 5G O-RAN Testbed

This repository provides a step-by-step guide for deploying a **private 5G network** using **OCUDU**, **Open5GS**, and a **LITEON FlexFi O-RU**.

The main objective is to document the installation and configuration of the complete 5G testbed, including the 5G Core, synchronization, O-RAN Radio Unit, OCUDU, and the required networking configuration.

The repository contains **two deployment scenarios**:

- **Without DPDK** — the baseline configuration using the standard Linux networking stack.
- **With DPDK** — a configuration using DPDK for packet processing.

The two scenarios share the same **Open5GS Core** and **synchronization configuration**, so these components are documented separately and reused by both setups.

---

## System Requirements

The testbed was developed and tested using:

```text
Operating System:
Ubuntu Server 24.04.4 LTS

Kernel:
6.8.1-1056-realtime

Radio Unit (O-RU):
LITEON FlexFi
```

---

# Repository Structure

```text
OCUDU-LITEON-5G-O-RAN-Testbed/
│
├── README.md
│
├── core/
│   └── Open5GS configuration
│
├── synchronization/
│   └── PTP and GPS synchronization
│
├── without-dpdk/
│   └── OCUDU setup without DPDK
│
├── with-dpdk/
│   └── OCUDU setup with DPDK
│
├── configs/
│   └── Configuration files

```

## `core/`

Contains the configuration and installation procedure for the **Open5GS 5G Core**.

This configuration is shared by both deployment scenarios:

```text
                  ┌──────────────┐
                  │   Open5GS    │
                  │     Core     │
                  └──────┬───────┘
                         │
              ┌──────────┴──────────┐
              │                     │
              ▼                     ▼
       Without DPDK             With DPDK
          OCUDU                   OCUDU
```

The Core does not change between the two scenarios.

---

## `synchronization/`

Contains the configuration required for synchronization between the RAN components.

This includes the **PTP/GPS synchronization setup** used by the testbed.

The same synchronization configuration is used for both:

- OCUDU without DPDK;
- OCUDU with DPDK.

---

## `without-dpdk/`

Contains the complete procedure for deploying the testbed **without DPDK**.

This is the **baseline setup** and should be completed and validated before moving to the DPDK configuration.

The main architecture is:

```text
LITEON FlexFi
      │
      │ O-RAN Fronthaul
      ▼
   Intel NIC
      │
      ▼
 Linux Networking
      │
      ▼
    OCUDU
      │
      ▼
   Open5GS
```

---

## `with-dpdk/`

Contains the procedure for deploying the testbed using **DPDK**.

This setup is based on the working `without-dpdk` environment and introduces the DPDK-based packet-processing path.

```text
LITEON FlexFi
      │
      │ O-RAN Fronthaul
      ▼
   Intel NIC
      │
      ▼
    DPDK
      │
      ▼
    OCUDU
      │
      ▼
   Open5GS
```

---

# Testbed Overview

The complete testbed consists of:

```text
                         ┌───────────────┐
                         │    5G UE      │
                         └───────┬───────┘
                                 │
                                5G
                                 │
                         ┌───────▼───────┐
                         │ LITEON FlexFi │
                         │     O-RU      │
                         └───────┬───────┘
                                 │
                           O-RAN Fronthaul
                                 │
                         ┌───────▼───────┐
                         │     OCUDU     │
                         │     CU + DU   │
                         └───────┬───────┘
                                 │
                              N2 / N3
                                 │
                         ┌───────▼───────┐
                         │    Open5GS    │
                         │    5G Core    │
                         └───────────────┘
```

Synchronization is provided separately through the PTP/GPS infrastructure documented in the `synchronization/` directory.

---

# Technologies

The testbed is based on:

- **OCUDU**
- **Open5GS**
- **LITEON FlexFi O-RU**
- **O-RAN**
- **Ubuntu Server**
- **Linux realtime kernel**
- **PTP / GPS synchronization**
- **DPDK** for the optimized setup

---
## Adding UEs and Subscriber Configuration

After configuring the Open5GS Core, subscriber information must be added to the Open5GS database for each UE/SIM that will connect to the network.

The subscriber configuration includes information such as:

- IMSI
- Authentication keys
- OP/OPc
- AMF
- APN/DNN
- Subscriber profile

For the detailed procedure to register subscribers and configure the required parameters, refer to the official Open5GS documentation and the OCUDU COTS UE tutorial.

The Open5GS documentation provides information about registering subscriber data in the Core, while the OCUDU tutorial provides a detailed procedure for configuring a commercial UE, including SIM and APN configuration. :contentReference[oaicite:0]{index=0}

### References

- [Open5GS Quickstart](https://open5gs.org/open5gs/docs/guide/01-quickstart/)
- [OCUDU COTS UE Tutorial](https://docs.ocudu.org/tutorials/cots_ue/)

# Documentation

- [`core/`](core/) — Open5GS installation and configuration
- [`synchronization/`](synchronization/) — PTP/GPS configuration
- [`without-dpdk/`](without-dpdk/) — baseline OCUDU setup
- [`with-dpdk/`](with-dpdk/) — DPDK-based OCUDU setup

