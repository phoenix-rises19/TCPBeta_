#include "message.h"
#include<string>
#include<cstdlib>
#include<cstring>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <arpa/inet.h>
#include <netinet/in.h>

namespace ethernet_packet {
    void make_eth_header(std::string &dst_, std::string &src_, std::vector<uint8_t> &buffer) {
        const char *start = dst_.c_str();

        for (int i = 0; i < 6; i++) {
            buffer.push_back(static_cast<uint8_t>(std::strtol(start, nullptr, 16)));
            start = std::strchr(start, '.');
            if (start) start++;
        }

        start = src_.c_str();
        for (int i = 0; i < 6; i++) {
            buffer.push_back(static_cast<uint8_t>(std::strtol(start, nullptr, 16)));
            start = std::strchr(start, '.');
            if (start) start++;
        }
        uint16_t type_ = (0x0800);
        buffer.push_back(type_ >> 8 & 0xFF);
        buffer.push_back(type_ & 0xFF);
    }

    void make_ip_header(uint32_t src_ip_, uint32_t dst_ip_, uint8_t ttl, std::string &payload,
                        std::vector<uint8_t> &buffer) {

        size_t ip_header_start = buffer.size();
        std::cout<<"starting point"<<ip_header_start<<std::endl;

        buffer.push_back(0x45); //v_ihl
        buffer.push_back(0x00); //dscp_ecn
        uint16_t total_len =
            sizeof(ethernet_packet::IPHeader) + sizeof(ethernet_packet::TCPHeader) + payload.size();
        buffer.push_back(static_cast<uint8_t>((total_len >> 8) & 0xFF));
        buffer.push_back(static_cast<uint8_t>(total_len & 0xFF));


        uint16_t identification = rand() % 65536;
        buffer.push_back(static_cast<uint8_t>(identification >> 8));
        buffer.push_back(static_cast<uint8_t>(identification & 0xFF));

        buffer.push_back(0x40);
        buffer.push_back(0x00);



        buffer.push_back(ttl);
        buffer.push_back(0x06);

        size_t checksum_pos = buffer.size();
        buffer.push_back(0x00);
        buffer.push_back(0x00);


        buffer.push_back(static_cast<uint8_t>((src_ip_ >> 24) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((src_ip_ >> 16) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((src_ip_ >> 8) & 0xFF));
        buffer.push_back(static_cast<uint8_t>(src_ip_ & 0xFF));


        buffer.push_back(static_cast<uint8_t>((dst_ip_ >> 24) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((dst_ip_ >> 16) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((dst_ip_ >> 8) & 0xFF));
        buffer.push_back(static_cast<uint8_t>(dst_ip_ & 0xFF));


        uint16_t checksum = calculate_ip_checksum(buffer, ip_header_start, checksum_pos);
        buffer[checksum_pos] = static_cast<uint8_t>((checksum >> 8) & 0xFF);
        buffer[checksum_pos + 1] = static_cast<uint8_t>(checksum & 0xFF);

    }

    void make_tcp_header(uint16_t src_port, uint16_t dst_port, std::vector<uint8_t> &buffer,uint8_t flag) {
        buffer.push_back(static_cast<uint8_t>((src_port >> 8) & 0xFF));
        buffer.push_back(static_cast<uint8_t>(src_port & 0xFF));

        buffer.push_back(static_cast<uint8_t>((dst_port >> 8) & 0xFF));
        buffer.push_back(static_cast<uint8_t>(dst_port & 0xFF));

        size_t seq_pos=buffer.size();
        uint32_t random_seq = static_cast<uint32_t>(rand());
        buffer.push_back(static_cast<uint8_t>((random_seq >> 24) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((random_seq >> 16) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((random_seq >>  8) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((random_seq      ) & 0xFF));

        size_t ack_pos=buffer.size();
        buffer.push_back(0x00);
        buffer.push_back(0x00);
        buffer.push_back(0x00);
        buffer.push_back(0x00);

        buffer.push_back(0x50);

        size_t flag_pos=buffer.size();
        buffer.push_back(flag);


        uint16_t window_size=0xFFFF;
        buffer.push_back(static_cast<uint8_t>((window_size >> 8) & 0xFF));
        buffer.push_back(static_cast<uint8_t>(window_size & 0xFF));

        size_t checksum_pos=buffer.size();
        buffer.push_back(0x00);
        buffer.push_back(0x00);

        buffer.push_back(0x00);
        buffer.push_back(0x00);
    }

    void print_packet(std::vector<uint8_t> &buffer) {
        if (buffer.size() < 14) {
            std::cerr << "Packet too small" << std::endl;
            return;
        }
        std::cout << "Ethernet Frame is:" << std::endl;
        std::cout << "Destination Mac Address: " << std::endl;
        for (size_t i = 0; i < 6; ++i) {
            std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
                    << static_cast<int>(buffer[i]);
            if (i < 5) std::cout << ":";
        }
        std::cout << std::dec << std::endl;

        std::cout << "Source Mac Address: " << std::endl;
        for (size_t i = 6; i < 12; ++i) {
            std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
                    << static_cast<int>(buffer[i]);
            if (i < 11) std::cout << ":";
        }
        std::cout << std::dec << std::endl;

        uint16_t etherType = (buffer[12] << 8) | buffer[13];
        std::cout << "EtherType: 0x" << std::hex << std::setw(4) << std::setfill('0')
                << etherType << std::hex << std::endl;

        std::cout<<"IP Header: "<<std::endl;
        for (size_t i = 14; i < 34; ++i) {
            std::cout << std::hex << std::uppercase << std::setw(2)
                      << std::setfill('0') << static_cast<int>(buffer[i]) << " ";
            if ((i - 14 + 1) % 4 == 0) std::cout << std::endl;  // Newline every 4 bytes for readability
        }
        std::cout<<std::endl;
    }

    uint16_t calculate_ip_checksum(std::vector<uint8_t> &buffer, size_t header_start, size_t checksum_pos) {
        // IP header is the first 20 bytes
        uint32_t sum = 0;

        // Process the IP header as 16-bit words
        for (size_t i = header_start; i < header_start+ 20; i += 2) {
            // Skip the checksum field itself
            if (i == checksum_pos) {
                continue;
            }

            uint16_t word = (static_cast<uint16_t>(buffer[i]) << 8) + buffer[i + 1];
            sum += word;
        }

        // Add carry
        while (sum >> 16) {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }

        // Take one's complement
        return static_cast<uint16_t>(~sum);
    }

    uint16_t calculate_tcp_checksum(std::vector<uint8_t> &buffer, size_t ip_header_start, size_t tcp_header_start, const std::string &payload) {
        uint32_t sum = 0;

        // Create pseudo header fields
        uint32_t src_ip = (static_cast<uint32_t>(buffer[ip_header_start + 12]) << 24) |
                          (static_cast<uint32_t>(buffer[ip_header_start + 13]) << 16) |
                          (static_cast<uint32_t>(buffer[ip_header_start + 14]) << 8) |
                          (static_cast<uint32_t>(buffer[ip_header_start + 15]));

        uint32_t dst_ip = (static_cast<uint32_t>(buffer[ip_header_start + 16]) << 24) |
                          (static_cast<uint32_t>(buffer[ip_header_start + 17]) << 16) |
                          (static_cast<uint32_t>(buffer[ip_header_start + 18]) << 8) |
                          (static_cast<uint32_t>(buffer[ip_header_start + 19]));

        uint8_t protocol = buffer[ip_header_start + 9]; // TCP protocol (6)
        uint16_t tcp_length = buffer.size() - tcp_header_start + payload.size();

        // Add pseudo-header fields to sum
        sum += (src_ip >> 16) & 0xFFFF; // High 16 bits of source IP
        sum += src_ip & 0xFFFF;         // Low 16 bits of source IP
        sum += (dst_ip >> 16) & 0xFFFF; // High 16 bits of destination IP
        sum += dst_ip & 0xFFFF;         // Low 16 bits of destination IP
        sum += protocol;                // Protocol (padded to 16 bits)
        sum += tcp_length;              // TCP length

        // Add TCP header to sum (excluding the checksum field)
        for (size_t i = tcp_header_start; i < buffer.size(); i += 2) {
            // Skip the checksum field
            if (i == tcp_header_start + 16) {
                continue;
            }

            uint16_t word;
            if (i + 1 < buffer.size()) {
                word = (static_cast<uint16_t>(buffer[i]) << 8) | buffer[i + 1];
            } else {
                word = static_cast<uint16_t>(buffer[i]) << 8; // Last byte if odd
            }
            sum += word;
        }

        // Add payload to sum
        for (size_t i = 0; i < payload.size(); i += 2) {
            uint16_t word;
            if (i + 1 < payload.size()) {
                word = (static_cast<uint16_t>(payload[i]) << 8) | payload[i + 1];
            } else {
                word = static_cast<uint16_t>(payload[i]) << 8; // Last byte if odd
            }
            sum += word;
        }

        // Add carry
        while (sum >> 16) {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }

        // Take one's complement
        return static_cast<uint16_t>(~sum);
    }
}


// revma::TCPHeader tcp;
// tcp.src_port_ = htons(12345);
// tcp.dst_port_ = htons(80);
// tcp.seq_ = htonl(0);
// tcp.ack_ = htonl(0);
// tcp.data_offset_reserved_ = (5 << 4);  // 5 * 4 = 20 bytes header
// tcp.flags_ = 0x02;  // SYN flag
// tcp.window_size_ = htons(8192);
// tcp.checksum_ = 0;  // Needs calculation
// tcp.urgent_ptr_ = 0;
// buffer.insert(buffer.end(),reinterpret_cast<uint8_t*>(&tcp),reinterpret_cast<uint8_t*>(&tcp)+sizeof(tcp));
