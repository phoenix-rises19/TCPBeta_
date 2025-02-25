#include<cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace tcp_stack {
    struct CustomKey {
        uint32_t srcIP_;
        uint16_t srcPort_;
        uint32_t dstIP_;
        uint16_t dstPort_;
    }__attribute__((packed));


    struct ConnectionHash {
        std::size_t operator()(const CustomKey &k) const {
            std::size_t hash_src = std::hash<uint32_t>()(k.srcIP_);
            std::size_t port_hash_src = std::hash<uint16_t>()(k.srcPort_);
            hash_src ^= port_hash_src + 0x9e3779bf4a7c15ULL + (hash_src << 6) + (hash_src >> 2);

            std::size_t hash_dst = std::hash<uint32_t>()(k.dstIP_);
            std::size_t port_hash_dst = std::hash<uint16_t>()(k.dstPort_);
            hash_dst ^= port_hash_dst + 0x9e3779bf4a7c15ULL + (hash_dst << 6) + (hash_dst >> 2);

            if (hash_dst < hash_src) {
                std::swap(hash_src, hash_dst);
            }

            std::size_t combined = hash_src;
            combined ^= hash_dst + 0x9e3779bf4a7c15ULL + (combined << 6) + (combined >> 2);
            return combined;
        }
    } __attribute__((packed));

    struct Equal {
        bool operator()(const CustomKey &k1, const CustomKey &k2) const {
            return ((k1.srcIP_ == k2.srcIP_) &&
                    (k1.dstIP_ == k2.dstIP_) &&
                    (k1.srcPort_ == k2.srcPort_) &&
                    (k1.dstPort_ == k2.dstPort_))
                   ||
                   ((k1.srcIP_ == k2.dstIP_) &&
                    (k1.dstIP_ == k2.srcIP_) &&
                    (k1.srcPort_ == k2.dstPort_) &&
                    (k1.dstPort_ == k2.srcPort_));
        }
    } __attribute__((packed));

    enum TCPState {
        LISTENING,
        SYN_SENT,
        SYN_RECEIVED,
        ESTABLISHED,
        FIN_WAITING,
        FIN_WAITING_ACK,
        CLOSE_WAITING,
        CLOSING,
        LAST_ACK,
        TIME_WAIT,
        CLOSED,
    }__attribute__((packed));

    struct ConnectionState {
        // TCPState initiator_state_;
        // TCPState responder_state_;
        // uint32_t initiatorIP_;
        // uint32_t initiator_seq_;
        // uint32_t responder_seq_;
        // uint32_t initiator_ack_;
        // uint32_t responder_ack_;
        // uint32_t next_sequence_expected_initiator_;
        // uint32_t next_sequence_expected_responder_;
        // std::map<std::pair<uint32_t, bool>, std::vector<uint8_t> > out_of_order_packets_;
        std::vector<uint8_t> packet_;
    }__attribute__((packed));

    ConnectionState ready_connection(std::string dst_mac, std::string src_mac,tcp_stack::CustomKey& key);
}
