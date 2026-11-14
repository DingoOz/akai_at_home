// Turns MPK knob messages into a 0..127 setting.
// Endless knobs in "relative" mode send steps (1 = +1, 127 = -1) rather than a position;
// reading those as positions makes a tiny turn jump between 0 and 127.
#pragma once
#include <algorithm>
#include <array>
#include <cstdlib>

struct KnobReader {
    enum Mode { Auto, Absolute, Relative };
    Mode mode = Auto;
    int speed = 2;  // setting change per relative click

    int read(int cc, int v, int current) {
        int prev = last[cc];
        last[cc] = v;
        bool step = v <= 15 || v >= 113;  // what a relative knob sends
        // Same small value twice in a row, or a 1 <-> 127 flip, can only come from a relative knob.
        if (mode == Auto && prev >= 0 && step && (v == prev || std::abs(v - prev) > 100)) relative[cc] = true;
        if (mode == Absolute) return v;
        if (mode == Auto && !relative[cc])
            return step && std::abs(v - current) > 20 ? current : v;  // not sure yet: don't jump
        int delta = v == 64 ? 0 : v < 64 ? v : v - 128;
        return std::clamp(current + delta * speed, 0, 127);
    }

private:
    std::array<int, 128> last = [] { std::array<int, 128> a; a.fill(-1); return a; }();
    std::array<bool, 128> relative{};
};
