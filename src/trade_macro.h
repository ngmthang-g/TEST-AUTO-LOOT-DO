#pragma once
#include <windows.h>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace itemtrade {

enum class Op { Sleep, Click, Grid };

struct Step {
    Op op = Op::Sleep;
    double x = 0.0;
    double y = 0.0;
    int repeat = 1;
    int intervalMs = 80;
    int afterMs = 0;
    int sleepMs = 0;
    int cols = 0;
    int rows = 0;
    double stepX = 0.0;
    double stepY = 0.0;
    int count = 0;
};

struct MacroDef {
    std::string name;
    std::vector<Step> steps;
    bool valid = false;
    std::wstring error;
};

inline std::string TrimAscii(std::string s) {
    auto ok = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), ok));
    s.erase(std::find_if(s.rbegin(), s.rend(), ok).base(), s.end());
    return s;
}

inline std::wstring AsciiToWide(const std::string& s) {
    return std::wstring(s.begin(), s.end());
}

inline bool ReadAllUtf8(const std::wstring& path, std::string& out) {
    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;
    LARGE_INTEGER sz{};
    if (!GetFileSizeEx(h, &sz) || sz.QuadPart < 0 || sz.QuadPart > 4 * 1024 * 1024) {
        CloseHandle(h); return false;
    }
    out.assign(static_cast<std::size_t>(sz.QuadPart), '\0');
    DWORD done = 0;
    bool ok = out.empty() || ReadFile(h, out.data(), static_cast<DWORD>(out.size()), &done, nullptr) != FALSE;
    CloseHandle(h);
    if (!ok) return false;
    out.resize(done);
    if (out.size() >= 3 && static_cast<unsigned char>(out[0]) == 0xEF &&
        static_cast<unsigned char>(out[1]) == 0xBB && static_cast<unsigned char>(out[2]) == 0xBF) {
        out.erase(0, 3);
    }
    return true;
}

class MacroLibrary {
public:
    explicit MacroLibrary(std::wstring dir = {}) : dir_(std::move(dir)) {}

    void SetDirectory(std::wstring dir) { dir_ = std::move(dir); cache_.clear(); }
    void Reload() { cache_.clear(); }

    const MacroDef* Get(const std::string& name) {
        auto it = cache_.find(name);
        if (it != cache_.end()) return &it->second;
        MacroDef def{};
        def.name = name;
        Parse(name, def);
        auto inserted = cache_.emplace(name, std::move(def));
        return &inserted.first->second;
    }

private:
    void Parse(const std::string& name, MacroDef& def) {
        if (dir_.empty()) { def.error = L"Chưa có thư mục macros"; return; }
        const std::wstring path = dir_ + L"\\" + AsciiToWide(name) + L".macro";
        std::string text;
        if (!ReadAllUtf8(path, text)) {
            def.error = L"Thiếu " + AsciiToWide(name) + L".macro";
            return;
        }
        std::istringstream input(text);
        std::string line;
        int lineNo = 0;
        while (std::getline(input, line)) {
            ++lineNo;
            if (!line.empty() && line.back() == '\r') line.pop_back();
            line = TrimAscii(line);
            if (line.empty() || line[0] == '#' || line[0] == ';') continue;
            if (line == "UNCONFIGURED") {
                def.error = L"Macro " + AsciiToWide(name) + L" chưa cấu hình";
                return;
            }
            std::istringstream ss(line);
            std::string cmd;
            ss >> cmd;
            if (cmd == "sleep") {
                Step s{}; s.op = Op::Sleep;
                if (!(ss >> s.sleepMs) || s.sleepMs < 0 || s.sleepMs > 600000) {
                    def.error = L"Sai sleep ở dòng " + std::to_wstring(lineNo); return;
                }
                def.steps.push_back(s);
            } else if (cmd == "click") {
                Step s{}; s.op = Op::Click;
                if (!(ss >> s.x >> s.y)) { def.error = L"Sai click ở dòng " + std::to_wstring(lineNo); return; }
                if (!(ss >> s.repeat)) s.repeat = 1;
                if (!(ss >> s.intervalMs)) s.intervalMs = 80;
                if (!(ss >> s.afterMs)) s.afterMs = 0;
                if (s.x < 0 || s.x > 1 || s.y < 0 || s.y > 1 || s.repeat < 1 || s.repeat > 999 ||
                    s.intervalMs < 0 || s.intervalMs > 60000 || s.afterMs < 0 || s.afterMs > 600000) {
                    def.error = L"Click ngoài giới hạn ở dòng " + std::to_wstring(lineNo); return;
                }
                def.steps.push_back(s);
            } else if (cmd == "grid") {
                Step s{}; s.op = Op::Grid;
                if (!(ss >> s.x >> s.y >> s.cols >> s.rows >> s.stepX >> s.stepY >> s.count)) {
                    def.error = L"Sai grid ở dòng " + std::to_wstring(lineNo); return;
                }
                if (!(ss >> s.intervalMs)) s.intervalMs = 80;
                if (!(ss >> s.afterMs)) s.afterMs = 0;
                if (s.x < 0 || s.x > 1 || s.y < 0 || s.y > 1 || s.cols < 1 || s.rows < 1 ||
                    s.stepX < 0 || s.stepY < 0 || s.count < 1 || s.count > s.cols * s.rows ||
                    s.intervalMs < 0 || s.intervalMs > 60000 || s.afterMs < 0 || s.afterMs > 600000) {
                    def.error = L"Grid ngoài giới hạn ở dòng " + std::to_wstring(lineNo); return;
                }
                def.steps.push_back(s);
            } else {
                def.error = L"Lệnh macro không hỗ trợ ở dòng " + std::to_wstring(lineNo) + L": " + AsciiToWide(cmd);
                return;
            }
        }
        if (def.steps.empty()) { def.error = L"Macro rỗng: " + AsciiToWide(name); return; }
        def.valid = true;
    }

    std::wstring dir_;
    std::map<std::string, MacroDef> cache_;
};

class BackgroundClicker {
public:
    bool Click(HWND hwnd, double nx, double ny, std::wstring& error) const {
        if (!hwnd || !IsWindow(hwnd)) { error = L"Cửa sổ game đã mất"; return false; }
        if (IsIconic(hwnd)) { error = L"Cửa sổ game đang minimized"; return false; }
        RECT rc{};
        if (!GetClientRect(hwnd, &rc)) { error = L"Không đọc được client rect"; return false; }
        const int w = rc.right - rc.left;
        const int h = rc.bottom - rc.top;
        if (w <= 1 || h <= 1) { error = L"Client size không hợp lệ"; return false; }
        const int x = std::clamp(static_cast<int>(nx * static_cast<double>(w - 1) + 0.5), 0, w - 1);
        const int y = std::clamp(static_cast<int>(ny * static_cast<double>(h - 1) + 0.5), 0, h - 1);
        const LPARAM p = MAKELPARAM(x, y);
        (void)PostMessageW(hwnd, WM_MOUSEMOVE, 0, p);
        if (!PostMessageW(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, p)) { error = L"PostMessage LEFTDOWN thất bại"; return false; }
        Sleep(25);
        if (!PostMessageW(hwnd, WM_LBUTTONUP, 0, p)) { error = L"PostMessage LEFTUP thất bại"; return false; }
        return true;
    }
};

enum class RunResult { Running, Done, Failed };

class MacroRunner {
public:
    bool Start(const MacroDef* def, int clickCap = 0) {
        def_ = def;
        step_ = 0;
        repeated_ = 0;
        gridIndex_ = 0;
        due_ = 0;
        clicksDone_ = 0;
        clickCap_ = clickCap;
        error_.clear();
        return def_ && def_->valid;
    }

    void Reset() {
        def_ = nullptr; step_ = 0; repeated_ = 0; gridIndex_ = 0; due_ = 0;
        clicksDone_ = 0; clickCap_ = 0; error_.clear();
    }

    const std::wstring& Error() const { return error_; }
    int ClicksDone() const { return clicksDone_; }

    RunResult Tick(HWND hwnd, DWORD now, const BackgroundClicker& clicker) {
        if (!def_ || !def_->valid) { error_ = L"Macro runner chưa có macro hợp lệ"; return RunResult::Failed; }
        if (step_ >= def_->steps.size()) return RunResult::Done;
        if (due_ != 0 && static_cast<LONG>(now - due_) < 0) return RunResult::Running;
        due_ = 0;
        const Step& s = def_->steps[step_];
        if (s.op == Op::Sleep) {
            if (repeated_ == 0) {
                repeated_ = 1;
                due_ = now + static_cast<DWORD>(s.sleepMs);
                return RunResult::Running;
            }
            repeated_ = 0;
            ++step_;
            return step_ >= def_->steps.size() ? RunResult::Done : RunResult::Running;
        }

        if (clickCap_ > 0 && clicksDone_ >= clickCap_) {
            return RunResult::Done;
        }

        double nx = s.x, ny = s.y;
        if (s.op == Op::Grid) {
            const int col = gridIndex_ % s.cols;
            const int row = gridIndex_ / s.cols;
            nx = s.x + static_cast<double>(col) * s.stepX;
            ny = s.y + static_cast<double>(row) * s.stepY;
            if (nx < 0 || nx > 1 || ny < 0 || ny > 1) {
                error_ = L"Grid sinh tọa độ ngoài cửa sổ"; return RunResult::Failed;
            }
        }
        if (!clicker.Click(hwnd, nx, ny, error_)) return RunResult::Failed;
        ++clicksDone_;

        if (s.op == Op::Click) {
            ++repeated_;
            if (repeated_ >= s.repeat) {
                repeated_ = 0; ++step_; due_ = now + static_cast<DWORD>(s.afterMs);
            } else {
                due_ = now + static_cast<DWORD>(s.intervalMs);
            }
        } else {
            ++gridIndex_;
            if (gridIndex_ >= s.count) {
                gridIndex_ = 0; ++step_; due_ = now + static_cast<DWORD>(s.afterMs);
            } else {
                due_ = now + static_cast<DWORD>(s.intervalMs);
            }
        }
        return step_ >= def_->steps.size() ? RunResult::Done : RunResult::Running;
    }

private:
    const MacroDef* def_ = nullptr;
    std::size_t step_ = 0;
    int repeated_ = 0;
    int gridIndex_ = 0;
    DWORD due_ = 0;
    int clicksDone_ = 0;
    int clickCap_ = 0;
    std::wstring error_;
};

} // namespace itemtrade
