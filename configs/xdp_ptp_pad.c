#include <linux/bpf.h>
#include <linux/if_ether.h>
#include <bpf/bpf_endian.h>
#include <bpf/bpf_helpers.h>

#define ETH_P_8021Q  0x8100
#define ETH_P_8021AD 0x88a8
#define ETH_P_1588   0x88f7

SEC("xdp")
int trim_ptp_delay_req(struct xdp_md *ctx)
{
	void *data = (void *)(long)ctx->data;
	void *data_end = (void *)(long)ctx->data_end;
	unsigned char *frame = data;
	unsigned char *ptp;
	__u16 proto;
	__u16 ptp_len;
	__u32 frame_len;
	__u32 l2_len = ETH_HLEN;
	__u32 wanted_len;
	int trim;

	/* Ethernet header. */
	if (frame + ETH_HLEN > (unsigned char *)data_end)
		return XDP_PASS;

	proto = ((__u16)frame[12] << 8) | frame[13];

	/* Handle one 802.1Q or 802.1ad VLAN header. */
	if (proto == ETH_P_8021Q || proto == ETH_P_8021AD) {
		if (frame + ETH_HLEN + 4 > (unsigned char *)data_end)
			return XDP_PASS;

		proto = ((__u16)frame[16] << 8) | frame[17];
		l2_len += 4;
	}

	if (proto != ETH_P_1588)
		return XDP_PASS;

	ptp = frame + l2_len;

	/* Need bytes through domainNumber. */
	if (ptp + 5 > (unsigned char *)data_end)
		return XDP_PASS;

	/* messageType = Delay_Req. */
	if ((ptp[0] & 0x0f) != 1)
		return XDP_PASS;

	/* majorVersionPTP = 2. */
	if ((ptp[1] & 0x0f) != 2)
		return XDP_PASS;

	/* Restrict this experiment to your domain. */
	if (ptp[4] != 24)
		return XDP_PASS;

	ptp_len = ((__u16)ptp[2] << 8) | ptp[3];

	/* Restrict the experiment to the observed Delay_Req format. */
	if (ptp_len != 44)
		return XDP_PASS;

	frame_len = (__u32)((unsigned char *)data_end - frame);
	wanted_len = l2_len + ptp_len;

	if (wanted_len >= frame_len)
		return XDP_PASS;

	trim = (int)wanted_len - (int)frame_len;

	if (bpf_xdp_adjust_tail(ctx, trim) < 0)
		return XDP_ABORTED;

	return XDP_PASS;
}

char LICENSE[] SEC("license") = "GPL";