struct ConfigData {
    std::string src_mac;
    std::string dst_mac;
    std::string src_ip;
    std::string dst_ip;
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t time_to_live;
}__attribute((packed));

bool read_config(const std::string& filename, ConfigData &cfg);
