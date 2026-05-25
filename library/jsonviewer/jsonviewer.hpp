#pragma once

/**
 * ===========================================================================
 * JSON VIEWER — JSON Tree Viewer with Expand/Collapse, Search, Pretty-Print
 * ===========================================================================
 *
 * An interactive JSON document explorer providing:
 *   - Tree-based navigation with expand/collapse at any depth
 *   - Full-text search across keys and values with match highlighting
 *   - Pretty-print with configurable indentation
 *   - Path breadcrumb tracking (e.g. "root > users[0] > name")
 *   - JSON validation and error reporting with line/column
 *   - Copy path, copy value, and copy subtree operations
 *   - Large-document support with virtualized node access
 *
 * This is a direct port of the Kotlin JSON Viewer used in the Matrix
 * client for inspecting event source JSON. Uses nlohmann_json under
 * the hood for parsing.
 */

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace jsonviewer {

// ---------- Enums ----------

enum class NodeType {
    Object,
    Array,
    String,
    Number,
    Boolean,
    Null
};

enum class SortMode {
    None,
    KeyAscending,
    KeyDescending
};

// ---------- Structs ----------

struct JsonPosition {
    int line = 1;
    int column = 1;
};

struct JsonError {
    std::string message;
    JsonPosition position;
};

struct SearchMatch {
    std::string path;          // JSON path of the match
    std::string matched_text;  // the text that matched
    int start_offset = 0;      // offset in the value string
    int end_offset = 0;
    bool in_key = false;       // match was in a key vs a value
};

struct JsonNode {
    std::string key;           // object key (empty for array elements)
    int array_index = -1;      // -1 if not an array element
    NodeType type = NodeType::Null;
    std::string value;         // formatted scalar value
    int depth = 0;
    bool expanded = false;
    bool has_children = false;
    int child_count = 0;
    std::string path;          // full JSON path
};

struct JsonViewerConfig {
    int indent_size = 2;
    bool indent_with_tabs = false;
    bool auto_expand_depth = 1;  // auto-expand first N levels
    int max_string_preview = 100;  // chars to show in tree
    bool show_array_indices = true;
    bool show_type_icons = true;
    bool wrap_long_lines = true;
    int wrap_column = 80;
    SortMode sort_mode = SortMode::None;
    bool case_sensitive_search = false;
    int max_search_results = 100;
};

struct JsonViewerState {
    std::vector<JsonNode> nodes;     // flat list for virtualized rendering
    int selected_index = -1;
    std::string root_path;
    bool valid = false;
    std::optional<JsonError> error;
};

// ---------- Callbacks ----------

using OnTreeChanged    = std::function<void()>;
using OnNodeSelected   = std::function<void(const JsonNode&)>;
using OnErrorCallback  = std::function<void(const JsonError&)>;
using OnSearchUpdated  = std::function<void(const std::vector<SearchMatch>&)>;

// =========================================================================
// JsonPath — JSON path parsing and manipulation
// =========================================================================

class JsonPath {
public:
    JsonPath() = default;
    ~JsonPath() = default;

    [[nodiscard]] static std::string join(const std::vector<std::string>& segments);
    [[nodiscard]] static std::vector<std::string> split(const std::string& path);
    [[nodiscard]] static std::string parent(const std::string& path);
    [[nodiscard]] static std::string append_key(const std::string& path,
                                                 const std::string& key);
    [[nodiscard]] static std::string append_index(const std::string& path, int index);
    [[nodiscard]] static std::string pretty(const std::string& path); // for breadcrumbs
};

// =========================================================================
// JsonPrettyPrinter — formatted JSON output
// =========================================================================

class JsonPrettyPrinter {
public:
    explicit JsonPrettyPrinter(int indent_size = 2, bool use_tabs = false);
    ~JsonPrettyPrinter() = default;

    [[nodiscard]] std::string pretty_print(const nlohmann::json& j) const;
    [[nodiscard]] std::string compact_print(const nlohmann::json& j) const;
    [[nodiscard]] std::string print_node(const JsonNode& node) const;
    [[nodiscard]] static std::string colorize(const std::string& json, bool ansi = true);

private:
    void print_recursive(const nlohmann::json& j, std::string& out, int depth,
                         bool compact) const;
    int indent_size_;
    bool use_tabs_;
};

// =========================================================================
// JsonSearcher — full-text search in JSON tree
// =========================================================================

class JsonSearcher {
public:
    explicit JsonSearcher(bool case_sensitive = false);
    ~JsonSearcher() = default;

    [[nodiscard]] std::vector<SearchMatch> search(const nlohmann::json& root,
                                                   const std::string& query,
                                                   int max_results = 100) const;

    [[nodiscard]] std::vector<int> find_matching_nodes(
        const std::vector<JsonNode>& flat_nodes,
        const std::vector<SearchMatch>& matches) const;

    [[nodiscard]] static std::vector<std::pair<int, int>>
    highlight_ranges(const std::string& text, const std::string& query,
                     bool case_sensitive);

private:
    void search_recursive(const nlohmann::json& node,
                          const std::string& path,
                          const std::string& query,
                          std::vector<SearchMatch>& results,
                          int max_results) const;

    bool case_sensitive_;
};

// =========================================================================
// JsonTreeBuilder — builds flat node list from JSON
// =========================================================================

class JsonTreeBuilder {
public:
    explicit JsonTreeBuilder(JsonViewerConfig config = {});
    ~JsonTreeBuilder() = default;

    [[nodiscard]] std::vector<JsonNode> build(const nlohmann::json& root) const;
    void set_expanded(std::vector<JsonNode>& nodes, int index, bool expanded);
    void toggle_expanded(std::vector<JsonNode>& nodes, int index);
    void expand_all(std::vector<JsonNode>& nodes);
    void collapse_all(std::vector<JsonNode>& nodes);
    void expand_to_depth(std::vector<JsonNode>& nodes, int max_depth);
    void expand_to_path(std::vector<JsonNode>& nodes, const std::string& path);

    [[nodiscard]] std::vector<JsonNode> rebuild_with_expansion(
        const nlohmann::json& root,
        const std::vector<JsonNode>& previous) const;

private:
    void build_recursive(const nlohmann::json& node,
                         const std::string& key,
                         const std::string& path,
                         int depth, bool expanded,
                         std::vector<JsonNode>& out) const;

    [[nodiscard]] static NodeType json_type(const nlohmann::json& j);
    [[nodiscard]] static std::string format_scalar(const nlohmann::json& j,
                                                    int max_len);

    JsonViewerConfig config_;
    std::vector<bool> expanded_states_; // indexed by path hash
};

// =========================================================================
// JsonViewer — main facade
// =========================================================================

class JsonViewer {
public:
    explicit JsonViewer(JsonViewerConfig config = {});
    ~JsonViewer() = default;

    // ---- Document loading ----
    bool load(const std::string& json_text);
    bool load(const nlohmann::json& j);
    void clear();
    [[nodiscard]] bool is_valid() const;
    [[nodiscard]] const std::optional<JsonError>& error() const;

    // ---- Tree navigation ----
    [[nodiscard]] const std::vector<JsonNode>& nodes() const;
    [[nodiscard]] const JsonNode* selected_node() const;
    void select(int index);
    void toggle_expand(int index);
    void expand_all();
    void collapse_all();
    void expand_to_depth(int depth);
    void refresh_tree();             // rebuild tree preserving expansion state

    // ---- Search ----
    [[nodiscard]] std::vector<SearchMatch> search(const std::string& query);
    [[nodiscard]] int next_search_result();
    [[nodiscard]] int prev_search_result();
    [[nodiscard]] int current_search_index() const;
    void clear_search();

    // ---- Pretty print ----
    [[nodiscard]] std::string pretty_print(bool colorize = false) const;
    [[nodiscard]] std::string compact_print() const;
    [[nodiscard]] std::string pretty_print_subtree(const std::string& path) const;

    // ---- Path operations ----
    [[nodiscard]] std::string current_path() const;
    [[nodiscard]] std::string path_for_index(int index) const;

    // ---- Validation ----
    [[nodiscard]] static std::optional<JsonError> validate(const std::string& json_text);

    // ---- Callbacks ----
    void set_on_tree_changed(OnTreeChanged cb);
    void set_on_node_selected(OnNodeSelected cb);
    void set_on_error(OnErrorCallback cb);
    void set_on_search_updated(OnSearchUpdated cb);

    // ---- Config ----
    void set_config(JsonViewerConfig config);
    [[nodiscard]] const JsonViewerConfig& config() const;

private:
    nlohmann::json parsed_;
    JsonViewerConfig config_;
    JsonViewerState state_;
    JsonTreeBuilder builder_;
    JsonPrettyPrinter printer_;
    JsonSearcher searcher_;

    // Search state
    std::vector<SearchMatch> search_results_;
    int search_result_index_ = -1;

    // Callbacks
    OnTreeChanged   on_tree_changed_;
    OnNodeSelected  on_node_selected_;
    OnErrorCallback on_error_;
    OnSearchUpdated on_search_updated_;
};

} // namespace jsonviewer
