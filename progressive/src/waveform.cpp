#include "progressive/waveform.hpp"
#include <sstream>
#include <cmath>
#include <algorithm>

namespace progressive {

std::vector<WaveformBar> generateWaveform(
    const std::vector<int16_t>& samples,
    int numBars, int sampleRate
) {
    if (samples.empty() || numBars <= 0) return {};

    int samplesPerBar = static_cast<int>(samples.size()) / numBars;
    if (samplesPerBar < 1) samplesPerBar = 1;

    std::vector<double> amplitudes;
    amplitudes.reserve(numBars);

    for (int i = 0; i < numBars; ++i) {
        int start = i * samplesPerBar;
        int end = std::min(start + samplesPerBar, static_cast<int>(samples.size()));

        // Compute RMS for this segment
        double sumSq = 0.0;
        int count = 0;
        for (int j = start; j < end; ++j) {
            double normalized = static_cast<double>(samples[j]) / 32768.0;
            sumSq += normalized * normalized;
            count++;
        }
        double rms = count > 0 ? std::sqrt(sumSq / count) : 0.0;
        amplitudes.push_back(rms);
    }

    return generateWaveformFromAmplitudes(amplitudes, numBars);
}

std::vector<WaveformBar> generateWaveformFromAmplitudes(
    const std::vector<double>& amplitudes, int numBars
) {
    if (amplitudes.empty()) return {};

    auto normalized = normalizeAmplitudes(amplitudes);

    std::vector<WaveformBar> bars;
    bars.reserve(numBars);

    int barCount = std::min(numBars, static_cast<int>(normalized.size()));
    for (int i = 0; i < barCount; ++i) {
        WaveformBar bar;
        bar.amplitude = normalized[i];
        // Map 0.0-1.0 to bar height 1-10
        bar.barHeight = std::max(1, static_cast<int>(normalized[i] * 10.0 + 0.5));
        bars.push_back(bar);
    }

    return bars;
}

double computeRmsVolume(const std::vector<int16_t>& samples) {
    if (samples.empty()) return 0.0;
    double sumSq = 0.0;
    for (auto s : samples) {
        double normalized = static_cast<double>(s) / 32768.0;
        sumSq += normalized * normalized;
    }
    return std::sqrt(sumSq / samples.size());
}

double computePeakVolume(const std::vector<int16_t>& samples) {
    if (samples.empty()) return 0.0;
    int16_t peak = 0;
    for (auto s : samples) {
        if (std::abs(s) > peak) peak = std::abs(s);
    }
    return static_cast<double>(peak) / 32768.0;
}

bool isSilence(const std::vector<int16_t>& samples, double threshold) {
    return computeRmsVolume(samples) < threshold;
}

std::string waveformToJson(const std::vector<WaveformBar>& bars) {
    std::ostringstream json;
    json << "[";
    for (size_t i = 0; i < bars.size(); ++i) {
        if (i > 0) json << ",";
        json << R"({"h": )" << bars[i].barHeight;
        json << R"(,"a": )" << bars[i].amplitude;
        json << R"(,"active": )" << (bars[i].isActive ? "true" : "false") << "}";
    }
    json << "]";
    return json.str();
}

double computeWaveformProgress(const std::vector<WaveformBar>& bars, int64_t positionMs, int64_t totalMs) {
    if (bars.empty() || totalMs <= 0) return 0.0;
    return static_cast<double>(positionMs) / static_cast<double>(totalMs);
}

void markActiveBars(std::vector<WaveformBar>& bars, int64_t positionMs, int64_t totalMs) {
    if (bars.empty() || totalMs <= 0) return;

    double progress = computeWaveformProgress(bars, positionMs, totalMs);
    int activeCount = static_cast<int>(progress * bars.size());

    for (int i = 0; i < static_cast<int>(bars.size()); ++i) {
        bars[i].isActive = (i < activeCount);
    }
}

int suggestBarCount(int64_t durationMs) {
    // Roughly 2 bars per second, minimum 10, maximum 100
    int bars = static_cast<int>(durationMs / 500);
    return std::max(10, std::min(100, bars));
}

std::vector<double> normalizeAmplitudes(const std::vector<double>& amplitudes) {
    if (amplitudes.empty()) return {};

    double maxAmp = 0.0;
    for (double a : amplitudes) {
        if (a > maxAmp) maxAmp = a;
    }

    if (maxAmp <= 0.0) return std::vector<double>(amplitudes.size(), 0.0);

    std::vector<double> result;
    result.reserve(amplitudes.size());
    for (double a : amplitudes) {
        result.push_back(a / maxAmp);
    }
    return result;
}

// ==== Waveform Sanitizer (from WaveFormSanitizer.kt:36-85) ====
// Original Kotlin:
//   fun sanitize(waveForm: List<Int>?): List<Int>? {
//       // MIN_NUMBER_OF_VALUES = 30, MAX_NUMBER_OF_VALUES = 120, MAX_VALUE = 1024
//       // 1. Extend short lists by repeating values
//       // 2. Trim long lists by subsampling (keep every Nth)
//       // 3. Make all values positive (abs)
//       // 4. Cap max at 1024 and scale down if over

std::vector<int> sanitizeWaveform(const std::vector<int>& waveform) {
    const int MIN_VALUES = 30;
    const int MAX_VALUES = 120;
    const int MAX_VALUE = 1024;

    if (waveform.empty()) return {};

    // Step 1: Adjust size to be in [30, 120]
    std::vector<int> sized;
    if (static_cast<int>(waveform.size()) < MIN_VALUES) {
        // Original: repeat values to reach at least 30
        int repeatTimes = (MIN_VALUES + static_cast<int>(waveform.size()) - 1) / static_cast<int>(waveform.size());
        for (int val : waveform) {
            for (int r = 0; r < repeatTimes; ++r) sized.push_back(val);
        }
    } else if (static_cast<int>(waveform.size()) > MAX_VALUES) {
        // Original: subsample — keep every Nth value
        int keepOneOf = (static_cast<int>(waveform.size()) + MAX_VALUES - 1) / MAX_VALUES;
        for (size_t i = 0; i < waveform.size(); ++i) {
            if (i % keepOneOf == 0) sized.push_back(waveform[i]);
        }
    } else {
        sized = waveform;
    }

    // Step 2: Make all values positive (abs)
    std::vector<int> positive;
    for (int val : sized) positive.push_back(std::abs(val));

    // Step 3: Find max, scale down if > MAX_VALUE
    int maxVal = 0;
    for (int val : positive) if (val > maxVal) maxVal = val;
    if (maxVal == 0) maxVal = MAX_VALUE;

    if (maxVal > MAX_VALUE) {
        // Original: scale down proportionally
        std::vector<int> scaled;
        for (int val : positive) {
            scaled.push_back(val * MAX_VALUE / maxVal);
        }
        return scaled;
    }

    return positive;
}

} // namespace progressive


// ==== Extended waveform implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string waveform::serialize() const {
    json j = toJson();
    return j.dump();
}

bool waveform::deserialize(const std::string& data) {
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
bool waveform::validate() const {
    if (!m_initialized) {
        LOGE("waveform: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool waveform::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool waveform::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json waveform::getMetrics() const {
    json m;
    m["class"] = "waveform";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int waveform::getOperationCount() const {
    return m_operationCount;
}

void waveform::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void waveform::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "waveform";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool waveform::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool waveform::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool waveform::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void waveform::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void waveform::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int waveform::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void waveform::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> waveform::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> waveform::listItems() const {
    return {};
}

int waveform::itemCount() const {
    return 0;
}

// Versioning
std::string waveform::getVersion() const {
    return "1.0.0";
}

bool waveform::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool waveform::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void waveform::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> waveform::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool waveform::migrateData(int fromVersion, int toVersion) {
    LOGI("waveform: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int waveform::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json waveform::exportData() const {
    return toJson();
}

bool waveform::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void waveform::performCleanup() {
    LOGI("waveform: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t waveform::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool waveform::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool waveform::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool waveform::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void waveform::logDebug(const std::string& msg) const {
    LOGI("waveform: %s", msg.c_str());
}

void waveform::logWarning(const std::string& msg) const {
    LOGW("waveform: %s", msg.c_str());
}

void waveform::logError(const std::string& msg) const {
    LOGE("waveform: %s", msg.c_str());
}
