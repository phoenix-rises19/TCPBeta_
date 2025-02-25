#include "machine_state.h"

#include <cstring>
#include <iostream>
#include <message.h>
#include <arpa/inet.h>

#include "packet_transfer.hpp"

namespace tcp_stack {
    tcp_stack::ConnectionState ready_connection(std::string dst_mac, std::string src_mac, tcp_stack::CustomKey &key) {
        tcp_stack::ConnectionState state;

        // Initialize buffer for this connection
        std::vector<uint8_t> &buffer = state.packet_;

        // Prepare Ethernet header
        ethernet_packet::make_eth_header(dst_mac, src_mac, buffer);

        // Prepare IP header without payload (we'll add it later when sending)
        std::string empty_payload = "";
        ethernet_packet::make_ip_header(key.srcIP_, key.dstIP_, static_cast<uint8_t>(64), empty_payload, buffer);
        ethernet_packet::print_packet(buffer);
        //
        // // Prepare TCP header with initial values
        ethernet_packet::make_tcp_header(key.srcPort_, key.dstPort_, buffer,0x02);

        // Initialize connection state
        state.initiator_state_ = tcp_stack::TCPState::CLOSED;
        state.responder_state_ = tcp_stack::TCPState::CLOSED;

        std::string interface_name = "enp5s0";
        if (tcp_stack::send_raw_packet(buffer, interface_name)) {
            std::cout << "Packet sent successfully!" << std::endl;
        } else {
            std::cerr << "Packet sending failed!" << std::endl;
        }

        // Store the connection state
        return state;
    }

    ConnectionState establish_connection(std::string dst_mac, std::string src_mac,tcp_stack::ConnectionState st) {
        // update seq number in the packet
        // ack number ke liye you wouldned the previous packet.
    }

}
