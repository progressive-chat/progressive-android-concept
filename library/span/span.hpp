#pragma once

/**
 * ===========================================================================
 * SPAN — Rich Text Span System
 * ===========================================================================
 *
 * A full-featured rich text span engine supporting:
 *   - Bold, italic, underline, strikethrough formatting
 *   - Inline code (monospace with background)
 *   - Hyperlinks with URL data
 *   - Foreground and background color spans (hex or named)
 *   - Custom typeface/font-family spans
 *   - Relative size spans (smaller, larger, percentage)
 *   - Subscript and superscript
 *   - Span merging, splitting, and intersection detection
 *   - Style cascading for overlapping spans
 *   - Builder pattern for declarative construction
 *
 * This is a direct port of Android's SpannableStringBuilder / Spanned
 * interface used in the Matrix chat client for message rendering.
 */

#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace span {

// ---------- Enums ----------

enum class TypefaceStyle {
    Normal,
    Bold,
    Italic,
    BoldItalic,
    Monospace,
    Serif,
    SansSerif
};

enum class SpanFlag : uint32_t {
    None            = 0,
    ExclusiveInclusive = 1 << 0,  // [start, end)
    InclusiveExclusive = 1 << 1,  // (start, end]
    InclusiveInclusive = 1 << 2,  // [start, end]
    ExclusiveExclusive = 1 << 3,  // (start, end)
};

constexpr SpanFlag operator|(SpanFlag a, SpanFlag b) {
    return static_cast<SpanFlag>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
constexpr bool operator&(SpanFlag a, SpanFlag b) {
    return (static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) != 0;
}

enum class SpanType {
    Bold,
    Italic,
    Underline,
    Strikethrough,
    Code,
    Link,
    ForegroundColor,
    BackgroundColor,
    Typeface,
    RelativeSize,
    Subscript,
    Superscript,
    Custom
};

// ---------- Color ----------

struct Color {
    uint8_t r = 0, g = 0, b = 0, a = 255;

    static Color from_hex(uint32_t hex);
    static Color from_hex_argb(uint32_t argb);
    static Color from_string(const std::string& name_or_hex);
    [[nodiscard]] uint32_t to_argb() const;
    [[nodiscard]] std::string to_hex() const;

    static const Color Red, Green, Blue, Yellow, Cyan, Magenta,
                       White, Black, Gray, Transparent;
};

// ---------- Base span type ----------

struct Span {
    int start = 0;  // character offset in source text
    int end = 0;
    SpanFlag flags = SpanFlag::ExclusiveInclusive;
    SpanType type = SpanType::Custom;

    [[nodiscard]] int length() const { return end - start; }
    [[nodiscard]] bool contains(int offset) const { return offset >= start && offset < end; }
    [[nodiscard]] bool overlaps(const Span& other) const {
        return start < other.end && other.start < end;
    }
};

// ---------- Concrete span types ----------

struct BoldSpan : Span {
    BoldSpan() { type = SpanType::Bold; }
};

struct ItalicSpan : Span {
    ItalicSpan() { type = SpanType::Italic; }
};

struct UnderlineSpan : Span {
    UnderlineSpan() { type = SpanType::Underline; }
};

struct StrikethroughSpan : Span {
    StrikethroughSpan() { type = SpanType::Strikethrough; }
};

struct CodeSpan : Span {
    CodeSpan() { type = SpanType::Code; }
    Color background = {40, 42, 54, 255}; // dark bg default
};

struct LinkSpan : Span {
    LinkSpan() { type = SpanType::Link; }
    std::string url;
    std::string tooltip;
};

struct ForegroundColorSpan : Span {
    ForegroundColorSpan() { type = SpanType::ForegroundColor; }
    Color color;
};

struct BackgroundColorSpan : Span {
    BackgroundColorSpan() { type = SpanType::BackgroundColor; }
    Color color;
};

struct TypefaceSpan : Span {
    TypefaceSpan() { type = SpanType::Typeface; }
    TypefaceStyle style = TypefaceStyle::Normal;
    std::string family_name; // e.g. "JetBrains Mono"
};

struct RelativeSizeSpan : Span {
    RelativeSizeSpan() { type = SpanType::RelativeSize; }
    float proportion = 1.0F; // 1.0 = normal, 1.5 = 150%
};

struct SubscriptSpan : Span {
    SubscriptSpan() { type = SpanType::Subscript; }
};

struct SuperscriptSpan : Span {
    SuperscriptSpan() { type = SpanType::Superscript; }
};

// ---------- Variant for type-erased span storage ----------

using SpanVariant = std::variant<
    BoldSpan,
    ItalicSpan,
    UnderlineSpan,
    StrikethroughSpan,
    CodeSpan,
    LinkSpan,
    ForegroundColorSpan,
    BackgroundColorSpan,
    TypefaceSpan,
    RelativeSizeSpan,
    SubscriptSpan,
    SuperscriptSpan
>;

// ---------- Helpers ----------

inline int span_start(const SpanVariant& sv) {
    return std::visit([](const auto& s) { return s.start; }, sv);
}
inline int span_end(const SpanVariant& sv) {
    return std::visit([](const auto& s) { return s.end; }, sv);
}
inline void set_span_range(SpanVariant& sv, int start, int end) {
    std::visit([=](auto& s) { s.start = start; s.end = end; }, sv);
}
inline SpanType span_type(const SpanVariant& sv) {
    return std::visit([](const auto& s) { return s.type; }, sv);
}

// =========================================================================
// SpannedString — text with associated spans
// =========================================================================

class SpannedString {
public:
    SpannedString() = default;
    explicit SpannedString(std::string text);
    SpannedString(std::string text, std::vector<SpanVariant> spans);
    ~SpannedString() = default;

    // ---- Text ----
    [[nodiscard]] const std::string& text() const;
    void set_text(std::string text);
    [[nodiscard]] int length() const;
    [[nodiscard]] std::string substring(int start, int end) const;

    // ---- Span management ----
    void add_span(SpanVariant span);
    void remove_spans(int start, int end, std::optional<SpanType> type = std::nullopt);
    void clear_spans();

    // ---- Span queries ----
    [[nodiscard]] std::vector<SpanVariant> spans_at(int offset) const;
    [[nodiscard]] std::vector<SpanVariant> spans_in_range(int start, int end) const;
    [[nodiscard]] const std::vector<SpanVariant>& all_spans() const;
    [[nodiscard]] int span_count() const;
    [[nodiscard]] bool has_span_type(SpanType type, int start, int end) const;

    // ---- Text mutation with span adjustment ----
    void insert(int where, const std::string& text);
    void replace(int start, int end, const std::string& text);
    void append(const std::string& text);
    void append(const SpannedString& other);

    // ---- Style extraction ----
    struct StyleAtOffset {
        bool bold = false;
        bool italic = false;
        bool underline = false;
        bool strikethrough = false;
        bool code = false;
        bool subscript = false;
        bool superscript = false;
        std::optional<Color> foreground;
        std::optional<Color> background;
        std::optional<std::string> link_url;
        std::optional<TypefaceStyle> typeface;
        float relative_size = 1.0F;
    };
    [[nodiscard]] StyleAtOffset style_at(int offset) const;

private:
    void adjust_spans_for_insert(int where, int len);
    void adjust_spans_for_delete(int start, int end);

    std::string text_;
    std::vector<SpanVariant> spans_;
};

// =========================================================================
// SpanBuilder — declarative span construction
// =========================================================================

class SpanBuilder {
public:
    SpanBuilder() = default;
    explicit SpanBuilder(std::string text);
    ~SpanBuilder() = default;

    SpanBuilder& bold(int start, int end);
    SpanBuilder& italic(int start, int end);
    SpanBuilder& underline(int start, int end);
    SpanBuilder& strikethrough(int start, int end);
    SpanBuilder& code(int start, int end, std::optional<Color> bg = std::nullopt);
    SpanBuilder& link(int start, int end, const std::string& url);
    SpanBuilder& foreground(int start, int end, Color color);
    SpanBuilder& background(int start, int end, Color color);
    SpanBuilder& typeface(int start, int end, TypefaceStyle style,
                          const std::string& family = "");
    SpanBuilder& relative_size(int start, int end, float proportion);
    SpanBuilder& sub(int start, int end);     // subscript
    SpanBuilder& sup(int start, int end);     // superscript
    SpanBuilder& text(const std::string& t);

    [[nodiscard]] SpannedString build() const;

private:
    std::string text_;
    std::vector<SpanVariant> spans_;
};

// =========================================================================
// SpanMerger — merges adjacent/compatible spans
// =========================================================================

class SpanMerger {
public:
    SpanMerger() = default;
    ~SpanMerger() = default;

    [[nodiscard]] std::vector<SpanVariant> merge(const std::vector<SpanVariant>& spans) const;
    [[nodiscard]] std::vector<SpanVariant> merge_adjacent(const std::vector<SpanVariant>& spans) const;

private:
    [[nodiscard]] static bool compatible(const SpanVariant& a, const SpanVariant& b);
};

// =========================================================================
// SpanParser — parses markup text into SpannedString
// =========================================================================

class SpanParser {
public:
    SpanParser() = default;
    ~SpanParser() = default;

    /**
     * Parse text with simple markup:
     *   **bold**  *italic*  __underline__  ~~strikethrough~~
     *   `code`  [link text](url)
     * Supports nested/overlapping spans.
     */
    [[nodiscard]] SpannedString parse_markdown(const std::string& text) const;

    /**
     * Parse text with HTML-like tags:
     *   <b>bold</b> <i>italic</i> <u>underline</u> <s>strikethrough</s>
     *   <code>code</code> <a href="url">link</a>
     */
    [[nodiscard]] SpannedString parse_html_like(const std::string& text) const;

private:
    struct ParseMatch {
        int start;
        int end;
        std::string delimiter;
    };
    [[nodiscard]] static std::vector<ParseMatch> find_delimiters(
        const std::string& text, const std::string& delim);
};

} // namespace span
