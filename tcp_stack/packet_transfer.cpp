#include "packet_transfer.hpp"
#include<vector>
#include<string>
#include <netpacket/packet.h>
#include <iostream>
#include <cerrno>
#include <cstring>
#include<arpa/inet.h>
#include<net/if.h>
#include<sys/socket.h>
#include <net/ethernet.h>
#include <unistd.h>



namespace tcp_stack{
    bool send_raw_packet(std::vector<uint8_t>& packet, const std::string& interface_name) {
        // 1. Create a raw socket
        int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if (sock < 0) {
            std::cerr << "Failed to create raw socket: " << strerror(errno) << std::endl;
            return false;
        }

        // 2. Prepare sockaddr_ll structure
        struct sockaddr_ll socket_address;
        memset(&socket_address, 0, sizeof(socket_address));
        socket_address.sll_family = AF_PACKET;
        socket_address.sll_protocol = htons(ETH_P_ALL); // <-- Use ETH_P_ALL for raw frames

        // 3. Get interface index
        socket_address.sll_ifindex = if_nametoindex(interface_name.c_str());
        if (socket_address.sll_ifindex == 0) {
            std::cerr << "Failed to get interface index for " << interface_name << ": " << strerror(errno) << std::endl;
            close(sock);
            return false;
        }

        // 4. Ensure the packet is large enough for Ethernet
        if (packet.size() < 14) { // 14 bytes minimum for Ethernet frame
            std::cerr << "Error: Packet too small for Ethernet frame." << std::endl;
            close(sock);
            return false;
        }

        // 5. Set destination MAC address (first 6 bytes of the packet)
        memcpy(socket_address.sll_addr, packet.data(), 6);
        socket_address.sll_halen = 6;

        // 6. Send the packet
        ssize_t sent_bytes = sendto(sock, packet.data(), packet.size(), 0,
                                    (struct sockaddr*)&socket_address, sizeof(socket_address));

        if (sent_bytes < 0) {
            std::cerr << "Failed to send packet: " << strerror(errno) << std::endl;
            close(sock);
            return false;
        }

        std::cout << "Packet sent successfully. Bytes sent: " << sent_bytes << std::endl;

        // 7. Close the socket
        close(sock);
        return true;
    }
}

