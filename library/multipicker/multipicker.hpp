#pragma once

/**
 * ===========================================================================
 * MULTIPICKER — Multi-file picker with MIME type handling
 * ===========================================================================
 *
 * A multi-file/media picker engine supporting:
 *   - Image, video, audio, file, contact, and camera-capture picker types
 *   - MIME type filtering (whitelist / blacklist)
 *   - Multi-selection with configurable min/max count
 *   - File size limits and dimension constraints (for images)
 *   - Capture mode for camera (photo / video)
 *   - Selection callbacks with progress reporting
 *   - File metadata extraction (name, size, MIME, dimensions, duration)
 *
 * This is a direct port of the Kotlin MultiPicker library used in the
 * Android Matrix client. The engine is UI-agnostic and designed to
 * interface with any native file picker / media-store backend.
 */

#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace multipicker {

// ---------- Enums ----------

enum class PickerType {
    Image,
    Video,
    Audio,
    File,
    Contact,
    Camera
};

enum class CameraMode {
    Photo,
    Video
};

enum class SelectionMode {
    Single,
    Multiple
};

enum class SortOrder {
    NameAsc,
    NameDesc,
    DateAsc,
    DateDesc,
    SizeAsc,
    SizeDesc
};

// ---------- Structs ----------

struct SizeConstraint {
    int min_width = 0;
    int min_height = 0;
    int max_width = 0;
    int max_height = 0;
};

struct PickerConfig {
    PickerType type = PickerType::Image;
    SelectionMode selection_mode = SelectionMode::Multiple;
    int min_selection = 0;
    int max_selection = 10;
    int64_t max_file_size = 100LL * 1024 * 1024; // 100 MB default
    std::vector<std::string> mime_whitelist;       // empty = all allowed
    std::vector<std::string> mime_blacklist;
    std::vector<std::string> allowed_extensions;
    SizeConstraint size_constraint;
    CameraMode camera_mode = CameraMode::Photo;
    bool show_hidden_files = false;
    SortOrder sort_order = SortOrder::DateDesc;
};

struct PickedFile {
    std::string id;
    std::filesystem::path uri;
    std::string display_name;
    std::string mime_type;
    int64_t file_size = 0;
    int width = 0;
    int height = 0;
    int64_t duration_ms = 0;  // audio/video
    int64_t date_added = 0;   // epoch seconds
    bool is_selected = false;
};

struct PickerState {
    std::vector<PickedFile> files;
    std::vector<int> selected_indices;
    bool loading = false;
    std::string error_message;
};

// ---------- Callbacks ----------

using OnSelectionChanged = std::function<void(const std::vector<PickedFile>& selected)>;
using OnFilesLoaded     = std::function<void(const std::vector<PickedFile>& files)>;
using OnError           = std::function<void(const std::string& message)>;
using OnCaptureComplete = std::function<void(const PickedFile& capture)>;
using OnProgress        = std::function<void(int current, int total)>;

// ---------- MIME Utilities ----------

struct MimeCategory {
    static constexpr const char* IMAGE = "image/";
    static constexpr const char* VIDEO = "video/";
    static constexpr const char* AUDIO = "audio/";
    static constexpr const char* TEXT  = "text/";
    static constexpr const char* APPLICATION = "application/";
};

// =========================================================================
// MimeFilter — MIME type matching and filtering
// =========================================================================

class MimeFilter {
public:
    MimeFilter() = default;

    void set_whitelist(std::vector<std::string> mimes);
    void set_blacklist(std::vector<std::string> mimes);
    void clear();

    [[nodiscard]] bool accepts(const std::string& mime) const;
    [[nodiscard]] bool is_image(const std::string& mime) const;
    [[nodiscard]] bool is_video(const std::string& mime) const;
    [[nodiscard]] bool is_audio(const std::string& mime) const;

    [[nodiscard]] static std::string extension_to_mime(const std::string& ext);
    [[nodiscard]] static std::vector<std::string> default_mimes_for(PickerType type);

private:
    static bool match_glob(const std::string& pattern, const std::string& mime);

    std::vector<std::string> whitelist_;
    std::vector<std::string> blacklist_;
};

// =========================================================================
// FileMetadataExtractor — extract metadata from file URIs
// =========================================================================

class FileMetadataExtractor {
public:
    FileMetadataExtractor() = default;

    [[nodiscard]] PickedFile extract(const std::string& uri) const;
    [[nodiscard]] static std::string guess_mime_type(const std::string& path_or_uri);
    [[nodiscard]] static std::string format_size(int64_t bytes);
    [[nodiscard]] static std::string format_duration(int64_t ms);
    [[nodiscard]] static std::string safe_display_name(const std::string& path);

private:
    std::pair<int, int> probe_dimensions(const std::string& uri) const;
    int64_t probe_duration(const std::string& uri) const;
};

// =========================================================================
// MultiPicker — core picker engine
// =========================================================================

class MultiPicker {
public:
    explicit MultiPicker(PickerConfig config = {});
    ~MultiPicker() = default;

    // ---- Configuration ----
    void configure(PickerConfig config);
    [[nodiscard]] const PickerConfig& config() const;

    // ---- File loading ----
    void load_files();                        // loads all files matching config
    void reload();                            // force reload
    void load_files_from_paths(std::vector<std::string> paths);

    // ---- Selection ----
    void toggle_selection(int index);
    void select_all();
    void deselect_all();
    void select_range(int from, int to);
    [[nodiscard]] bool is_selected(int index) const;
    [[nodiscard]] int selected_count() const;
    [[nodiscard]] const std::vector<PickedFile>& selected_files() const;
    [[nodiscard]] const std::vector<PickedFile>& all_files() const;

    // ---- Filtering & Sorting ----
    void filter_by_mime(const std::string& mime_prefix);
    void filter_by_name(const std::string& query);
    void clear_filters();
    void sort(SortOrder order);

    // ---- Validation ----
    [[nodiscard]] bool can_confirm() const;
    [[nodiscard]] std::string validation_message() const;

    // ---- Camera capture ----
    void notify_capture_complete(const PickedFile& file);

    // ---- State ----
    [[nodiscard]] const PickerState& state() const;
    [[nodiscard]] bool loading() const;

    // ---- Callbacks ----
    void set_on_selection_changed(OnSelectionChanged cb);
    void set_on_files_loaded(OnFilesLoaded cb);
    void set_on_error(OnError cb);
    void set_on_capture_complete(OnCaptureComplete cb);
    void set_on_progress(OnProgress cb);

private:
    void apply_filters_and_sort();
    void notify_selection();
    bool validate_file(const PickedFile& f) const;

    PickerConfig config_;
    PickerState state_;
    MimeFilter mime_filter_;
    FileMetadataExtractor extractor_;

    // Callbacks
    OnSelectionChanged on_selection_changed_;
    OnFilesLoaded     on_files_loaded_;
    OnError           on_error_;
    OnCaptureComplete on_capture_complete_;
    OnProgress        on_progress_;
};

} // namespace multipicker
