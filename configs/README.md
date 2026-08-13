
# Common Configuration

This directory contains configuration procedures that are shared between the OCUDU deployments with and without DPDK.

The configurations in this directory must be applied in both deployment scenarios.

---

## Network Interface Configuration

Configure the MTU of the O-RAN Fronthaul interface:

```bash
sudo ip link set dev ens4f0np0 mtu 9000
```

Disable VLAN filtering and VLAN offloading:

```bash
sudo ethtool -K ens4f0np0 rx-vlan-filter off
sudo ethtool -K ens4f0np0 rx-vlan-stag-filter off
sudo ethtool -K ens4f0np0 rx-vlan-offload off
sudo ethtool -K ens4f0np0 tx-vlan-offload off
```

These settings are used in both the DPDK and non-DPDK configurations.

---

## XDP Configuration

The testbed uses XDP for PTP-related packet processing.

Attach the XDP program to the O-RAN Fronthaul interface:

```bash
sudo ip link set dev ens4f0np0 xdpgeneric obj xdp_ptp_pad.o sec xdp
```

This configuration is shared between the DPDK and non-DPDK deployments.

---

## OCUDU Performance Configuration

Before starting OCUDU, run the performance configuration script:

```bash
sudo ~/ocudu/scripts/ocudu_performance
```

This script applies the performance-related system configuration required by the OCUDU testbed.

The same configuration is used for both:

- OCUDU without DPDK
- OCUDU with DPDK

---
