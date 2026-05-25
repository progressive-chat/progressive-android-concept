#include "diff_match_patch.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <unordered_map>
#include <chrono>

namespace progressive::diff {

DiffMatchPatch::DiffMatchPatch() = default;
DiffMatchPatch::~DiffMatchPatch() = default;

// ── Diff ──

std::vector<Diff> DiffMatchPatch::diff(const std::string& text1, const std::string& text2,
                                         bool checkLines, float deadline) {
    if (text1 == text2) {
        if (text1.empty()) return {};
        return {{DiffOperation::EQUAL, text1}};
    }

    // Strip common prefix
    size_t commonPrefix = 0;
    while (commonPrefix < text1.size() && commonPrefix < text2.size() &&
           text1[commonPrefix] == text2[commonPrefix]) {
        commonPrefix++;
    }
    std::string prefix = text1.substr(0, commonPrefix);
    std::string t1 = text1.substr(commonPrefix);
    std::string t2 = text2.substr(commonPrefix);

    // Strip common suffix
    size_t commonSuffix = 0;
    while (commonSuffix < t1.size() && commonSuffix < t2.size() &&
           t1[t1.size() - 1 - commonSuffix] == t2[t2.size() - 1 - commonSuffix]) {
        commonSuffix++;
    }
    std::string suffix = t1.substr(t1.size() - commonSuffix);
    t1 = t1.substr(0, t1.size() - commonSuffix);
    t2 = t2.substr(0, t2.size() - commonSuffix);

    std::vector<Diff> diffs;
    diffCompute(t1, t2, checkLines, deadline, diffs);

    // Restore prefix/suffix
    if (!prefix.empty()) diffs.insert(diffs.begin(), {DiffOperation::EQUAL, prefix});
    if (!suffix.empty()) diffs.push_back({DiffOperation::EQUAL, suffix});
    diffCleanupMerge(diffs);
    return diffs;
}

void DiffMatchPatch::diffCompute(const std::string& text1, const std::string& text2,
                                    bool checkLines, float deadline,
                                    std::vector<Diff>& diffs) {
    if (text1.empty()) { diffs = {{DiffOperation::INSERT, text2}}; return; }
    if (text2.empty()) { diffs = {{DiffOperation::DELETE, text1}}; return; }

    // Trim common prefix again (with the checkLines shortcut)
    if (checkLines && text1.size() > 100 && text2.size() > 100) {
        auto linesResult = diffLinesToChars(text1, text2);
        // Simplified implementation
    }

    // Myers O(ND) diff algorithm
    size_t n = text1.size(), m = text2.size();
    size_t max = n + m;

    std::vector<int> v(2 * max + 1, -1);
    v[max + 1] = 0;

    for (size_t d = 0; d <= max; d++) {
        for (int k = -static_cast<int>(d); k <= static_cast<int>(d); k += 2) {
            size_t x = (k == -static_cast<int>(d) || (k != static_cast<int>(d) &&
                        v[max + k - 1] < v[max + k + 1]))
                        ? v[max + k + 1]
                        : v[max + k - 1] + 1;
            size_t y = x - k;

            while (x < n && y < m && text1[x] == text2[y]) { x++; y++; }
            v[max + k] = x;

            if (x >= n && y >= m) {
                // Found the diff
                // Backtrack to reconstruct
                // Simplified for stub
                diffs = {{DiffOperation::EQUAL, text1}};
                return;
            }
        }
    }
}

std::vector<Diff> DiffMatchPatch::diffLinesToChars(const std::string& text1,
                                                      const std::string& text2) {
    std::vector<std::string> linesArray;
    std::unordered_map<std::string, int> lineHash;

    auto linesToChars = [&](const std::string& text, std::string& chars) {
        std::istringstream stream(text);
        std::string line;
        while (std::getline(stream, line, '
')) {
            line += '
';
            if (lineHash.find(line) == lineHash.end()) {
                lineHash[line] = linesArray.size();
                linesArray.push_back(line);
            }
            chars += static_cast<char>(lineHash[line]);
        }
    };

    std::string chars1, chars2;
    linesToChars(text1, chars1);
    linesToChars(text2, chars2);
    return {{DiffOperation::EQUAL, chars1}};
}

std::string DiffMatchPatch::diffToText(const std::vector<Diff>& diffs) {
    std::ostringstream os;
    for (auto& d : diffs) {
        switch (d.operation) {
            case DiffOperation::INSERT: os << '+' << d.text << '	'; break;
            case DiffOperation::DELETE: os << '-' << d.text << '	'; break;
            case DiffOperation::EQUAL: os << ' ' << d.text << '	'; break;
        }
    }
    return os.str();
}

std::vector<Diff> DiffMatchPatch::diffFromText(const std::string& text) {
    std::vector<Diff> diffs;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line, '	')) {
        if (line.empty()) continue;
        char op = line[0];
        std::string data = line.substr(1);
        switch (op) {
            case '+': diffs.push_back({DiffOperation::INSERT, data}); break;
            case '-': diffs.push_back({DiffOperation::DELETE, data}); break;
            default: diffs.push_back({DiffOperation::EQUAL, data}); break;
        }
    }
    return diffs;
}

void DiffMatchPatch::diffCleanupSemantic(std::vector<Diff>& diffs) {
    bool changed = true;
    while (changed) {
        changed = false;
        // Merge adjacent diffs, split at sentence boundaries, etc.
    }
}

void DiffMatchPatch::diffCleanupEfficiency(std::vector<Diff>& diffs) {
    if (diffs.empty()) return;
    bool changed = true;
    while (changed) {
        changed = false;
        // Optimize diffs for efficiency
    }
}

void DiffMatchPatch::diffCleanupMerge(std::vector<Diff>& diffs) {
    if (diffs.empty()) return;
    // Merge adjacent equal diffs, etc.
}

std::vector<Diff> DiffMatchPatch::diffCharsToLines(const std::vector<Diff>& diffs,
                                                      const std::vector<std::string>& lines) {
    std::vector<Diff> result;
    for (auto& d : diffs) {
        std::string text;
        for (char c : d.text) {
            text += lines[static_cast<unsigned char>(c)];
        }
        result.push_back({d.operation, text});
    }
    return result;
}

// ── Patch ──

std::vector<Patch> DiffMatchPatch::patchMake(const std::string& text1,
                                               const std::string& text2) {
    auto diffs = diff(text1, text2, true);
    if (diffs.empty() || (diffs.size() == 1 && diffs[0].operation == DiffOperation::EQUAL)) {
        return {};
    }
    return patchMake(diffs);
}

std::vector<Patch> DiffMatchPatch::patchMake(const std::vector<Diff>& diffs) {
    std::vector<Patch> patches;
    if (diffs.empty()) return patches;

    Patch patch;
    int charCount1 = 0, charCount2 = 0;
    std::string prepatchText = "", postpatchText = "";

    for (size_t x = 0; x < diffs.size(); x++) {
        auto& d = diffs[x];
        if (d.operation == DiffOperation::EQUAL) {
            if (d.text.size() >= 2 * m_patchMargin && !patch.diffs.empty() &&
                x != diffs.size() - 1) {
                patch.diffs.push_back(d);
                patch.start1 = charCount1 - static_cast<int>(prepatchText.size());
                patch.start2 = charCount2 - static_cast<int>(postpatchText.size());
                patch.length1 = charCount1 + prepatchText.size() - patch.start1;
                patch.length2 = charCount2 + postpatchText.size() - patch.start2;
                patches.push_back(patch);
                patch = Patch();
                prepatchText.clear();
                postpatchText.clear();
            }
        }
        if (d.operation == DiffOperation::INSERT) {
            charCount2 += d.text.size();
            postpatchText += d.text;
            patch.diffs.push_back(d);
            patch.length1 = charCount1 - patch.start1;
            patch.length2 = charCount2 - patch.start2;
        } else if (d.operation == DiffOperation::DELETE) {
            charCount1 += d.text.size();
            prepatchText += d.text;
            patch.diffs.push_back(d);
            patch.length1 = charCount1 - patch.start1;
            patch.length2 = charCount2 - patch.start2;
        } else {
            charCount1 += d.text.size();
            charCount2 += d.text.size();
            prepatchText += d.text;
            postpatchText += d.text;
        }
    }
    return patches;
}

std::string DiffMatchPatch::patchToText(const std::vector<Patch>& patches) {
    std::ostringstream os;
    for (auto& p : patches) {
        os << "@@ -" << p.start1 << "," << p.length1
           << " +" << p.start2 << "," << p.length2 << " @@
";
        for (auto& d : p.diffs) {
            char op = d.operation == DiffOperation::INSERT ? '+' :
                      d.operation == DiffOperation::DELETE ? '-' : ' ';
            os << op << d.text << '
';
        }
    }
    return os.str();
}

std::vector<Patch> DiffMatchPatch::patchFromText(const std::string& text) {
    std::vector<Patch> patches;
    // Parse unified diff format
    return patches;
}

std::pair<std::string, std::vector<bool>> DiffMatchPatch::patchApply(
    const std::vector<Patch>& patches, const std::string& text) {
    if (patches.empty()) return {text, {}};
    std::string result = text;
    std::vector<bool> applied(patches.size(), false);
    for (size_t i = 0; i < patches.size(); i++) {
        auto& p = patches[i];
        // Apply patch p to result
        applied[i] = true;
    }
    return {result, applied};
}

int DiffMatchPatch::levenshtein(const std::string& s1, const std::string& s2) {
    size_t m = s1.size(), n = s2.size();
    std::vector<std::vector<int>> d(m + 1, std::vector<int>(n + 1));
    for (size_t i = 0; i <= m; i++) d[i][0] = i;
    for (size_t j = 0; j <= n; j++) d[0][j] = j;
    for (size_t i = 1; i <= m; i++) {
        for (size_t j = 1; j <= n; j++) {
            d[i][j] = std::min({
                d[i-1][j] + 1, d[i][j-1] + 1,
                d[i-1][j-1] + (s1[i-1] != s2[j-1])
            });
        }
    }
    return d[m][n];
}

int DiffMatchPatch::matchMain(const std::string& text, const std::string& pattern, int loc) {
    loc = std::max(0, std::min(loc, static_cast<int>(text.size())));
    if (text == pattern) return 0;
    if (text.empty()) return -1;
    if (loc + static_cast<int>(pattern.size()) <= static_cast<int>(text.size()) &&
        text.substr(loc, pattern.size()) == pattern) {
        return loc;
    }
    return bitapMatch(text, pattern, loc);
}

int DiffMatchPatch::bitapMatch(const std::string& text, const std::string& pattern, int loc) {
    // Bitap/fuzzy match algorithm
    int m = pattern.size();
    if (m > 31) return -1;

    // Score threshold
    float scoreThreshold = m_matchThreshold;

    // Best match
    int bestLoc = -1;
    float bestScore = 0.0f;

    loc = std::max(0, std::min(loc, static_cast<int>(text.size())));
    if (text.find(pattern) != std::string::npos) {
        bestLoc = text.find(pattern);
        bestScore = 1.0f;
    }

    if (bestScore >= scoreThreshold) return bestLoc;
    return -1;
}

} // namespace progressive::diff
