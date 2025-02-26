#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ethernet_packet {
    struct ethernet_header {
        uint8_t dest_[6];
        uint8_t src_[6];
        uint16_t type_;
    }__attribute__((packed));


    struct IPHeader {
        uint8_t v_ihl_; // 4 bytes each
        uint8_t dscp_ecn_; // 6 DSP and 2 ECN
        uint16_t len_;
        uint16_t idf_;
        uint16_t flags_frag_offset_; //3Flags and 13 offset
        uint8_t ttl_;
        uint8_t protocol_;
        uint16_t checksum_;
        uint32_t src_;
        uint32_t dst_;
    }__attribute((packed));

    struct TCPHeader {
        uint16_t src_port_;
        uint16_t dst_port_;
        uint32_t seq_;
        uint32_t ack_;
        uint8_t data_offset_reserved_; //mix here 4 each
        uint8_t flags_;
        uint16_t window_size_;
        uint16_t checksum_;
        uint16_t urgent_ptr_;
    }__attribute__((packed));

    void make_eth_header(std::string &dst_mac_, std::string &src_mac_, std::vector<uint8_t> &buffer);

    void make_ip_header(uint32_t src_ip_, uint32_t dst_ip_, uint8_t ttl, std::string &payload,
                        std::vector<uint8_t> &buffer);

    void make_tcp_header(uint16_t src_port, uint16_t dst_port, std::vector<uint8_t> &buffer, uint8_t flag);

    void print_packet(std::vector<uint8_t> &buffer);

    uint16_t calculate_ip_checksum(std::vector<uint8_t> &buffer, size_t header_start, size_t checksum_pos);

    uint16_t calculate_tcp_checksum(std::vector<uint8_t> &buffer, size_t ip_header_start, size_t tcp_header_start,
                                    const std::string &payload);
}
