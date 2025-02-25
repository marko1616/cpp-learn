#ifndef WIREANA_MMDB_HPP
#define WIREANA_MMDB_HPP

#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>

#include "maxminddb.h"
#include "spdlog/spdlog.h"
#include "utils.hpp"

namespace fs = std::filesystem;

namespace wireana {
namespace mmdb {

struct Geolite2CityData {
    std::string city = "";
    std::string country_code = "";
    float latitude = 0.0f;
    float longitude = 0.0f;
    std::string postcode = "";
    std::string state1 = "";
    std::string state2 = "";
    std::string timezone = "";
};

struct Geolite2ASNData {
    uint32_t asn = 0;
    std::string asn_org = "";
};

class MMDB {
   private:
    MMDB_s mmdb;

   public:
    explicit MMDB(fs::path path);
    std::optional<MMDB_entry_data_list_s*> lookup_entry(const std::string& ip);
    ~MMDB();
};

class Geolite2CityDB : public MMDB {
   public:
    explicit Geolite2CityDB(fs::path path);
    std::optional<Geolite2CityData> lookup_city(const std::string& ip);
};

class Geolite2ASNDB : public MMDB {
   public:
    explicit Geolite2ASNDB(fs::path path);
    std::optional<Geolite2ASNData> lookup_asn(const std::string& ip);
};

extern Geolite2CityDB geolite2db_city_ipv4;
extern Geolite2CityDB geolite2db_city_ipv6;
extern Geolite2ASNDB geolite2db_asn_ipv4;
extern Geolite2ASNDB geolite2db_asn_ipv6;
}  // namespace mmdb
}  // namespace wireana

#endif  // WIREANA_MMDB_HPP