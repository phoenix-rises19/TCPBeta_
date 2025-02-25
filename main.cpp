#include "ethernet_packet/message.h"
#include"config.h"
#include<cstring>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <arpa/inet.h>
#include"tcp_stack/machine_state.h"
#include"tcp_stack/packet_transfer.hpp"

int main(){
    std::string filename = "config.json";
    ConfigData config;

    if (!read_config(filename, config)) {
        return 1;
    }

    std::cout << "Config Loaded:\n";
    std::cout << "Source MAC: " << config.src_mac << "\n";
    std::cout << "Destination MAC: " << config.dst_mac << "\n";
    std::cout << "Source IP: " << config.src_ip << "\n";
    std::cout << "Destination IP: " << config.dst_ip << "\n";
    std::cout << "Source Port: " << config.src_port << "\n";
    std::cout << "Destination Port: " << config.dst_port << "\n";
    std::cout << "TTL: " << (int)config.time_to_live << "\n";

    struct in_addr src_addr;
    inet_pton(AF_INET, config.src_ip.c_str(), &src_addr);
    uint32_t src_ip_bin = htonl(src_addr.s_addr);

    struct in_addr dst_addr;
    inet_pton(AF_INET, config.dst_ip.c_str(), &dst_addr);
    uint32_t dst_ip_bin = htonl(dst_addr.s_addr);

    std::unordered_map<tcp_stack::CustomKey, tcp_stack::ConnectionState, tcp_stack::ConnectionHash, tcp_stack::Equal> connections_map;

    tcp_stack::CustomKey key;
    key.srcIP_= src_ip_bin;
    key.srcPort_= config.src_port;
    key.dstIP_= dst_ip_bin;
    key.dstPort_= config.dst_port;

    if (connections_map.find(key) == connections_map.end()) {
        tcp_stack::ConnectionState st;
        connections_map[key]=tcp_stack::ready_connection(config.dst_mac, config.src_mac, key);

        std::string interface_name = "enp5s0";

        if (tcp_stack::send_raw_packet(connections_map[key].packet_, interface_name)) {
            std::cout << "Packet sent successfully!" << std::endl;
        } else {
            std::cerr << "Packet sending failed!" << std::endl;
        }

    }
    // std::vector<uint8_t> buffer;
    // std::string payload="Hello";
    // ethernet_packet::make_eth_header(config.dst_mac,config.src_mac,buffer);
    // ethernet_packet::make_ip_header(key.srcIP_,key.dstIP_,config.time_to_live,payload,buffer);
    // ethernet_packet::make_tcp_header(key.srcPort_,key.dstPort_,buffer);
    //
    //
    // ethernet_packet::print_packet(buffer);


    // auto it = connections_map.find(key);
    // if (it == connections_map.end()) {
    //     tcp_stack::ConnectionState st;
    //     st.initiator_state_ = tcp_stack::TCPState::SYN_SENT;
    //     st.responder_state_ = tcp_stack::TCPState::LISTENING;
    //     // store initial seq, ack, etc.
    //     connections_map[key] = st;
    // }

    return 0;
}

// ip fragment not needed
// tcp timestamp on off karne ka method
// source port change karne ke ability
// window scaling pramas exposed
// configrable ho thru API
// config chaye by this weekend
// when NSE sendsfragemtn
//TCP reordering, out of buffer--> thgius must be processed
// TTl can be put manually
//listner call back vitual function
// confidrable vs
//retransmision time to be configrable
// max retransission
//