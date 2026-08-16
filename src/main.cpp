#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <tlhelp32.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cctype>
#include <cstring>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;
using Clock = std::chrono::steady_clock;

namespace {

std::string Trim(std::string s) {
    auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    return s;
}

std::wstring Utf8ToWide(const std::string& s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
    if (n <= 0) return {};
    std::wstring out(static_cast<size_t>(n), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), out.data(), n);
    return out;
}

std::string WideToUtf8(const std::wstring& s) {
    if (s.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0, nullptr, nullptr);
    if (n <= 0) return {};
    std::string out(static_cast<size_t>(n), '\0');
    WideCharToMultiByte(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), out.data(), n, nullptr, nullptr);
    return out;
}

std::wstring ToLower(std::wstring s) {
    std::transform(s.begin(), s.end(), s.begin(), [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
    return s;
}

struct Ini {
    std::map<std::string, std::map<std::string, std::string>> data;

    bool Load(const fs::path& path) {
        std::ifstream in(path, std::ios::binary);
        if (!in) return false;
        std::string section = "general";
        std::string line;
        while (std::getline(in, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            line = Trim(line);
            if (line.empty() || line[0] == '#' || line[0] == ';') continue;
            if (line.front() == '[' && line.back() == ']') {
                section = Trim(line.substr(1, line.size() - 2));
                std::transform(section.begin(), section.end(), section.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                continue;
            }
            auto pos = line.find('=');
            if (pos == std::string::npos) continue;
            auto key = Trim(line.substr(0, pos));
            auto value = Trim(line.substr(pos + 1));
            std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            data[section][key] = value;
        }
        return true;
    }

    std::string Get(const std::string& sec, const std::string& key, const std::string& def = {}) const {
        auto s = data.find(sec);
        if (s == data.end()) return def;
        auto k = s->second.find(key);
        if (k == s->second.end()) return def;
        return k->second;
    }

    int GetInt(const std::string& sec, const std::string& key, int def) const {
        try { return std::stoi(Get(sec, key, std::to_string(def))); } catch (...) { return def; }
    }

    double GetDouble(const std::string& sec, const std::string& key, double def) const {
        try { return std::stod(Get(sec, key, std::to_string(def))); } catch (...) { return def; }
    }

    bool GetBool(const std::string& sec, const std::string& key, bool def) const {
        auto v = Get(sec, key, def ? "1" : "0");
        std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return v == "1" || v == "true" || v == "yes" || v == "on";
    }
};

fs::path ExeDir() {
    std::wstring buf(32768, L'\0');
    DWORD n = GetModuleFileNameW(nullptr, buf.data(), static_cast<DWORD>(buf.size()));
    buf.resize(n);
    return fs::path(buf).parent_path();
}

struct WindowInfo {
    HWND hwnd{};
    DWORD pid{};
    std::wstring title;
    int clientW{};
    int clientH{};
};

struct EnumContext {
    std::wstring needle;
    std::vector<WindowInfo> windows;
};

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lp) {
    auto* ctx = reinterpret_cast<EnumContext*>(lp);
    if (!IsWindowVisible(hwnd) || IsIconic(hwnd)) return TRUE;
    if (GetWindow(hwnd, GW_OWNER) != nullptr) return TRUE;
    wchar_t title[1024]{};
    int len = GetWindowTextW(hwnd, title, 1024);
    if (len <= 0) return TRUE;
    std::wstring t(title, static_cast<size_t>(len));
    if (!ctx->needle.empty() && ToLower(t).find(ToLower(ctx->needle)) == std::wstring::npos) return TRUE;
    RECT rc{};
    if (!GetClientRect(hwnd, &rc)) return TRUE;
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;
    if (w < 320 || h < 240) return TRUE;
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    ctx->windows.push_back(WindowInfo{hwnd, pid, t, w, h});
    return TRUE;
}

std::vector<WindowInfo> FindGameWindows(const std::wstring& titleNeedle) {
    EnumContext ctx{titleNeedle, {}};
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&ctx));
    std::sort(ctx.windows.begin(), ctx.windows.end(), [](const auto& a, const auto& b) { return a.pid < b.pid; });
    return ctx.windows;
}

bool RefreshWindowSize(WindowInfo& w) {
    if (!IsWindow(w.hwnd) || IsIconic(w.hwnd)) return false;
    RECT rc{};
    if (!GetClientRect(w.hwnd, &rc)) return false;
    w.clientW = rc.right - rc.left;
    w.clientH = rc.bottom - rc.top;
    return w.clientW > 0 && w.clientH > 0;
}

struct PixelBuffer {
    int w{};
    int h{};
    std::vector<std::uint8_t> bgra;

    const std::uint8_t* Pixel(int x, int y) const {
        if (x < 0 || y < 0 || x >= w || y >= h) return nullptr;
        return &bgra[(static_cast<size_t>(y) * static_cast<size_t>(w) + static_cast<size_t>(x)) * 4];
    }
};

bool CaptureClient(HWND hwnd, PixelBuffer& out) {
    RECT rc{};
    if (!GetClientRect(hwnd, &rc)) return false;
    const int w = rc.right - rc.left;
    const int h = rc.bottom - rc.top;
    if (w <= 0 || h <= 0) return false;

    HDC src = GetDC(hwnd);
    if (!src) return false;
    HDC mem = CreateCompatibleDC(src);
    if (!mem) { ReleaseDC(hwnd, src); return false; }

    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP bmp = CreateDIBSection(src, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!bmp || !bits) {
        if (bmp) DeleteObject(bmp);
        DeleteDC(mem);
        ReleaseDC(hwnd, src);
        return false;
    }
    HGDIOBJ old = SelectObject(mem, bmp);

    BOOL ok = PrintWindow(hwnd, mem, PW_CLIENTONLY);
    if (!ok) ok = BitBlt(mem, 0, 0, w, h, src, 0, 0, SRCCOPY | CAPTUREBLT);
    if (ok) {
        out.w = w;
        out.h = h;
        out.bgra.resize(static_cast<size_t>(w) * static_cast<size_t>(h) * 4);
        std::memcpy(out.bgra.data(), bits, out.bgra.size());
    }

    SelectObject(mem, old);
    DeleteObject(bmp);
    DeleteDC(mem);
    ReleaseDC(hwnd, src);
    return ok == TRUE;
}

struct SampleStats {
    double r{};
    double g{};
    double b{};
    double variance{};
};

std::optional<SampleStats> SamplePatch(const PixelBuffer& img, int cx, int cy, int half) {
    if (half < 1) half = 1;
    const int x0 = std::max(0, cx - half);
    const int y0 = std::max(0, cy - half);
    const int x1 = std::min(img.w - 1, cx + half);
    const int y1 = std::min(img.h - 1, cy + half);
    if (x0 >= x1 || y0 >= y1) return std::nullopt;

    double sr = 0, sg = 0, sb = 0;
    std::vector<double> lumas;
    lumas.reserve(static_cast<size_t>((x1 - x0 + 1) * (y1 - y0 + 1)));
    int n = 0;
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            auto* p = img.Pixel(x, y);
            if (!p) continue;
            double b = p[0], g = p[1], r = p[2];
            sr += r; sg += g; sb += b;
            lumas.push_back(0.2126 * r + 0.7152 * g + 0.0722 * b);
            ++n;
        }
    }
    if (!n) return std::nullopt;
    const double mr = sr / n, mg = sg / n, mb = sb / n;
    double ml = 0;
    for (double v : lumas) ml += v;
    ml /= n;
    double var = 0;
    for (double v : lumas) { double d = v - ml; var += d * d; }
    var /= n;
    return SampleStats{mr, mg, mb, var};
}

struct BagConfig {
    bool enabled{false};
    double left{0};
    double top{0};
    int cols{10};
    int rows{9};
    double stepX{0};
    double stepY{0};
    int sampleHalfPx{5};
    double colorThreshold{38.0};
    double varianceThreshold{650.0};
    int calibrationRow{8};
    int calibrationCol{9};
    std::string openMacro{"bag_open_sort"};
    std::string closeMacro{"bag_close"};
};

struct BagCalibration {
    bool valid{false};
    SampleStats empty{};
};

bool SaveBagCalibration(const fs::path& path, const BagCalibration& c) {
    std::ofstream out(path, std::ios::trunc);
    if (!out) return false;
    out << std::fixed << std::setprecision(4)
        << "r=" << c.empty.r << "\n"
        << "g=" << c.empty.g << "\n"
        << "b=" << c.empty.b << "\n"
        << "variance=" << c.empty.variance << "\n";
    return true;
}

BagCalibration LoadBagCalibration(const fs::path& path) {
    BagCalibration c;
    std::ifstream in(path);
    if (!in) return c;
    std::string line;
    std::map<std::string, double> v;
    while (std::getline(in, line)) {
        auto p = line.find('=');
        if (p == std::string::npos) continue;
        try { v[Trim(line.substr(0, p))] = std::stod(Trim(line.substr(p + 1))); } catch (...) {}
    }
    if (v.count("r") && v.count("g") && v.count("b") && v.count("variance")) {
        c.valid = true;
        c.empty = SampleStats{v["r"], v["g"], v["b"], v["variance"]};
    }
    return c;
}

class ClickEngine {
public:
    enum class Mode { Post, Send };

    explicit ClickEngine(Mode mode, int downUpMs) : mode_(mode), downUpMs_(downUpMs) {}

    bool Click(WindowInfo& w, double nx, double ny) const {
        if (!RefreshWindowSize(w)) return false;
        if (nx < 0 || nx > 1 || ny < 0 || ny > 1) return false;
        int x = std::clamp(static_cast<int>(std::lround(nx * (w.clientW - 1))), 0, w.clientW - 1);
        int y = std::clamp(static_cast<int>(std::lround(ny * (w.clientH - 1))), 0, w.clientH - 1);
        LPARAM lp = MAKELPARAM(static_cast<short>(x), static_cast<short>(y));
        if (mode_ == Mode::Post) {
            PostMessageW(w.hwnd, WM_MOUSEMOVE, 0, lp);
            if (!PostMessageW(w.hwnd, WM_LBUTTONDOWN, MK_LBUTTON, lp)) return false;
            Sleep(static_cast<DWORD>(downUpMs_));
            return PostMessageW(w.hwnd, WM_LBUTTONUP, 0, lp) != FALSE;
        }
        DWORD_PTR result = 0;
        SendMessageTimeoutW(w.hwnd, WM_MOUSEMOVE, 0, lp, SMTO_ABORTIFHUNG, 300, &result);
        if (!SendMessageTimeoutW(w.hwnd, WM_LBUTTONDOWN, MK_LBUTTON, lp, SMTO_ABORTIFHUNG, 300, &result)) return false;
        Sleep(static_cast<DWORD>(downUpMs_));
        return SendMessageTimeoutW(w.hwnd, WM_LBUTTONUP, 0, lp, SMTO_ABORTIFHUNG, 300, &result) != 0;
    }

private:
    Mode mode_;
    int downUpMs_;
};

struct MacroStep {
    enum class Kind { Click, Sleep, Grid } kind{Kind::Sleep};
    double x{}, y{}, dx{}, dy{};
    int cols{}, rows{}, count{};
    int repeat{1};
    int intervalMs{120};
    int afterMs{0};
    int sleepMs{0};
};

struct Macro {
    std::string name;
    std::vector<MacroStep> steps;
};

std::optional<Macro> LoadMacro(const fs::path& path, const std::string& name) {
    std::ifstream in(path);
    if (!in) return std::nullopt;
    Macro m{name, {}};
    std::string line;
    int lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        line = Trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;
        std::istringstream ss(line);
        std::string cmd;
        ss >> cmd;
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (cmd == "sleep") {
            MacroStep s; s.kind = MacroStep::Kind::Sleep;
            if (!(ss >> s.sleepMs)) return std::nullopt;
            m.steps.push_back(s);
        } else if (cmd == "click") {
            MacroStep s; s.kind = MacroStep::Kind::Click;
            if (!(ss >> s.x >> s.y)) return std::nullopt;
            if (!(ss >> s.repeat)) s.repeat = 1;
            if (!(ss >> s.intervalMs)) s.intervalMs = 120;
            if (!(ss >> s.afterMs)) s.afterMs = 0;
            m.steps.push_back(s);
        } else if (cmd == "grid") {
            MacroStep s; s.kind = MacroStep::Kind::Grid;
            if (!(ss >> s.x >> s.y >> s.cols >> s.rows >> s.dx >> s.dy >> s.count)) return std::nullopt;
            if (!(ss >> s.intervalMs)) s.intervalMs = 120;
            if (!(ss >> s.afterMs)) s.afterMs = 0;
            m.steps.push_back(s);
        } else {
            std::cerr << "Unknown macro command in " << path.string() << ":" << lineNo << " -> " << cmd << "\n";
            return std::nullopt;
        }
    }
    return m;
}

class MacroLibrary {
public:
    bool LoadDirectory(const fs::path& dir) {
        macros_.clear();
        if (!fs::exists(dir)) return false;
        for (const auto& e : fs::directory_iterator(dir)) {
            if (!e.is_regular_file() || e.path().extension() != ".macro") continue;
            auto name = e.path().stem().string();
            auto m = LoadMacro(e.path(), name);
            if (m) macros_[name] = std::move(*m);
            else std::cerr << "Failed to parse macro: " << e.path().string() << "\n";
        }
        return !macros_.empty();
    }

    const Macro* Get(const std::string& name) const {
        auto it = macros_.find(name);
        return it == macros_.end() ? nullptr : &it->second;
    }

    bool Run(const std::string& name, WindowInfo& w, const ClickEngine& click, std::atomic_bool* stop = nullptr, int gridClickCap = -1) const {
        const Macro* m = Get(name);
        if (!m) {
            std::cerr << "Macro not found: " << name << "\n";
            return false;
        }
        int remainingGrid = gridClickCap;
        for (const auto& s : m->steps) {
            if (stop && stop->load()) return false;
            if (s.kind == MacroStep::Kind::Sleep) {
                Sleep(static_cast<DWORD>(std::max(0, s.sleepMs)));
                continue;
            }
            if (s.kind == MacroStep::Kind::Click) {
                for (int i = 0; i < std::max(1, s.repeat); ++i) {
                    if (stop && stop->load()) return false;
                    if (!click.Click(w, s.x, s.y)) return false;
                    if (i + 1 < std::max(1, s.repeat)) Sleep(static_cast<DWORD>(std::max(0, s.intervalMs)));
                }
                if (s.afterMs > 0) Sleep(static_cast<DWORD>(s.afterMs));
                continue;
            }
            if (s.kind == MacroStep::Kind::Grid) {
                int emitted = 0;
                int limit = s.count <= 0 ? s.cols * s.rows : s.count;
                if (remainingGrid >= 0) limit = std::min(limit, remainingGrid);
                for (int r = 0; r < s.rows && emitted < limit; ++r) {
                    for (int c = 0; c < s.cols && emitted < limit; ++c) {
                        if (stop && stop->load()) return false;
                        if (!click.Click(w, s.x + c * s.dx, s.y + r * s.dy)) return false;
                        ++emitted;
                        if (emitted < limit) Sleep(static_cast<DWORD>(std::max(0, s.intervalMs)));
                    }
                }
                if (remainingGrid >= 0) remainingGrid -= emitted;
                if (s.afterMs > 0) Sleep(static_cast<DWORD>(s.afterMs));
            }
        }
        return true;
    }

    std::vector<std::string> Names() const {
        std::vector<std::string> out;
        for (const auto& [k, _] : macros_) out.push_back(k);
        return out;
    }

private:
    std::map<std::string, Macro> macros_;
};

struct BagScanResult {
    bool ok{false};
    bool uncertain{false};
    int freeSlots{-1};
    int totalSlots{0};
    int nearThreshold{0};
};

class BagScanner {
public:
    BagScanner(BagConfig cfg, BagCalibration calibration) : cfg_(cfg), cal_(calibration) {}

    void SetCalibration(const BagCalibration& c) { cal_ = c; }
    const BagCalibration& Calibration() const { return cal_; }
    const BagConfig& Config() const { return cfg_; }

    bool GeometryValid() const {
        return cfg_.enabled && cfg_.left > 0 && cfg_.top > 0 && cfg_.stepX > 0 && cfg_.stepY > 0 && cfg_.cols > 0 && cfg_.rows > 0;
    }

    std::optional<BagCalibration> Calibrate(WindowInfo& w) const {
        if (!GeometryValid()) return std::nullopt;
        PixelBuffer img;
        if (!CaptureClient(w.hwnd, img)) return std::nullopt;
        int col = std::clamp(cfg_.calibrationCol, 0, cfg_.cols - 1);
        int row = std::clamp(cfg_.calibrationRow, 0, cfg_.rows - 1);
        int cx = static_cast<int>(std::lround((cfg_.left + col * cfg_.stepX) * (img.w - 1)));
        int cy = static_cast<int>(std::lround((cfg_.top + row * cfg_.stepY) * (img.h - 1)));
        auto s = SamplePatch(img, cx, cy, cfg_.sampleHalfPx);
        if (!s) return std::nullopt;
        return BagCalibration{true, *s};
    }

    BagScanResult Scan(WindowInfo& w) const {
        BagScanResult out;
        if (!GeometryValid() || !cal_.valid) return out;
        PixelBuffer img;
        if (!CaptureClient(w.hwnd, img)) return out;
        out.totalSlots = cfg_.cols * cfg_.rows;
        int free = 0;
        int near = 0;
        for (int r = 0; r < cfg_.rows; ++r) {
            for (int c = 0; c < cfg_.cols; ++c) {
                int cx = static_cast<int>(std::lround((cfg_.left + c * cfg_.stepX) * (img.w - 1)));
                int cy = static_cast<int>(std::lround((cfg_.top + r * cfg_.stepY) * (img.h - 1)));
                auto s = SamplePatch(img, cx, cy, cfg_.sampleHalfPx);
                if (!s) return BagScanResult{};
                const double colorDist = (std::abs(s->r - cal_.empty.r) + std::abs(s->g - cal_.empty.g) + std::abs(s->b - cal_.empty.b)) / 3.0;
                const double varDist = std::abs(s->variance - cal_.empty.variance);
                const bool colorEmpty = colorDist <= cfg_.colorThreshold;
                const bool varEmpty = varDist <= cfg_.varianceThreshold;
                if (colorEmpty && varEmpty) ++free;
                if (std::abs(colorDist - cfg_.colorThreshold) <= 5.0 || std::abs(varDist - cfg_.varianceThreshold) <= 60.0) ++near;
            }
        }
        out.ok = true;
        out.freeSlots = free;
        out.nearThreshold = near;
        out.uncertain = near > std::max(3, out.totalSlots / 12);
        return out;
    }

private:
    BagConfig cfg_;
    BagCalibration cal_;
};

struct VisualSignatureConfig {
    bool enabled{false};
    double centerX{0.5};
    double centerY{0.2};
    int sampleHalfPx{8};
    double colorThreshold{20.0};
    double varianceThreshold{500.0};
    int checkIntervalMs{1000};
    int recoverySettleMs{5000};
};

struct VisualSignatureCalibration {
    bool valid{false};
    SampleStats sample{};
};

bool SaveVisualSignature(const fs::path& path, const VisualSignatureCalibration& c) {
    std::ofstream out(path, std::ios::trunc);
    if (!out) return false;
    out << std::fixed << std::setprecision(4)
        << "r=" << c.sample.r << "\n"
        << "g=" << c.sample.g << "\n"
        << "b=" << c.sample.b << "\n"
        << "variance=" << c.sample.variance << "\n";
    return true;
}

VisualSignatureCalibration LoadVisualSignature(const fs::path& path) {
    VisualSignatureCalibration c;
    std::ifstream in(path);
    if (!in) return c;
    std::map<std::string, double> v;
    std::string line;
    while (std::getline(in, line)) {
        auto p = line.find('=');
        if (p == std::string::npos) continue;
        try { v[Trim(line.substr(0, p))] = std::stod(Trim(line.substr(p + 1))); } catch (...) {}
    }
    if (v.count("r") && v.count("g") && v.count("b") && v.count("variance")) {
        c.valid = true;
        c.sample = SampleStats{v["r"], v["g"], v["b"], v["variance"]};
    }
    return c;
}

class VisualSignatureDetector {
public:
    VisualSignatureDetector(VisualSignatureConfig cfg, VisualSignatureCalibration cal)
        : cfg_(cfg), cal_(cal) {}

    bool Enabled() const { return cfg_.enabled; }
    bool Ready() const { return cfg_.enabled && cal_.valid; }
    const VisualSignatureConfig& Config() const { return cfg_; }
    void SetCalibration(const VisualSignatureCalibration& c) { cal_ = c; }

    std::optional<VisualSignatureCalibration> Calibrate(WindowInfo& w) const {
        PixelBuffer img;
        if (!CaptureClient(w.hwnd, img)) return std::nullopt;
        int cx = static_cast<int>(std::lround(cfg_.centerX * (img.w - 1)));
        int cy = static_cast<int>(std::lround(cfg_.centerY * (img.h - 1)));
        auto st = SamplePatch(img, cx, cy, cfg_.sampleHalfPx);
        if (!st) return std::nullopt;
        return VisualSignatureCalibration{true, *st};
    }

    bool Matches(WindowInfo& w) const {
        if (!Ready()) return false;
        PixelBuffer img;
        if (!CaptureClient(w.hwnd, img)) return false;
        int cx = static_cast<int>(std::lround(cfg_.centerX * (img.w - 1)));
        int cy = static_cast<int>(std::lround(cfg_.centerY * (img.h - 1)));
        auto st = SamplePatch(img, cx, cy, cfg_.sampleHalfPx);
        if (!st) return false;
        const double colorDist = (std::abs(st->r - cal_.sample.r) + std::abs(st->g - cal_.sample.g) + std::abs(st->b - cal_.sample.b)) / 3.0;
        const double varDist = std::abs(st->variance - cal_.sample.variance);
        return colorDist <= cfg_.colorThreshold && varDist <= cfg_.varianceThreshold;
    }

private:
    VisualSignatureConfig cfg_;
    VisualSignatureCalibration cal_;
};

enum class Role { Main, Child };
enum class SessionState { Idle, Training, WaitingTransfer, BusyTransfer, Selling, Recovering, Paused, Error };

struct AccountSession {
    int id{};
    Role role{Role::Child};
    int tradeSlot{0};
    WindowInfo window;
    SessionState state{SessionState::Idle};
    int freeSlots{-1};
    bool bagReliable{false};
    Clock::time_point lastBagScan{};
    Clock::time_point lastTransfer{};
    Clock::time_point lastDeathCheck{};
};

std::string StateName(SessionState s) {
    switch (s) {
        case SessionState::Idle: return "IDLE";
        case SessionState::Training: return "TRAINING";
        case SessionState::WaitingTransfer: return "WAIT_TRANSFER";
        case SessionState::BusyTransfer: return "BUSY_TRANSFER";
        case SessionState::Selling: return "SELLING";
        case SessionState::Recovering: return "RECOVERING";
        case SessionState::Paused: return "PAUSED";
        default: return "ERROR";
    }
}

struct RuntimeConfig {
    std::wstring titleNeedle;
    int bagCheckIntervalMs{300000};
    int childTriggerFreeSlots{9};
    int mainStopFreeSlots{9};
    int maxTransferClicksPerTrade{9};
    int moveSettleMs{3500};
    int tradeInviteSettleMs{900};
    int tradeWindowSettleMs{900};
    int postTradeWaitMs{1800};
    int sellSettleMs{5000};
    int loopSleepMs{250};
    bool startTrainOnLaunch{true};
    bool moveToAnchorOnLaunch{true};
    bool rescanAllAfterTrade{true};
    bool scanBeforeStart{true};
};

class Coordinator {
public:
    Coordinator(RuntimeConfig cfg, BagScanner scanner, VisualSignatureDetector death, const MacroLibrary& macros, const ClickEngine& click)
        : cfg_(std::move(cfg)), scanner_(std::move(scanner)), death_(std::move(death)), macros_(macros), click_(click) {}

    void SetSessions(std::vector<AccountSession> s) { sessions_ = std::move(s); }
    std::vector<AccountSession>& Sessions() { return sessions_; }

    void Stop() { stop_.store(true); }

    void PrintStatus() const {
        std::cout << "\n--- STATUS ---\n";
        for (const auto& s : sessions_) {
            std::cout << "#" << s.id << " " << (s.role == Role::Main ? "MAIN " : "CHILD")
                      << (s.role == Role::Child ? (" slot=" + std::to_string(s.tradeSlot)) : "")
                      << " pid=" << s.window.pid
                      << " free=" << s.freeSlots
                      << " reliable=" << (s.bagReliable ? "Y" : "N")
                      << " state=" << StateName(s.state) << "\n";
        }
    }

    bool ScanBag(AccountSession& s, bool runOpenClose = true) {
        if (!scanner_.GeometryValid() || !scanner_.Calibration().valid) {
            std::cerr << "Bag scanner not calibrated/configured.\n";
            return false;
        }
        if (runOpenClose && !macros_.Run(scanner_.Config().openMacro, s.window, click_, &stop_)) {
            std::cerr << "Open-bag macro failed for PID " << s.window.pid << "\n";
            s.bagReliable = false;
            return false;
        }
        Sleep(250);
        auto r = scanner_.Scan(s.window);
        if (runOpenClose) macros_.Run(scanner_.Config().closeMacro, s.window, click_, &stop_);
        s.lastBagScan = Clock::now();
        s.bagReliable = r.ok && !r.uncertain;
        if (r.ok) s.freeSlots = r.freeSlots;
        std::cout << "[bag] PID " << s.window.pid << " free=" << r.freeSlots << "/" << r.totalSlots
                  << " uncertain=" << (r.uncertain ? "YES" : "NO") << " near=" << r.nearThreshold << "\n";
        return s.bagReliable;
    }

    bool RescanAll() {
        bool all = true;
        for (auto& s : sessions_) {
            if (!ScanBag(s, true)) all = false;
            if (stop_.load()) return false;
        }
        return all;
    }

    void Run() {
        if (sessions_.empty()) return;
        auto* main = Main();
        if (!main) return;

        if (cfg_.scanBeforeStart) RescanAll();
        if (cfg_.moveToAnchorOnLaunch) {
            for (auto& s : sessions_) macros_.Run("move_anchor", s.window, click_, &stop_);
            Sleep(static_cast<DWORD>(std::max(0, cfg_.moveSettleMs)));
        }
        if (cfg_.startTrainOnLaunch) {
            for (auto& s : sessions_) {
                if (macros_.Run("start_train", s.window, click_, &stop_)) s.state = SessionState::Training;
            }
        }

        while (!stop_.load()) {
            if (!ValidateWindows()) {
                std::cerr << "A selected game window disappeared/minimized. Automation paused.\n";
                for (auto& s : sessions_) s.state = SessionState::Paused;
                break;
            }

            const auto now = Clock::now();
            if (death_.Ready()) {
                bool recovered = false;
                for (auto& s : sessions_) {
                    if (s.lastDeathCheck.time_since_epoch().count() != 0 &&
                        std::chrono::duration_cast<std::chrono::milliseconds>(now - s.lastDeathCheck).count() < death_.Config().checkIntervalMs) continue;
                    s.lastDeathCheck = now;
                    if (death_.Matches(s.window)) {
                        if (!Recover(s)) {
                            s.state = SessionState::Error;
                            stop_.store(true);
                            break;
                        }
                        recovered = true;
                    }
                }
                if (stop_.load()) break;
                if (recovered) { RescanAll(); continue; }
            }
            for (auto& s : sessions_) {
                if (s.lastBagScan.time_since_epoch().count() == 0 ||
                    std::chrono::duration_cast<std::chrono::milliseconds>(now - s.lastBagScan).count() >= cfg_.bagCheckIntervalMs) {
                    ScanBag(s, true);
                    if (stop_.load()) break;
                }
            }
            if (stop_.load()) break;

            main = Main();
            if (!main) break;
            if (main->bagReliable && main->freeSlots < cfg_.mainStopFreeSlots) {
                if (!SellMain(*main)) {
                    main->state = SessionState::Error;
                    break;
                }
                RescanAll();
                continue;
            }

            auto childIndex = ChooseNextChild();
            if (childIndex) {
                auto& child = sessions_[*childIndex];
                if (!main->bagReliable || !child.bagReliable) {
                    RescanAll();
                    continue;
                }
                if (main->freeSlots < cfg_.mainStopFreeSlots) continue;
                if (!Transfer(*main, child)) {
                    std::cerr << "Transfer sequence failed; forcing full rescan before any retry.\n";
                    RescanAll();
                    child.state = SessionState::Error;
                    Sleep(1000);
                } else if (cfg_.rescanAllAfterTrade) {
                    RescanAll();
                } else {
                    ScanBag(*main, true);
                    ScanBag(child, true);
                }
                continue;
            }

            Sleep(static_cast<DWORD>(std::max(50, cfg_.loopSleepMs)));
        }
        std::cout << "Coordinator stopped.\n";
    }

private:
    AccountSession* Main() {
        for (auto& s : sessions_) if (s.role == Role::Main) return &s;
        return nullptr;
    }

    bool ValidateWindows() {
        for (auto& s : sessions_) if (!RefreshWindowSize(s.window)) return false;
        return true;
    }

    std::optional<size_t> ChooseNextChild() {
        if (sessions_.size() <= 1) return std::nullopt;
        const size_t n = sessions_.size();
        for (size_t k = 0; k < n; ++k) {
            size_t i = (roundRobinCursor_ + k) % n;
            auto& s = sessions_[i];
            if (s.role != Role::Child || !s.bagReliable) continue;
            if (s.freeSlots <= cfg_.childTriggerFreeSlots) {
                roundRobinCursor_ = (i + 1) % n;
                return i;
            }
        }
        return std::nullopt;
    }

    bool Recover(AccountSession& s) {
        std::scoped_lock lk(transactionMutex_);
        std::cout << "\n[recover] PID " << s.window.pid << " death signature matched\n";
        s.state = SessionState::Recovering;
        macros_.Run("stop_train", s.window, click_, &stop_);
        if (!macros_.Run("revive_return", s.window, click_, &stop_)) return false;
        Sleep(static_cast<DWORD>(std::max(0, death_.Config().recoverySettleMs)));
        macros_.Run("move_anchor", s.window, click_, &stop_);
        Sleep(static_cast<DWORD>(std::max(0, cfg_.moveSettleMs)));
        if (!macros_.Run("start_train", s.window, click_, &stop_)) return false;
        s.state = SessionState::Training;
        return true;
    }

    bool Transfer(AccountSession& main, AccountSession& child) {
        std::scoped_lock lk(transactionMutex_);
        std::cout << "\n[trade] MAIN pid=" << main.window.pid << " <- CHILD pid=" << child.window.pid
                  << " slot=" << child.tradeSlot << "\n";
        main.state = SessionState::BusyTransfer;
        child.state = SessionState::BusyTransfer;

        if (!macros_.Run("stop_train", main.window, click_, &stop_)) return false;
        if (!macros_.Run("stop_train", child.window, click_, &stop_)) return false;
        if (!macros_.Run("move_anchor", main.window, click_, &stop_)) return false;
        if (!macros_.Run("move_anchor", child.window, click_, &stop_)) return false;
        Sleep(static_cast<DWORD>(std::max(0, cfg_.moveSettleMs)));

        const std::string inviteMacro = "trade_invite_" + std::to_string(child.tradeSlot);
        if (!macros_.Run(inviteMacro, main.window, click_, &stop_)) return false;
        Sleep(static_cast<DWORD>(std::max(0, cfg_.tradeInviteSettleMs)));
        if (!macros_.Run("trade_accept_child", child.window, click_, &stop_)) return false;
        Sleep(static_cast<DWORD>(std::max(0, cfg_.tradeWindowSettleMs)));

        const int targetBelow = std::max(0, cfg_.mainStopFreeSlots - 1);
        const int capacityClickCap = std::max(1, main.freeSlots - targetBelow);
        const int transferClickCap = std::max(1, std::min(cfg_.maxTransferClicksPerTrade, capacityClickCap));
        std::cout << "[trade] dynamic item-click cap=" << transferClickCap
                  << " (main free=" << main.freeSlots << ", stop-below=" << cfg_.mainStopFreeSlots << ")\n";
        if (!macros_.Run("trade_give_items_child", child.window, click_, &stop_, transferClickCap)) return false;
        if (!macros_.Run("trade_confirm_child", child.window, click_, &stop_)) return false;
        if (!macros_.Run("trade_confirm_main", main.window, click_, &stop_)) return false;
        Sleep(static_cast<DWORD>(std::max(0, cfg_.postTradeWaitMs)));

        child.lastTransfer = Clock::now();
        macros_.Run("start_train", child.window, click_, &stop_);
        macros_.Run("start_train", main.window, click_, &stop_);
        child.state = SessionState::Training;
        main.state = SessionState::Training;
        return true;
    }

    bool SellMain(AccountSession& main) {
        std::scoped_lock lk(transactionMutex_);
        std::cout << "\n[sell] MAIN pid=" << main.window.pid << " free=" << main.freeSlots << "\n";
        main.state = SessionState::Selling;
        if (!macros_.Run("stop_train", main.window, click_, &stop_)) return false;
        if (!macros_.Run("sell_main", main.window, click_, &stop_)) return false;
        Sleep(static_cast<DWORD>(std::max(0, cfg_.sellSettleMs)));
        if (!ScanBag(main, true)) return false;
        macros_.Run("move_anchor", main.window, click_, &stop_);
        Sleep(static_cast<DWORD>(std::max(0, cfg_.moveSettleMs)));
        macros_.Run("start_train", main.window, click_, &stop_);
        main.state = SessionState::Training;
        return true;
    }

    RuntimeConfig cfg_;
    BagScanner scanner_;
    VisualSignatureDetector death_;
    const MacroLibrary& macros_;
    const ClickEngine& click_;
    std::vector<AccountSession> sessions_;
    std::mutex transactionMutex_;
    size_t roundRobinCursor_{0};
    std::atomic_bool stop_{false};
};

std::vector<int> ParseIndexList(const std::string& s) {
    std::vector<int> out;
    std::set<int> seen;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, ',')) {
        item = Trim(item);
        if (item.empty()) continue;
        try {
            int v = std::stoi(item);
            if (!seen.count(v)) { seen.insert(v); out.push_back(v); }
        } catch (...) {}
    }
    return out;
}

void PrintWindows(const std::vector<WindowInfo>& ws) {
    std::wcout << L"\nFound windows:\n";
    for (size_t i = 0; i < ws.size(); ++i) {
        std::wcout << L"  [" << (i + 1) << L"] PID=" << ws[i].pid << L" " << ws[i].clientW << L"x" << ws[i].clientH
                   << L" title=\"" << ws[i].title << L"\"\n";
    }
}

BagConfig ReadBagConfig(const Ini& ini) {
    BagConfig c;
    c.enabled = ini.GetBool("bag", "enabled", false);
    c.left = ini.GetDouble("bag", "grid_left", 0.0);
    c.top = ini.GetDouble("bag", "grid_top", 0.0);
    c.cols = ini.GetInt("bag", "cols", 10);
    c.rows = ini.GetInt("bag", "rows", 9);
    c.stepX = ini.GetDouble("bag", "step_x", 0.0);
    c.stepY = ini.GetDouble("bag", "step_y", 0.0);
    c.sampleHalfPx = ini.GetInt("bag", "sample_half_px", 5);
    c.colorThreshold = ini.GetDouble("bag", "color_threshold", 38.0);
    c.varianceThreshold = ini.GetDouble("bag", "variance_threshold", 650.0);
    c.calibrationRow = ini.GetInt("bag", "calibration_row", c.rows - 1);
    c.calibrationCol = ini.GetInt("bag", "calibration_col", c.cols - 1);
    c.openMacro = ini.Get("bag", "open_macro", "bag_open_sort");
    c.closeMacro = ini.Get("bag", "close_macro", "bag_close");
    return c;
}

bool SaveBagGeometry(const fs::path& path, const BagConfig& c) {
    std::ofstream out(path, std::ios::trunc);
    if (!out) return false;
    out << std::fixed << std::setprecision(8)
        << "grid_left=" << c.left << "\n"
        << "grid_top=" << c.top << "\n"
        << "step_x=" << c.stepX << "\n"
        << "step_y=" << c.stepY << "\n";
    return true;
}

bool LoadBagGeometry(const fs::path& path, BagConfig& c) {
    std::ifstream in(path);
    if (!in) return false;
    std::map<std::string, double> v;
    std::string line;
    while (std::getline(in, line)) {
        auto p = line.find('=');
        if (p == std::string::npos) continue;
        try { v[Trim(line.substr(0, p))] = std::stod(Trim(line.substr(p + 1))); } catch (...) {}
    }
    if (!v.count("grid_left") || !v.count("grid_top") || !v.count("step_x") || !v.count("step_y")) return false;
    if (v["grid_left"] <= 0 || v["grid_top"] <= 0 || v["step_x"] <= 0 || v["step_y"] <= 0) return false;
    c.left = v["grid_left"];
    c.top = v["grid_top"];
    c.stepX = v["step_x"];
    c.stepY = v["step_y"];
    return true;
}

std::optional<std::pair<double, double>> CursorNormalized(WindowInfo& w) {
    if (!RefreshWindowSize(w)) return std::nullopt;
    POINT pt{};
    if (!GetCursorPos(&pt)) return std::nullopt;
    if (!ScreenToClient(w.hwnd, &pt)) return std::nullopt;
    if (pt.x < 0 || pt.y < 0 || pt.x >= w.clientW || pt.y >= w.clientH) return std::nullopt;
    double nx = w.clientW > 1 ? static_cast<double>(pt.x) / static_cast<double>(w.clientW - 1) : 0.0;
    double ny = w.clientH > 1 ? static_cast<double>(pt.y) / static_cast<double>(w.clientH - 1) : 0.0;
    return std::pair<double, double>{nx, ny};
}

bool RunBagGeometryWizard(WindowInfo& w, BagConfig& c, const fs::path& savePath) {
    std::cout << "\nBag geometry wizard (setup only; runtime automation still does not move the mouse).\n"
              << "Open the bag manually and keep the game window visible.\n";
    std::string line;
    auto capture = [&](const char* prompt) -> std::optional<std::pair<double, double>> {
        std::cout << prompt << " then press ENTER here: ";
        std::getline(std::cin, line);
        auto p = CursorNormalized(w);
        if (!p) std::cerr << "Cursor is not inside the selected MAIN client area.\n";
        return p;
    };

    auto p0 = capture("Move cursor to CENTER of the TOP-LEFT bag slot");
    if (!p0) return false;
    auto p1 = capture("Move cursor to CENTER of the NEXT slot to the RIGHT");
    if (!p1) return false;
    auto p2 = capture("Move cursor to CENTER of the NEXT slot one ROW DOWN from top-left");
    if (!p2) return false;

    const double dx = p1->first - p0->first;
    const double dy = p2->second - p0->second;
    if (dx <= 0.002 || dy <= 0.002) {
        std::cerr << "Derived step is invalid. Repeat the wizard carefully.\n";
        return false;
    }
    c.left = p0->first;
    c.top = p0->second;
    c.stepX = dx;
    c.stepY = dy;
    if (!SaveBagGeometry(savePath, c)) return false;
    std::cout << std::fixed << std::setprecision(8)
              << "Saved bag geometry: left=" << c.left << " top=" << c.top
              << " stepX=" << c.stepX << " stepY=" << c.stepY << "\n";
    return true;
}

RuntimeConfig ReadRuntimeConfig(const Ini& ini) {
    RuntimeConfig c;
    c.titleNeedle = Utf8ToWide(ini.Get("general", "window_title_contains", "Thần Long"));
    c.bagCheckIntervalMs = ini.GetInt("general", "bag_check_interval_ms", 300000);
    c.childTriggerFreeSlots = ini.GetInt("general", "child_trigger_free_slots", 9);
    c.mainStopFreeSlots = ini.GetInt("general", "main_stop_free_slots", 9);
    c.maxTransferClicksPerTrade = ini.GetInt("general", "max_transfer_clicks_per_trade", 9);
    c.moveSettleMs = ini.GetInt("general", "move_settle_ms", 3500);
    c.tradeInviteSettleMs = ini.GetInt("general", "trade_invite_settle_ms", 900);
    c.tradeWindowSettleMs = ini.GetInt("general", "trade_window_settle_ms", 900);
    c.postTradeWaitMs = ini.GetInt("general", "post_trade_wait_ms", 1800);
    c.sellSettleMs = ini.GetInt("general", "sell_settle_ms", 5000);
    c.loopSleepMs = ini.GetInt("general", "loop_sleep_ms", 250);
    c.startTrainOnLaunch = ini.GetBool("general", "start_train_on_launch", true);
    c.moveToAnchorOnLaunch = ini.GetBool("general", "move_to_anchor_on_launch", true);
    c.rescanAllAfterTrade = ini.GetBool("general", "rescan_all_after_trade", true);
    c.scanBeforeStart = ini.GetBool("general", "scan_before_start", true);
    return c;
}

VisualSignatureConfig ReadDeathConfig(const Ini& ini) {
    VisualSignatureConfig c;
    c.enabled = ini.GetBool("death", "enabled", false);
    c.centerX = ini.GetDouble("death", "center_x", 0.5);
    c.centerY = ini.GetDouble("death", "center_y", 0.2);
    c.sampleHalfPx = ini.GetInt("death", "sample_half_px", 8);
    c.colorThreshold = ini.GetDouble("death", "color_threshold", 20.0);
    c.varianceThreshold = ini.GetDouble("death", "variance_threshold", 500.0);
    c.checkIntervalMs = ini.GetInt("death", "check_interval_ms", 1000);
    c.recoverySettleMs = ini.GetInt("death", "recovery_settle_ms", 5000);
    return c;
}

bool CheckRequiredMacros(const MacroLibrary& lib, int childCount, const BagConfig& bag) {
    std::vector<std::string> required = {
        "start_train", "stop_train", "move_anchor", bag.openMacro, bag.closeMacro,
        "trade_accept_child", "trade_give_items_child", "trade_confirm_child", "trade_confirm_main", "sell_main", "revive_return"
    };
    for (int i = 1; i <= childCount; ++i) required.push_back("trade_invite_" + std::to_string(i));
    bool ok = true;
    for (const auto& r : required) {
        if (!lib.Get(r)) { std::cerr << "Missing required macro: " << r << ".macro\n"; ok = false; }
    }
    return ok;
}

} // namespace

int wmain() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    auto setDpi = reinterpret_cast<BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT)>(GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetProcessDpiAwarenessContext"));
    if (setDpi) setDpi(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    const fs::path root = ExeDir();
    Ini ini;
    const fs::path configPath = root / "config" / "tool.ini";
    if (!ini.Load(configPath)) {
        std::cerr << "Cannot load config/tool.ini next to the executable.\n";
        return 2;
    }
    RuntimeConfig cfg = ReadRuntimeConfig(ini);
    BagConfig bagCfg = ReadBagConfig(ini);
    VisualSignatureConfig deathCfg = ReadDeathConfig(ini);
    LoadBagGeometry(root / "config" / "bag_geometry.txt", bagCfg);

    ClickEngine::Mode mode = ToLower(Utf8ToWide(ini.Get("click", "mode", "post"))) == L"send" ? ClickEngine::Mode::Send : ClickEngine::Mode::Post;
    ClickEngine click(mode, ini.GetInt("click", "down_up_ms", 25));

    MacroLibrary macros;
    if (!macros.LoadDirectory(root / "macros")) {
        std::cerr << "No macros loaded from macros/*.macro\n";
        return 3;
    }

    auto windows = FindGameWindows(cfg.titleNeedle);
    if (windows.size() < 2) {
        std::wcerr << L"Need at least 2 visible game windows matching title: " << cfg.titleNeedle << L"\n";
        PrintWindows(windows);
        return 4;
    }
    PrintWindows(windows);

    std::cout << "\nChoose MAIN window index: ";
    std::string line;
    std::getline(std::cin, line);
    int mainIndex = 0;
    try { mainIndex = std::stoi(Trim(line)); } catch (...) { mainIndex = 0; }
    if (mainIndex < 1 || mainIndex > static_cast<int>(windows.size())) return 5;

    std::cout << "Choose CHILD window indices, comma-separated (max 6, order = trade slot 1..6): ";
    std::getline(std::cin, line);
    auto childIndices = ParseIndexList(line);
    childIndices.erase(std::remove(childIndices.begin(), childIndices.end(), mainIndex), childIndices.end());
    childIndices.erase(std::remove_if(childIndices.begin(), childIndices.end(), [&](int i) { return i < 1 || i > static_cast<int>(windows.size()); }), childIndices.end());
    if (childIndices.empty() || childIndices.size() > 6) {
        std::cerr << "Choose 1..6 child windows.\n";
        return 6;
    }

    if (!CheckRequiredMacros(macros, static_cast<int>(childIndices.size()), bagCfg)) return 7;

    BagCalibration cal = LoadBagCalibration(root / "config" / "bag_calibration.txt");

    std::vector<AccountSession> sessions;
    sessions.push_back(AccountSession{1, Role::Main, 0, windows[static_cast<size_t>(mainIndex - 1)]});
    int sid = 2;
    int slot = 1;
    for (int idx : childIndices) {
        sessions.push_back(AccountSession{sid++, Role::Child, slot++, windows[static_cast<size_t>(idx - 1)]});
    }

    std::cout << "\nThanLong Item Consolidator v0.1.0\n"
              << "Action mode: background window messages only. No internal Game/Lua/packet calls.\n"
              << "Coordinates in macros are normalized 0..1 and rescaled on every click.\n\n";

    if (!(bagCfg.enabled && bagCfg.left > 0 && bagCfg.top > 0 && bagCfg.stepX > 0 && bagCfg.stepY > 0)) {
        std::cout << "Bag geometry has not been calibrated yet. Run the mouse-assisted setup wizard now? [Y/n]: ";
        std::getline(std::cin, line);
        if (!Trim(line).empty() && Trim(line) != "y" && Trim(line) != "Y") return 8;
        if (!RunBagGeometryWizard(sessions[0].window, bagCfg, root / "config" / "bag_geometry.txt")) return 8;
    }

    BagScanner scanner(bagCfg, cal);
    if (!scanner.GeometryValid()) return 8;

    if (!cal.valid) {
        std::cout << "No bag calibration found.\n"
                  << "Open/sort macro will run on MAIN. Ensure configured calibration_row/calibration_col points to an EMPTY slot.\n"
                  << "Press ENTER to capture the empty-slot visual signature, or type q to exit: ";
        std::getline(std::cin, line);
        if (Trim(line) == "q" || Trim(line) == "Q") return 0;
        auto& w = sessions[0].window;
        if (!macros.Run(bagCfg.openMacro, w, click)) {
            std::cerr << "Cannot run bag-open macro for calibration.\n";
            return 9;
        }
        Sleep(300);
        auto newCal = scanner.Calibrate(w);
        macros.Run(bagCfg.closeMacro, w, click);
        if (!newCal || !SaveBagCalibration(root / "config" / "bag_calibration.txt", *newCal)) {
            std::cerr << "Calibration failed.\n";
            return 10;
        }
        scanner.SetCalibration(*newCal);
        std::cout << "Calibration saved to config/bag_calibration.txt\n";
    }

    auto deathCal = LoadVisualSignature(root / "config" / "death_calibration.txt");
    VisualSignatureDetector deathDetector(deathCfg, deathCal);
    if (deathCfg.enabled && !deathCal.valid) {
        std::cout << "Death detector is ENABLED but not calibrated. Put MAIN in the dead/revive screen, ensure [death] center_x/center_y targets a stable UI patch, then press ENTER to capture; type q to disable for this run: ";
        std::getline(std::cin, line);
        if (Trim(line) == "q" || Trim(line) == "Q") {
            deathCfg.enabled = false;
            deathDetector = VisualSignatureDetector(deathCfg, deathCal);
        } else {
            auto d = deathDetector.Calibrate(sessions[0].window);
            if (!d || !SaveVisualSignature(root / "config" / "death_calibration.txt", *d)) {
                std::cerr << "Death signature calibration failed.\n";
                return 11;
            }
            deathDetector.SetCalibration(*d);
            std::cout << "Death signature saved to config/death_calibration.txt\n";
        }
    }

    Coordinator coordinator(cfg, scanner, deathDetector, macros, click);
    coordinator.SetSessions(std::move(sessions));

    std::cout << "Press ENTER to START. During runtime press Ctrl+C to terminate.\n"
              << "Before unattended use, test each macro one-by-one with safe delays.\n";
    std::getline(std::cin, line);
    coordinator.Run();
    coordinator.PrintStatus();
    return 0;
}
