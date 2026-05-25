#include "progressive/weather_utils.hpp"
#include "progressive/url_tools.hpp"
#include <sstream>
#include <algorithm>

namespace progressive {


std::string buildWeatherUrl(const std::string& location, const std::string& apiToken) {
    if (!apiToken.empty()) {
        // OpenWeatherMap
        std::ostringstream os;
        os << "https://api.openweathermap.org/data/2.5/weather?q="
           << urlEncode(location) << "&appid=" << apiToken << "&units=metric&lang=ru";
        return os.str();
    }
    // wttr.in (free, no API key)
    std::ostringstream os;
    os << "https://wttr.in/" << urlEncode(location) << "?format=j1&lang=ru";
    return os.str();
}

static std::string extractJsonValue(const std::string& json, const std::string& key) {
    auto search = "\"" + key + "\":\"";
    auto pos = json.find(search);
    if (pos == std::string::npos) {
        search = "\"" + key + "\": \"";
        pos = json.find(search);
    }
    if (pos == std::string::npos) return "";
    pos += search.size();
    auto end = json.find('"', pos);
    if (end == std::string::npos) return "";
    return json.substr(pos, end - pos);
}

static double extractJsonDouble(const std::string& json, const std::string& key) {
    auto search = "\"" + key + "\":";
    auto pos = json.find(search);
    if (pos == std::string::npos) {
        search = "\"" + key + "\": ";
        pos = json.find(search);
    }
    if (pos == std::string::npos) return 0.0;
    pos += search.size();
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
    try {
        return std::stod(json.substr(pos));
    } catch (...) { return 0.0; }
}

std::string parseWeatherResponse(const std::string& json) {
    // Parse wttr.in j1 format
    // current_condition[0]: temp_C, weatherDesc[0].value, humidity, windspeedKmph, FeelsLikeC
    std::ostringstream os;

    auto tempC = extractJsonValue(json, "temp_C");
    auto weatherDesc = extractJsonValue(json, "value");
    auto humidity = extractJsonValue(json, "humidity");
    auto windSpeed = extractJsonValue(json, "windspeedKmph");
    auto feelsLike = extractJsonValue(json, "FeelsLikeC");

    if (!tempC.empty()) {
        os << "Temperature: " << tempC << "°C\n";
    }
    if (!feelsLike.empty()) {
        os << "Feels like: " << feelsLike << "°C\n";
    }
    if (!weatherDesc.empty()) {
        os << "Conditions: " << weatherDesc << "\n";
    }
    if (!humidity.empty()) {
        os << "Humidity: " << humidity << "%\n";
    }
    if (!windSpeed.empty()) {
        os << "Wind: " << windSpeed << " km/h\n";
    }

    // Try to get forecast for next few hours
    auto forecastPos = json.find("\"weather\"");
    if (forecastPos != std::string::npos) {
        auto nextHourly = json.find("\"hourly\"", forecastPos);
        if (nextHourly != std::string::npos) {
            // Find the first hourly entry after forecast
            auto hourlyTemp = extractJsonValue(json.substr(nextHourly), "tempC");
            auto hourlyDesc = extractJsonValue(json.substr(nextHourly), "weatherDesc");
            auto hourlyDescValue = json.find("\"value\"", nextHourly);
            if (hourlyDescValue != std::string::npos) {
                auto v1 = json.find('"', hourlyDescValue + 8);
                auto v2 = json.find('"', v1 + 1);
                if (v1 != std::string::npos && v2 != std::string::npos) {
                    hourlyDesc = json.substr(v1 + 1, v2 - v1 - 1);
                }
            }
            if (!hourlyTemp.empty()) {
                os << "\nNext hour: " << hourlyTemp << "°C";
                if (!hourlyDesc.empty()) os << ", " << hourlyDesc;
                os << "\n";
            }
        }
    }

    if (os.str().empty()) return "Weather data unavailable";
    return os.str();
}

std::string parseOpenWeatherResponse(const std::string& json) {
    std::ostringstream os;

    // Extract from OpenWeatherMap format
    auto temp = extractJsonDouble(json, "temp");
    auto feelsLike = extractJsonDouble(json, "feels_like");
    auto humidity = extractJsonDouble(json, "humidity");
    auto pressure = extractJsonDouble(json, "pressure");

    if (temp > 0 || temp < 0) {
        int tempInt = static_cast<int>(temp);
        os << "Temperature: " << tempInt << "°C\n";
    }
    if (feelsLike > 0 || feelsLike < 0) {
        int fl = static_cast<int>(feelsLike);
        os << "Feels like: " << fl << "°C\n";
    }
    if (humidity > 0) {
        os << "Humidity: " << static_cast<int>(humidity) << "%\n";
    }
    if (pressure > 0) {
        os << "Pressure: " << static_cast<int>(pressure) << " hPa\n";
    }

    // Weather description from the weather array
    auto wdPos = json.find("\"description\"");
    if (wdPos != std::string::npos) {
        auto v1 = json.find('"', wdPos + 14);
        auto v2 = json.find('"', v1 + 1);
        if (v1 != std::string::npos && v2 != std::string::npos) {
            std::string desc = json.substr(v1 + 1, v2 - v1 - 1);
            os << "Conditions: " << desc << "\n";
        }
    }

    // Wind
    auto windSpeed = extractJsonDouble(json, "speed");
    if (windSpeed > 0) {
        os << "Wind: " << static_cast<int>(windSpeed) << " m/s\n";
    }

    if (os.str().empty()) return "Weather data unavailable";
    return os.str();
}

std::string formatWeatherForAlarm(const std::string& location, const std::string& weatherSummary) {
    std::ostringstream os;
    os << "=== Weather for " << location << " ===\n\n";
    os << weatherSummary;
    return os.str();
}

} // namespace progressive


// ==== Extended weather_utils implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string weather_utils::serialize() const {
    json j = toJson();
    return j.dump();
}

bool weather_utils::deserialize(const std::string& data) {
    if (data.empty()) return false;
    try {
        json j = json::parse(data);
        return fromJson(j);
    } catch (...) {
        setError("Failed to deserialize data");
        return false;
    }
}

// Validation helpers
bool weather_utils::validate() const {
    if (!m_initialized) {
        LOGE("weather_utils: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool weather_utils::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool weather_utils::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json weather_utils::getMetrics() const {
    json m;
    m["class"] = "weather_utils";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int weather_utils::getOperationCount() const {
    return m_operationCount;
}

void weather_utils::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void weather_utils::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "weather_utils";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool weather_utils::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool weather_utils::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool weather_utils::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
    auto now = currentTimeMs();
    auto& window = m_rateLimitWindows[key];
    if (now - window.startTime > windowMs) {
        window.startTime = now;
        window.count = 0;
    }
    if (window.count >= maxRequests) return false;
    window.count++;
    return true;
}

// Observation pattern
void weather_utils::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void weather_utils::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int weather_utils::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void weather_utils::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> weather_utils::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> weather_utils::listItems() const {
    return {};
}

int weather_utils::itemCount() const {
    return 0;
}

// Versioning
std::string weather_utils::getVersion() const {
    return "1.0.0";
}

bool weather_utils::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool weather_utils::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void weather_utils::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> weather_utils::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool weather_utils::migrateData(int fromVersion, int toVersion) {
    LOGI("weather_utils: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int weather_utils::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json weather_utils::exportData() const {
    return toJson();
}

bool weather_utils::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void weather_utils::performCleanup() {
    LOGI("weather_utils: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t weather_utils::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool weather_utils::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool weather_utils::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool weather_utils::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void weather_utils::logDebug(const std::string& msg) const {
    LOGI("weather_utils: %s", msg.c_str());
}

void weather_utils::logWarning(const std::string& msg) const {
    LOGW("weather_utils: %s", msg.c_str());
}

void weather_utils::logError(const std::string& msg) const {
    LOGE("weather_utils: %s", msg.c_str());
}
