#pragma once

/**
 * ===========================================================================
 * BARCODE SCANNER — QR / Barcode Scanning Interface
 * ===========================================================================
 *
 * An abstraction layer for barcode and QR code scanning providing:
 *   - Multi-format barcode detection (QR, EAN, UPC, Code128, Code39,
 *     DataMatrix, PDF417, Aztec, ITF, Codabar)
 *   - Camera frame analysis pipeline (YUV/NV21 → grayscale → decode)
 *   - Viewport region-of-interest for targeting
 *   - Auto-focus trigger and torch control
 *   - Continuous vs. single-scan modes
 *   - Scan result with bounding box coordinates
 *   - Error handling for missing camera permissions
 *   - Symbology-specific validation (e.g. ISBN for EAN-13)
 *
 * This is a direct port of the Kotlin barcode scanner module used in
 * the Matrix client for QR-based device verification and room joining.
 * The actual image decoding is delegated to a platform backend
 * (ZXing-C++, ZBar, or platform-native scanner), but all orchestration,
 * format configuration, and result processing lives here.
 */

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace barcodescanner {

// ---------- Enums ----------

enum class BarcodeFormat {
    QRCode,
    EAN8,
    EAN13,
    UPC_A,
    UPC_E,
    Code128,
    Code39,
    Code93,
    DataMatrix,
    PDF417,
    Aztec,
    ITF,         // Interleaved 2 of 5
    Codabar,
    MaxiCode,
    RSS14,       // Reduced Space Symbology
    RSExpanded,
    Unknown
};

enum class ScanMode {
    Single,       // scan one code then stop
    Continuous    // keep scanning until stopped
};

enum class ScannerState {
    Idle,
    Scanning,
    Decoding,
    Success,
    Error
};

// ---------- Structs ----------

struct Point {
    int x = 0;
    int y = 0;
};

struct Rect {
    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;

    [[nodiscard]] int width() const { return right - left; }
    [[nodiscard]] int height() const { return bottom - top; }
};

struct BarcodeResult {
    std::string raw_text;          // decoded data
    BarcodeFormat format = BarcodeFormat::Unknown;
    std::string format_name;       // human-readable format name
    std::vector<Point> corners;    // 4 corners of barcode in image
    Rect bounding_box;
    int64_t timestamp_ms = 0;      // when scanned
    std::optional<std::string> error_correction_level; // QR: L, M, Q, H
};

struct ScannerConfig {
    std::vector<BarcodeFormat> enabled_formats = {
        BarcodeFormat::QRCode,
        BarcodeFormat::EAN13,
        BarcodeFormat::EAN8,
        BarcodeFormat::UPC_A,
        BarcodeFormat::UPC_E,
        BarcodeFormat::Code128,
        BarcodeFormat::Code39,
        BarcodeFormat::DataMatrix,
        BarcodeFormat::PDF417,
        BarcodeFormat::Aztec,
        BarcodeFormat::ITF,
        BarcodeFormat::Codabar
    };
    ScanMode scan_mode = ScanMode::Single;
    bool try_harder = false;       // spend more CPU per frame
    bool try_rotate = true;        // attempt 90/180/270 rotations
    bool try_inverted = false;     // try inverted (white-on-black) codes
    int max_results_per_frame = 1;
    Rect region_of_interest;       // empty = full frame
    int camera_preview_width = 1920;
    int camera_preview_height = 1080;
    float auto_focus_interval = 2.0F; // seconds between auto-focus triggers
    bool enable_torch = false;
    bool require_valid_checksum = true;
};

struct ScannerStateInfo {
    ScannerState state = ScannerState::Idle;
    std::optional<BarcodeResult> last_result;
    std::string error_message;
    bool torch_on = false;
    int frame_count = 0;
    int scan_count = 0;
};

// ---------- Callbacks ----------

using OnScanResult  = std::function<void(const BarcodeResult&)>;
using OnStateChange = std::function<void(ScannerState old_state, ScannerState new_state)>;
using OnError       = std::function<void(const std::string& message)>;
using OnTorchChange = std::function<void(bool enabled)>;

// ---------- Format utilities ----------

[[nodiscard]] const char* barcode_format_name(BarcodeFormat fmt);
[[nodiscard]] std::optional<BarcodeFormat> barcode_format_from_string(const std::string& name);
[[nodiscard]] bool is_1d_format(BarcodeFormat fmt);
[[nodiscard]] bool is_2d_format(BarcodeFormat fmt);

// =========================================================================
// FrameAnalyzer — pre-processes camera frames for decoding
// =========================================================================

class FrameAnalyzer {
public:
    FrameAnalyzer() = default;
    ~FrameAnalyzer() = default;

    /**
     * Convert YUV NV21 frame data to grayscale bytes.
     * NV21 is the standard Android camera preview format.
     */
    [[nodiscard]] static std::vector<uint8_t> nv21_to_grayscale(
        const uint8_t* nv21_data, int width, int height);

    /**
     * Convert YUV YV12 frame data to grayscale bytes.
     */
    [[nodiscard]] static std::vector<uint8_t> yv12_to_grayscale(
        const uint8_t* yv12_data, int width, int height);

    /**
     * Crop a region of interest from grayscale data.
     */
    [[nodiscard]] static std::vector<uint8_t> crop_roi(
        const std::vector<uint8_t>& gray, int width, int height, Rect roi);

    /**
     * Apply a simple contrast enhancement (histogram stretching).
     */
    [[nodiscard]] static std::vector<uint8_t> enhance_contrast(
        const std::vector<uint8_t>& gray);

    /**
     * Rotate grayscale image by 90, 180, or 270 degrees.
     */
    [[nodiscard]] static std::vector<uint8_t> rotate_gray(
        const std::vector<uint8_t>& gray, int width, int height, int degrees);

    /**
     * Detect if the image is mostly dark (for inverted code detection).
     */
    [[nodiscard]] static bool is_mostly_dark(const std::vector<uint8_t>& gray,
                                              float threshold = 0.6F);

    /**
     * Invert grayscale image.
     */
    [[nodiscard]] static std::vector<uint8_t> invert(const std::vector<uint8_t>& gray);
};

// =========================================================================
// BarcodeValidator — symbology-specific checks
// =========================================================================

class BarcodeValidator {
public:
    BarcodeValidator() = default;
    ~BarcodeValidator() = default;

    [[nodiscard]] static bool is_valid_ean13(const std::string& code);
    [[nodiscard]] static bool is_valid_ean8(const std::string& code);
    [[nodiscard]] static bool is_valid_upc_a(const std::string& code);
    [[nodiscard]] static bool is_valid_upc_e(const std::string& code);
    [[nodiscard]] static bool is_valid_isbn10(const std::string& code);
    [[nodiscard]] static bool is_valid_isbn13(const std::string& code);

    /**
     * Validate a result based on its format.
     */
    [[nodiscard]] static bool validate(const BarcodeResult& result,
                                        bool require_checksum = true);

    /**
     * Compute Luhn-like checksums for various formats.
     */
    [[nodiscard]] static int compute_ean13_checksum(const std::string& digits);
    [[nodiscard]] static int compute_upc_a_checksum(const std::string& digits);

private:
    [[nodiscard]] static int digit_sum(const std::string& s, const std::vector<int>& weights);
};

// =========================================================================
// BarcodeScanner — main scanner engine
// =========================================================================

class BarcodeScanner {
public:
    explicit BarcodeScanner(ScannerConfig config = {});
    ~BarcodeScanner() = default;

    // ---- Lifecycle ----
    void start();
    void stop();
    void pause();
    void resume();
    [[nodiscard]] bool is_running() const;

    // ---- Frame processing ----
    /**
     * Process a camera frame and attempt to decode barcodes.
     * For NV21 frames (Android Camera1/Camera2).
     */
    void process_frame_nv21(const uint8_t* data, int width, int height,
                            int64_t timestamp_ms = 0);

    /**
     * Process a pre-converted grayscale frame.
     */
    void process_frame_gray(const uint8_t* data, int width, int height,
                            int64_t timestamp_ms = 0);

    // ---- Configuration ----
    void set_config(ScannerConfig config);
    [[nodiscard]] const ScannerConfig& config() const;
    void set_enabled_formats(std::vector<BarcodeFormat> formats);
    void set_region_of_interest(Rect roi);

    // ---- Torch / Flash ----
    void set_torch(bool enabled);
    void toggle_torch();
    [[nodiscard]] bool torch_on() const;

    // ---- Results ----
    [[nodiscard]] const std::optional<BarcodeResult>& last_result() const;
    [[nodiscard]] std::vector<BarcodeResult> drain_results(); // returns and clears
    [[nodiscard]] int scan_count() const;
    void clear_results();

    // ---- State ----
    [[nodiscard]] const ScannerStateInfo& state_info() const;

    // ---- Callbacks ----
    void set_on_scan_result(OnScanResult cb);
    void set_on_state_change(OnStateChange cb);
    void set_on_error(OnError cb);
    void set_on_torch_change(OnTorchChange cb);

private:
    void set_state(ScannerState new_state);
    void process_results(std::vector<BarcodeResult> results);

    ScannerConfig config_;
    ScannerStateInfo state_info_;

    OnScanResult  on_scan_result_;
    OnStateChange on_state_change_;
    OnError       on_error_;
    OnTorchChange on_torch_change_;
};

} // namespace barcodescanner
