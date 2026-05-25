#pragma once

/**
 * ===========================================================================
 * TEXT DRAWABLE — Text-Based Avatar Generation
 * ===========================================================================
 *
 * A text-to-avatar generator providing:
 *   - Initials extraction from display names (handles Unicode, emoji)
 *   - Deterministic color generation from name hash
 *   - Circular and rounded-rect avatar shapes
 *   - Configurable text size, color, and background color
 *   - Multiple color palettes (material, pastel, vibrant, grayscale)
 *   - Contrast ratio calculation for text-on-background readability
 *   - Bitmap output as raw RGBA pixel buffer
 *   - Caching with LRU eviction for frequently-used avatars
 *
 * This is a direct port of the Kotlin TextDrawable library used
 * throughout the Matrix client for user/room avatars when no
 * image is available.
 */

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace textdrawable {

// ---------- Enums ----------

enum class AvatarShape {
    Circle,
    RoundedRect,
    Square
};

enum class ColorPalette {
    Material,   // Google Material Design colors
    Pastel,     // soft pastels
    Vibrant,    // saturated, high-contrast
    Grayscale,  // black/white/gray
    Flat        // flat UI colors
};

// ---------- Structs ----------

struct AvatarConfig {
    int size = 128;                // width & height in pixels
    int font_size = 48;           // default font size in pixels
    int corner_radius = 16;       // for RoundedRect shape
    float border_width = 0.0F;    // 0 = no border
    ColorPalette palette = ColorPalette::Material;
    AvatarShape shape = AvatarShape::Circle;
    bool bold_text = true;
    bool uppercase = true;
    int max_initials = 2;         // how many initials to use
};

struct RgbaColor {
    uint8_t r = 0, g = 0, b = 0, a = 255;

    static RgbaColor from_hex(uint32_t hex);
    [[nodiscard]] uint32_t to_uint32() const;
    [[nodiscard]] float luminance() const;   // relative luminance
    [[nodiscard]] float contrast_ratio(const RgbaColor& other) const;
    [[nodiscard]] RgbaColor lighten(float factor) const;  // 0..1
    [[nodiscard]] RgbaColor darken(float factor) const;   // 0..1
};

struct AvatarBitmap {
    std::vector<uint8_t> pixels;  // RGBA packed, row-major
    int width = 0;
    int height = 0;

    [[nodiscard]] bool empty() const { return pixels.empty(); }
    [[nodiscard]] size_t byte_size() const { return pixels.size(); }
};

struct AvatarResult {
    AvatarBitmap bitmap;
    RgbaColor background_color;
    RgbaColor text_color;
    std::string initials;
    std::string display_name;
    int size = 0;
};

// ---------- Color palette data ----------

struct PaletteColor {
    RgbaColor color;
    std::string name;
};

using Palette = std::vector<PaletteColor>;

// =========================================================================
// ColorPaletteEngine — palette management and deterministic color selection
// =========================================================================

class ColorPaletteEngine {
public:
    ColorPaletteEngine() = default;
    ~ColorPaletteEngine() = default;

    /**
     * Select a deterministic color from a palette based on a string key.
     * Uses std::hash for distribution.
     */
    [[nodiscard]] RgbaColor color_for_name(const std::string& name,
                                            ColorPalette palette) const;

    /**
     * Get the full palette.
     */
    [[nodiscard]] static const Palette& get_palette(ColorPalette palette);

    /**
     * Compute an appropriate text color (black or white) based on
     * background luminance for readability (WCAG AA compliance).
     */
    [[nodiscard]] static RgbaColor text_color_for_background(RgbaColor bg);

    /**
     * Compute contrast ratio between two colors (WCAG 2.0).
     */
    [[nodiscard]] static float contrast_ratio(RgbaColor fg, RgbaColor bg);

private:
    static Palette material_palette_;
    static Palette pastel_palette_;
    static Palette vibrant_palette_;
    static Palette grayscale_palette_;
    static Palette flat_palette_;
};

// =========================================================================
// InitialsExtractor — extract initials from display names
// =========================================================================

class InitialsExtractor {
public:
    InitialsExtractor() = default;
    ~InitialsExtractor() = default;

    /**
     * Extract initials from a display name.
     *
     * Rules:
     *   - "John Doe" → "JD"
     *   - "john.doe" → "JD"
     *   - "admin" → "A"
     *   - "Alice Bob Charlie" → "AB"
     *   - "@user123" → "U"
     *   - "" → "?"
     *   - "张三" → "张" (single CJK character)
     */
    [[nodiscard]] std::string extract(const std::string& name,
                                       int max_initials = 2) const;

    /**
     * Special handling for matrix.to user IDs (@user:domain)
     */
    [[nodiscard]] std::string extract_from_user_id(const std::string& user_id,
                                                     const std::string& display_name = "",
                                                     int max_initials = 2) const;

private:
    [[nodiscard]] static bool is_cjk(char32_t cp);
    [[nodiscard]] static bool is_emoji(char32_t cp);
    [[nodiscard]] static std::vector<std::string> split_name(const std::string& name);
    [[nodiscard]] static std::u32string to_utf32(const std::string& s);
};

// =========================================================================
// TextDrawableRenderer — bitmap rendering
// =========================================================================

class TextDrawableRenderer {
public:
    explicit TextDrawableRenderer(AvatarConfig config = {});
    ~TextDrawableRenderer() = default;

    /**
     * Render an avatar bitmap for a given name.
     */
    [[nodiscard]] AvatarResult render(const std::string& name) const;

    /**
     * Render with explicit colors (bypass palette lookup).
     */
    [[nodiscard]] AvatarResult render_with_colors(const std::string& name,
                                                    RgbaColor background,
                                                    RgbaColor text_color) const;

    [[nodiscard]] const AvatarConfig& config() const;
    void set_config(AvatarConfig config);

    /**
     * Draw simple bitmap text (no external font library required).
     * Uses built-in 5x7 pixel font for ASCII characters.
     */
    [[nodiscard]] static AvatarBitmap draw_text(const std::string& text,
                                                  int size, int font_size,
                                                  RgbaColor text_color,
                                                  RgbaColor bg_color);

private:
    [[nodiscard]] AvatarBitmap render_circle(const AvatarBitmap& source) const;
    [[nodiscard]] AvatarBitmap render_rounded_rect(const AvatarBitmap& source) const;
    [[nodiscard]] AvatarBitmap apply_shape(const AvatarBitmap& source) const;

    [[nodiscard]] static std::vector<std::vector<bool>> bitmap_char(char c);
    [[nodiscard]] static int char_width(char c);

    AvatarConfig config_;
    ColorPaletteEngine palette_engine_;
    InitialsExtractor initials_extractor_;
};

// =========================================================================
// AvatarCache — LRU cache for rendered avatars
// =========================================================================

class AvatarCache {
public:
    explicit AvatarCache(size_t max_size = 200);
    ~AvatarCache() = default;

    [[nodiscard]] std::optional<AvatarResult> get(const std::string& key) const;
    void put(const std::string& key, AvatarResult result);
    void remove(const std::string& key);
    void clear();
    [[nodiscard]] size_t size() const;
    [[nodiscard]] size_t max_size() const;

private:
    struct CacheEntry {
        AvatarResult result;
        size_t access_order; // monotonically increasing
    };

    size_t max_size_;
    size_t access_counter_ = 0;
    mutable std::unordered_map<std::string, CacheEntry> cache_;
    // LRU eviction: the CacheEntry with the smallest access_order is evicted
    void evict_if_needed();
};

// =========================================================================
// TextDrawable — main facade
// =========================================================================

class TextDrawable {
public:
    explicit TextDrawable(AvatarConfig config = {});
    ~TextDrawable() = default;

    /**
     * Get or create an avatar for the given name.
     * Uses cache; returns cached result if available.
     */
    [[nodiscard]] AvatarResult get_avatar(const std::string& name);

    /**
     * Get or create an avatar for a Matrix user ID.
     */
    [[nodiscard]] AvatarResult get_avatar_for_user(const std::string& user_id,
                                                     const std::string& display_name = "");

    /**
     * Force re-render (bypass cache).
     */
    [[nodiscard]] AvatarResult render(const std::string& name);

    /**
     * Clear the avatar cache.
     */
    void clear_cache();

    /**
     * Pre-warm the cache with a list of names.
     */
    void warm_cache(const std::vector<std::string>& names);

    [[nodiscard]] const AvatarConfig& config() const;
    void set_config(AvatarConfig config);

    /**
     * Generate a deterministic color for a string.
     */
    [[nodiscard]] static RgbaColor color_for_name(const std::string& name,
                                                    ColorPalette palette = ColorPalette::Material);

    /**
     * Extract initials from a name.
     */
    [[nodiscard]] static std::string initials(const std::string& name,
                                                int max_initials = 2);

private:
    AvatarConfig config_;
    TextDrawableRenderer renderer_;
    AvatarCache cache_;
    InitialsExtractor extractor_;
};

} // namespace textdrawable
