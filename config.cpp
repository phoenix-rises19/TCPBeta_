#include <iostream>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include"config.h"


bool read_config(const std::string& filename, ConfigData &cfg) {
    using boost::property_tree::ptree;

    ptree pt;

    try {
        // Read JSON file into property tree
        boost::property_tree::read_json(filename, pt);

        // Extract values
        cfg.src_mac      = pt.get<std::string>("src_mac");
        cfg.dst_mac      = pt.get<std::string>("dst_mac");
        cfg.src_ip       = pt.get<std::string>("src_ip");
        cfg.dst_ip       = pt.get<std::string>("dst_ip");
        cfg.src_port     = pt.get<uint16_t>("src_port");
        cfg.dst_port     = pt.get<uint16_t>("dst_port");
        cfg.time_to_live = pt.get<uint8_t>("time_to_live");
    } catch (const std::exception &e) {
        std::cerr << "Error parsing config: " << e.what() << std::endl;
        return false;
    }

    return true;
}