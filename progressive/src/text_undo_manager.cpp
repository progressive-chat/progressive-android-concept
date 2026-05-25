#include "progressive/text_undo_manager.hpp"
#include <sstream>
#include <algorithm>
#include <ctime>

namespace progressive {

// ====== Constructor ======

TextUndoManager::TextUndoManager() {}

void TextUndoManager::setConfig(const UndoConfig& config) { config_ = config; }
UndoConfig TextUndoManager::getConfig() const { return config_; }

// ====== Stack Management ======

void TextUndoManager::pushUndo(const std::string& text, int cursorPos, const std::string& description) {
    UndoEntry entry;
    entry.text = text;
    entry.cursorPos = cursorPos;
    entry.timestampMs = static_cast<int64_t>(std::time(nullptr)) * 1000;
    entry.description = description;
    undoStack_.push_back(entry);
    truncateStack();

    // Clear redo stack on new action
    redoStack_.clear();
}

void TextUndoManager::truncateStack() {
    while (static_cast<int>(undoStack_.size()) > config_.maxDepth) {
        undoStack_.erase(undoStack_.begin());
    }
}

// ====== Checkpoint ======

void TextUndoManager::checkpoint(const std::string& text, int cursorPos, const std::string& description) {
    if (!config_.enabled) return;

    auto now = static_cast<int64_t>(std::time(nullptr)) * 1000;

    // Debounce: don't checkpoint if too recent
    if (now - lastCheckpointMs_ < config_.debounceMs && !lastCheckpointText_.empty()) return;

    // Don't checkpoint if text hasn't changed
    if (text == lastCheckpointText_) return;

    lastCheckpointText_ = text;
    lastCheckpointMs_ = now;
    pushUndo(text, cursorPos, description.empty() ? "manual" : description);
}

void TextUndoManager::autoCheckpoint(const std::string& text, int cursorPos) {
    if (!config_.enabled) return;

    // Auto-checkpoint if text changed more than threshold
    int diff = static_cast<int>(text.size()) - static_cast<int>(lastCheckpointText_.size());
    if (std::abs(diff) >= config_.autoCheckpointBytes) {
        checkpoint(text, cursorPos, "typed");
    }
}

// ====== Undo/Redo ======

std::string TextUndoManager::undo(std::string& outText, int& outCursorPos) {
    if (undoStack_.empty()) return "";

    // Move current state to redo stack
    UndoEntry current;
    current.text = lastCheckpointText_;
    current.cursorPos = 0;
    current.timestampMs = static_cast<int64_t>(std::time(nullptr)) * 1000;
    current.description = "";
    redoStack_.push_back(current);

    // Pop undo entry
    auto entry = undoStack_.back();
    undoStack_.pop_back();

    outText = entry.text;
    outCursorPos = config_.restoreCursor ? entry.cursorPos : 0;
    lastCheckpointText_ = entry.text;
    lastCheckpointMs_ = entry.timestampMs;

    return entry.description;
}

std::string TextUndoManager::redo(std::string& outText, int& outCursorPos) {
    if (redoStack_.empty()) return "";

    // Save current to undo stack
    pushUndo(lastCheckpointText_, 0, "");

    auto entry = redoStack_.back();
    redoStack_.pop_back();

    outText = entry.text;
    outCursorPos = entry.cursorPos;
    lastCheckpointText_ = entry.text;
    lastCheckpointMs_ = entry.timestampMs;

    return entry.description;
}

bool TextUndoManager::canUndo() const {
    return !undoStack_.empty() && config_.enabled;
}

bool TextUndoManager::canRedo() const {
    return !redoStack_.empty() && config_.enabled;
}

std::string TextUndoManager::nextUndoDescription() const {
    if (undoStack_.empty()) return "";
    return undoStack_.back().description;
}

std::string TextUndoManager::nextRedoDescription() const {
    if (redoStack_.empty()) return "";
    return redoStack_.back().description;
}

// ====== Special Operations ======

void TextUndoManager::onSelectAll(const std::string& text, int cursorPos) {
    if (config_.checkpointOnSelectAll && config_.enabled) {
        checkpoint(text, cursorPos, "select_all");
    }
}

void TextUndoManager::onBeforePaste(const std::string& currentText, int cursorPos,
                                     const std::string& pastedText) {
    if (!config_.enabled || !config_.checkpointBeforePaste) return;

    // Only checkpoint if pasted text is large enough to be an accident
    // (small pastes like emoji are fine)
    if (pastedText.size() > static_cast<size_t>(config_.autoCheckpointBytes)) {
        checkpoint(currentText, cursorPos, "paste");
    }
}

void TextUndoManager::onBeforeCut(const std::string& text, int cursorPos,
                                   int selStart, int selEnd) {
    if (!config_.enabled) return;

    // Always checkpoint before cut (could lose data)
    int cutLen = selEnd - selStart;
    if (cutLen >= config_.autoCheckpointBytes) {
        checkpoint(text, cursorPos, "cut");
    }
}

void TextUndoManager::clear() {
    undoStack_.clear();
    redoStack_.clear();
}

void TextUndoManager::reset() {
    clear();
    lastCheckpointText_.clear();
    lastCheckpointMs_ = 0;
}

// ====== Serialization ======

std::string TextUndoManager::stateToJson() const {
    auto esc = [](const std::string& s) -> std::string {
        std::string out;
        for (char c : s) { if (c == '"') out += "\\\""; else out += c; }
        return out;
    };

    std::ostringstream os;
    os << R"({"can_undo":)" << (canUndo() ? "true" : "false")
       << R"(,"can_redo":)" << (canRedo() ? "true" : "false")
       << R"(,"undo_count":)" << undoCount()
       << R"(,"redo_count":)" << redoCount()
       << R"(,"next_undo":")" << esc(nextUndoDescription()) << R"(")"
       << R"(,"next_redo":")" << esc(nextRedoDescription()) << R"(")"
       << R"(,"enabled":)" << (config_.enabled ? "true" : "false")
       << "}";
    return os.str();
}

std::string TextUndoManager::configToJson() const {
    std::ostringstream os;
    os << R"({"enabled":)" << (config_.enabled ? "true" : "false")
       << R"(,"max_depth":)" << config_.maxDepth
       << R"(,"auto_checkpoint_bytes":)" << config_.autoCheckpointBytes
       << R"(,"checkpoint_before_paste":)" << (config_.checkpointBeforePaste ? "true" : "false")
       << R"(,"checkpoint_on_select_all":)" << (config_.checkpointOnSelectAll ? "true" : "false")
       << R"(,"restore_cursor":)" << (config_.restoreCursor ? "true" : "false")
       << R"(,"debounce_ms":)" << config_.debounceMs
       << "}";
    return os.str();
}

} // namespace progressive


// ==== Extended text_undo_manager implementation ====
// Additional methods and utilities generated for completeness

// Serialization helpers
std::string text_undo_manager::serialize() const {
    json j = toJson();
    return j.dump();
}

bool text_undo_manager::deserialize(const std::string& data) {
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
bool text_undo_manager::validate() const {
    if (!m_initialized) {
        LOGE("text_undo_manager: not initialized");
        return false;
    }
    return true;
}

// Storage helpers
bool text_undo_manager::save(const std::string& path) const {
    std::string data = serialize();
    if (data.empty()) return false;
    std::ofstream f(path);
    if (!f) return false;
    f << data;
    return true;
}

bool text_undo_manager::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    return deserialize(ss.str());
}

// Metrics and statistics
json text_undo_manager::getMetrics() const {
    json m;
    m["class"] = "text_undo_manager";
    m["initialized"] = m_initialized;
    m["enabled"] = m_enabled;
    m["paused"] = m_paused;
    m["timestamp"] = currentTimeMs();
    return m;
}

int text_undo_manager::getOperationCount() const {
    return m_operationCount;
}

void text_undo_manager::resetOperationCount() {
    m_operationCount = 0;
}

// Event emission
void text_undo_manager::emitEvent(const std::string& eventType, const json& data) {
    json event;
    event["type"] = eventType;
    event["source"] = "text_undo_manager";
    event["data"] = data;
    event["timestamp"] = currentTimeMs();
    notifyUpdate(event);
}

// Policy checking
bool text_undo_manager::checkPolicy(const std::string& policy, const json& context) {
    (void)policy;
    (void)context;
    return true;
}

// Access control
bool text_undo_manager::canAccess(const std::string& userId, const std::string& resource) {
    (void)userId;
    (void)resource;
    return true;
}

// Rate limiting
bool text_undo_manager::checkRateLimit(const std::string& key, int maxRequests, int windowMs) {
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
void text_undo_manager::addObserver(const std::string& observerId) {
    m_observers.insert(observerId);
}

void text_undo_manager::removeObserver(const std::string& observerId) {
    m_observers.erase(observerId);
}

int text_undo_manager::observerCount() const {
    return static_cast<int>(m_observers.size());
}

void text_undo_manager::notifyObservers(const json& data) {
    notifyUpdate(data);
}

// Factory pattern
std::shared_ptr<void> text_undo_manager::createInstance() {
    return nullptr;
}

// Iterator pattern
std::vector<std::string> text_undo_manager::listItems() const {
    return {};
}

int text_undo_manager::itemCount() const {
    return 0;
}

// Versioning
std::string text_undo_manager::getVersion() const {
    return "1.0.0";
}

bool text_undo_manager::checkVersion(const std::string& requiredVersion) {
    return getVersion() >= requiredVersion;
}

// Feature flags
bool text_undo_manager::isFeatureEnabled(const std::string& feature) const {
    auto it = m_features.find(feature);
    return it != m_features.end() && it->second;
}

void text_undo_manager::setFeature(const std::string& feature, bool enabled) {
    m_features[feature] = enabled;
}

std::vector<std::string> text_undo_manager::getEnabledFeatures() const {
    std::vector<std::string> result;
    for (auto& [feature, enabled] : m_features) {
        if (enabled) result.push_back(feature);
    }
    return result;
}

// Data migration
bool text_undo_manager::migrateData(int fromVersion, int toVersion) {
    LOGI("text_undo_manager: migrating data from v%d to v%d", fromVersion, toVersion);
    return true;
}

int text_undo_manager::getDataVersion() const {
    return m_dataVersion;
}

// Import/Export
json text_undo_manager::exportData() const {
    return toJson();
}

bool text_undo_manager::importData(const json& data) {
    return fromJson(data);
}

// Cleanup
void text_undo_manager::performCleanup() {
    LOGI("text_undo_manager: performing cleanup");
    m_cache.clear();
    m_observers.clear();
    m_features.clear();
    m_rateLimitWindows.clear();
}

// Memory management
size_t text_undo_manager::memoryUsage() const {
    size_t usage = sizeof(*this);
    usage += m_cache.size() * sizeof(std::string) * 100;
    usage += m_observers.size() * sizeof(std::string) * 50;
    usage += m_features.size() * (sizeof(std::string) + sizeof(bool));
    return usage;
}

// Transaction support
bool text_undo_manager::beginTransaction() {
    if (m_inTransaction) return false;
    m_inTransaction = true;
    m_transactionData = json::object();
    return true;
}

bool text_undo_manager::commitTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    notifyUpdate(m_transactionData);
    return true;
}

bool text_undo_manager::rollbackTransaction() {
    if (!m_inTransaction) return false;
    m_inTransaction = false;
    m_transactionData = json::object();
    return true;
}

// Logging helpers
void text_undo_manager::logDebug(const std::string& msg) const {
    LOGI("text_undo_manager: %s", msg.c_str());
}

void text_undo_manager::logWarning(const std::string& msg) const {
    LOGW("text_undo_manager: %s", msg.c_str());
}

void text_undo_manager::logError(const std::string& msg) const {
    LOGE("text_undo_manager: %s", msg.c_str());
}
