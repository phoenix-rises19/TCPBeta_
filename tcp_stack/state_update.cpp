#include <cstdint>
#include<fstream>
#include<string>
#include<iostream>
#include <bits/ios_base.h>
#include "message.h"
#include "machine_state.h"
#include<arpa/inet.h>


namespace tcp_stack{
tcp_stack::ConnectionState updateCurrentState(const tcp_stack::ConnectionState &currentState, const tcp_stack::IPHeader &ipHeader,
                                          const tcp_stack::TCPHeader &tcpHeader,
                                          int flag, std::vector<uint8_t> &buffer) {
    tcp_stack::ConnectionState newState = currentState;

    bool fin = flag & 0x01;
    bool syn = flag & 0x02;
    bool rst = flag & 0x04;
    bool psh = flag & 0x08;
    bool ack = flag & 0x10;

    bool messageFromInitiator = (ntohl(ipHeader.src_) == currentState.initiatorIP_);
    tcp_stack::TCPState &initState = newState.initiator_state_;
    tcp_stack::TCPState &respState = newState.responder_state_;

    auto initSeq = newState.initiator_seq_;
    auto respSeq = newState.responder_seq_;
    auto initAck = newState.initiator_ack_;
    auto respAck = newState.responder_ack_;
    uint32_t next_sequence_expected_initiator = newState.next_sequence_expected_initiator_;
    uint32_t next_sequence_expected_responder = newState.next_sequence_expected_responder_;

    uint32_t packet_seq_num = ntohl(tcpHeader.seq_);
    uint32_t packet_ack_num = ntohl(tcpHeader.ack_);
    uint16_t payload_len = ntohs(ipHeader.len_) - ((ipHeader.v_ihl_ & 0X0F) * 4) - (
                               ((tcpHeader.data_offset_reserved_ >> 4) & 0X0F) * 4);

    if (messageFromInitiator) {
        if (initState == tcp_stack::TCPState::SYN_RECEIVED &&
            respState == tcp_stack::TCPState::SYN_SENT) {
            if (ack) {
                if (initSeq + 1 != packet_seq_num and packet_ack_num != respSeq + 1) {
                    std::cout << "Handshake Issues" << std::endl;
                } else {
                    newState.initiator_state_ = tcp_stack::TCPState::ESTABLISHED;
                    newState.responder_state_ = tcp_stack::TCPState::ESTABLISHED;
                    newState.initiator_seq_ = tcpHeader.seq_;
                    newState.initiator_seq_ = tcpHeader.ack_;
                    newState.next_sequence_expected_initiator_ = packet_ack_num;
                    newState.next_sequence_expected_responder_ = packet_seq_num;
                    std::cout << "TCP State: ESTABLISHED" << std::endl;
                }
            }
        } else if (initState == tcp_stack::TCPState::ESTABLISHED &&
                   respState == tcp_stack::TCPState::ESTABLISHED) {
            if ((psh and ack) or ack) {
                if (payload_len > 0) {
                    std::cout << "[Initiator -> Responder] Data Packet, Seq=" << packet_seq_num
                            << ", Ack=" << packet_ack_num << ", Payload Len=" << payload_len << " ";
                    if (packet_seq_num == next_sequence_expected_responder) {
                        std::cout << "In-Order" << std::endl;
                        newState.next_sequence_expected_responder_ += payload_len;
                        while (true) {
                            auto it = newState.out_of_order_packets_.find(std::make_pair(packet_seq_num, true));
                            if (it != newState.out_of_order_packets_.end()) {
                                newState.next_sequence_expected_responder_ += (it->second.size());
                                newState.out_of_order_packets_.erase(it);
                            } else {
                                break;
                            }
                        }
                    } else if (packet_seq_num < next_sequence_expected_responder) {
                        std::cout << "Retrans-mission" << std::endl;
                    } else if (packet_seq_num > next_sequence_expected_responder) {
                        std::cout << "Out-of-Order" << std::endl;
                        newState.out_of_order_packets_[std::make_pair(packet_seq_num, true)] = buffer;
                        std::cout << "[Initiator -> Responder] ACK Packet, Ack=" << packet_ack_num
                                << ", expecting next responder seq=" << next_sequence_expected_responder << std::endl;
                    }
                    newState.last_segment_sent_time_ = std::chrono::steady_clock::now();
                    newState.unacknowledged_seq_num_ = packet_seq_num + payload_len;
                    newState.waiting_for_ack_ = true;
                    newState.retransmission_count_=0;

                    std::cout << "[Initiator -> Responder] Data Packet Sent, Seq=" << packet_seq_num
                              << ", Ack=" << packet_ack_num << ", Payload Len=" << payload_len << ", Waiting for ACK" << std::endl;

                } else if (ack and payload_len == 0) {
                    std::cout << "[Initiator -> Responder] ACK Packet (No Data), Ack=" << packet_ack_num
                            << ", expecting next responder seq=" << next_sequence_expected_responder << std::endl;
                    if (currentState.waiting_for_ack_ and packet_ack_num >currentState.unacknowledged_seq_num_) {
                        newState.waiting_for_ack_ = false;
                        newState.retransmission_count_=0;
                        std::cout << "[Initiator -> Responder] ACK received for Seq=" << currentState.unacknowledged_seq_num_ << ", Stopping Retransmission Timer" << std::endl;
                    }else {
                        std::cout << "[Initiator -> Responder] ACK received, but not acknowledging our sent data or not waiting for ACK" << std::endl;
                    }
                }
            }
        }
    } else {
        if (initState == tcp_stack::TCPState::SYN_SENT &&
            respState == tcp_stack::TCPState::LISTENING) {
            if (syn && ack) {
                if (packet_ack_num != initSeq + 1) {
                    std::cout << "1st " << packet_seq_num << "2nd " << initSeq << std::endl;
                    std::cout << "Error is handshake" << std::endl;
                } else {
                    newState.initiator_state_ = tcp_stack::TCPState::SYN_RECEIVED;
                    newState.responder_state_ = tcp_stack::TCPState::SYN_SENT;
                    newState.responder_seq_ = packet_seq_num;
                    newState.responder_ack_ = packet_ack_num;
                    std::cout << "2nd Leg of Handshake Achieved" << std::endl;
                }
            }
        } else if (initState == tcp_stack::TCPState::ESTABLISHED &&
                   respState == tcp_stack::TCPState::ESTABLISHED) {
            if ((psh and ack) or ack) {
                if (payload_len > 0) {
                    std::cout << "[Responder -> Initiator] Data Packet, Seq=" << packet_seq_num
                            << ", Ack=" << packet_ack_num << ", Payload Len=" << payload_len;
                    if (packet_seq_num == next_sequence_expected_initiator) {
                        std::cout << "In-Order" << std::endl;
                        newState.next_sequence_expected_initiator_ += payload_len;
                        // now check here if this is present in the map.
                        while (true) {
                            auto it = newState.out_of_order_packets_.find(std::make_pair(packet_seq_num, false));
                            if (it != newState.out_of_order_packets_.end()) {
                                newState.next_sequence_expected_initiator_ += (it->second.size());
                                newState.out_of_order_packets_.erase(it);
                            } else {
                                break;
                            }
                        }
                    } else if (packet_seq_num < next_sequence_expected_initiator) {
                        std::cout << "Retransmission" << std::endl;
                    } else {
                        std::cout << "Out-of-Order" << std::endl;
                        newState.out_of_order_packets_[std::make_pair(packet_seq_num, false)] = buffer;
                        std::cout << "[Responder -> Initiator] Out-of-Order Packet, Ack=" << packet_ack_num
                                << ", expecting next initiator seq=" << next_sequence_expected_initiator << std::endl;
                    }
                    newState.last_segment_sent_time_ = std::chrono::steady_clock::now();
                    newState.unacknowledged_seq_num_ = packet_seq_num + payload_len;
                    newState.waiting_for_ack_ = true;
                    newState.retransmission_count_=0;

                    std::cout << "[Responder -> Initiator] Data Packet Sent, Seq=" << packet_seq_num
                              << ", Ack=" << packet_ack_num << ", Payload Len=" << payload_len << ", Waiting for ACK" << std::endl;
                } else if (ack && payload_len == 0) {
                    std::cout << "[Responder -> Initiator] ACK Packet (No Data), Ack=" << packet_ack_num
                            << ", expecting next initiator seq=" << next_sequence_expected_initiator << std::endl;
                    if (currentState.waiting_for_ack_ and packet_ack_num >currentState.unacknowledged_seq_num_) {
                        newState.waiting_for_ack_ = false;
                        newState.retransmission_count_=0;
                        std::cout << "[Responder -> Initiator] ACK received for Seq=" << currentState.unacknowledged_seq_num_ << ", Stopping Retransmission Timer" << std::endl;
                    }else {
                        std::cout << "[Responder -> Initiator] ACK received, but not acknowledging our sent data or not waiting for ACK" << std::endl;
                    }
                }
            }
        }
    }
    return newState;
}
}


