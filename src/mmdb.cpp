#include "mmdb.hpp"

namespace wireana {
namespace mmdb {
fs::path executable_dir = wireana::utils::get_executable_path().parent_path();

Geolite2CityDB geolite2db_city_ipv4 = Geolite2CityDB(
    executable_dir / "assets" / "geolite2-city-ipv4.mmdb");
Geolite2CityDB geolite2db_city_ipv6 = Geolite2CityDB(
    executable_dir / "assets" / "geolite2-city-ipv6.mmdb");
Geolite2ASNDB geolite2db_asn_ipv4 = Geolite2ASNDB(
    executable_dir / "assets" / "geolite2-asn-ipv4.mmdb");
Geolite2ASNDB geolite2db_asn_ipv6 = Geolite2ASNDB(
    executable_dir / "assets" / "geolite2-asn-ipv6.mmdb");

MMDB::MMDB(fs::path path) {
    spdlog::info("Loading MMDB: {}", path.string());
    int status = MMDB_open(path.string().data(), MMDB_MODE_MMAP, &mmdb);
    if (status != MMDB_SUCCESS) {
        throw std::runtime_error(std::string(MMDB_strerror(status)));
    }
}

std::optional<MMDB_entry_data_list_s*> MMDB::lookup_entry(
    const std::string& ip) {
    int gai_error, mmdb_error;
    MMDB_lookup_result_s result =
        MMDB_lookup_string(&mmdb, ip.data(), &gai_error, &mmdb_error);
    if (gai_error) {
#ifdef _WIN32
        throw std::runtime_error(gai_strerrorA(gai_error));
#else
        throw std::runtime_error(gai_strerror(gai_error));
#endif
    }
    if (mmdb_error != MMDB_SUCCESS) {
        throw std::runtime_error(MMDB_strerror(mmdb_error));
    }
    MMDB_entry_data_list_s* entry_data_list = nullptr;

    if (result.found_entry) {
        int status = MMDB_get_entry_data_list(&result.entry, &entry_data_list);
        if (MMDB_SUCCESS != status) {
            throw std::runtime_error(std::string(MMDB_strerror(status)));
        }
        return std::make_optional(entry_data_list);
    } else {
        // No data
        return std::nullopt;
    }
}

MMDB::~MMDB() { MMDB_close(&mmdb); }

Geolite2CityDB::Geolite2CityDB(fs::path path) : MMDB(path) {}
std::optional<Geolite2CityData> Geolite2CityDB::lookup_city(
    const std::string& ip) {
    Geolite2CityData result;
    auto entry_data_list = lookup_entry(ip);
    if (!entry_data_list.has_value()) {
        return std::nullopt;
    }
    MMDB_entry_data_list_s* entry_data_list_ptr = entry_data_list.value();
    uint16_t num_item = entry_data_list_ptr->entry_data.data_size;
    entry_data_list_ptr = entry_data_list_ptr->next;
    for (int i = 0; i < num_item; i++) {
        if (!entry_data_list_ptr)
            throw std::runtime_error("Invalid entry data list");
        if (entry_data_list_ptr->entry_data.type !=
            MMDB_DATA_TYPE_UTF8_STRING) {
            throw std::runtime_error("Invalid entry data map");
        }
        auto key_size = entry_data_list_ptr->entry_data.data_size;
        const std::string key =
            std::string(entry_data_list_ptr->entry_data.utf8_string, key_size);
        entry_data_list_ptr = entry_data_list_ptr->next;
        auto may_string_len = entry_data_list_ptr->entry_data.data_size;
        if (key == "city") {
            result.city = std::string(
                entry_data_list_ptr->entry_data.utf8_string, may_string_len);
        } else if (key == "country_code") {
            result.country_code = std::string(
                entry_data_list_ptr->entry_data.utf8_string, may_string_len);
        } else if (key == "postcode") {
            result.postcode = std::string(
                entry_data_list_ptr->entry_data.utf8_string, may_string_len);
        } else if (key == "state1") {
            result.state1 = std::string(
                entry_data_list_ptr->entry_data.utf8_string, may_string_len);
        } else if (key == "state2") {
            result.state2 = std::string(
                entry_data_list_ptr->entry_data.utf8_string, may_string_len);
        } else if (key == "timezone") {
            result.timezone = std::string(
                entry_data_list_ptr->entry_data.utf8_string, may_string_len);
        } else if (key == "latitude") {
            result.latitude = entry_data_list_ptr->entry_data.float_value;
        } else if (key == "longitude") {
            result.longitude = entry_data_list_ptr->entry_data.float_value;
        }
        entry_data_list_ptr = entry_data_list_ptr->next;
    }
    MMDB_free_entry_data_list(entry_data_list_ptr);
    return std::make_optional(result);
}

Geolite2ASNDB::Geolite2ASNDB(fs::path path) : MMDB(path) {}
std::optional<Geolite2ASNData> Geolite2ASNDB::lookup_asn(
    const std::string& ip) {
    Geolite2ASNData result;
    auto entry_data_list = lookup_entry(ip);
    if (!entry_data_list.has_value()) {
        return std::nullopt;
    }
    MMDB_entry_data_list_s* entry_data_list_ptr = entry_data_list.value();
    uint16_t num_item = entry_data_list_ptr->entry_data.data_size;
    entry_data_list_ptr = entry_data_list_ptr->next;
    for (int i = 0; i < num_item; i++) {
        if (!entry_data_list_ptr)
            throw std::runtime_error("Invalid entry data list");
        if (entry_data_list_ptr->entry_data.type !=
            MMDB_DATA_TYPE_UTF8_STRING) {
            throw std::runtime_error("Invalid entry data map");
        }
        auto key_size = entry_data_list_ptr->entry_data.data_size;
        const std::string key =
            std::string(entry_data_list_ptr->entry_data.utf8_string, key_size);
        entry_data_list_ptr = entry_data_list_ptr->next;
        auto may_string_len = entry_data_list_ptr->entry_data.data_size;
        if (key == "autonomous_system_number") {
            result.asn = entry_data_list_ptr->entry_data.uint32;
        } else if (key == "autonomous_system_organization") {
            result.asn_org = std::string(
                entry_data_list_ptr->entry_data.utf8_string, may_string_len);
        }
        entry_data_list_ptr = entry_data_list_ptr->next;
    }
    MMDB_free_entry_data_list(entry_data_list_ptr);
    return std::make_optional(result);
}

}  // namespace mmdb
}  // namespace wireana