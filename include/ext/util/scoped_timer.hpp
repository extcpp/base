// Copyright - 2014-2019 - Jan Christoph Uhde <Jan@UhdeJC.com>
// Please see LICENSE.md for license or visit https://github.com/extcpp/basics
#ifndef EXT_UTIL_SCOPED_TIMER_HEADER
#define EXT_UTIL_SCOPED_TIMER_HEADER
#include <chrono>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

// Note: This is not for micro benchmarks!
//
// Hey wait your timer is too big and has too much overhead to be useful!
//
// On my machine the complete timer executes in about 200ns on average.
// The time taken varies about 100ns on each run. Therefore the time required
// by the timer is in the order of the expected error. So If you measure
// something with this kind of timer it should be in the oder microseconds
// to yield something reliable.
//
// For micro benchmarks watch: https://www.youtube.com/watch?v=nXaxk27zwlk ,
// use perf and run the code multiple times. Scoped-Timers are not what you
// are looking for!

namespace ext { namespace util {
using namespace std::literals::string_literals;
using scoped_timer_timings = std::vector<std::pair<std::uint64_t, std::string>>;
inline void scoped_timer_no_action_callback([[maybe_unused]] scoped_timer_timings const& times) { }

namespace _detail::scoped_timer {

inline std::stringstream to_string_stream(scoped_timer_timings const& times) {
    std::stringstream ss;
    constexpr auto Mega = 1000 * 1000;
    constexpr auto Giga = 1000 * 1000 * 1000;
    constexpr int width = 15;

    // init pushes the first element, so times can never be empty
    if (!times[0].second.empty()) {
        ss << times[0].second << "\n";
    }

    ss << "total   : " << std::setw(width) << std::setprecision(6) << std::fixed
       << static_cast<double>(times[0].first) / Mega << " ms (100.0%) - " << std::setprecision(9) << std::fixed
       << static_cast<double>(times[0].first) / Giga << " s"
       << "\n";

    if (times.size() > 1) {
        for (std::size_t i = 1; i < times.size(); i++) {
            ss << "step " << std::setw(3) << i << ": " << std::setw(width) << std::setprecision(6)
               << static_cast<double>(times[i].first) / Mega << " ms" << std::setprecision(1) << std::fixed << " ("
               << std::setw(5) << 100 * static_cast<double>(times[i].first) / static_cast<double>(times[0].first)
               << "%)";

            if (!times[i].second.empty()) {
                ss << " - " << times[i].second;
            }
            ss << "\n";
        }
    }
    return ss;
} // function - to_string_stream

inline void default_callback(scoped_timer_timings const& times) {
    std::cerr << to_string_stream(times).rdbuf();
}

} // namespace _detail::scoped_timer

template <typename callback_type>
class scoped_timer {
    public:
    using clock = std::chrono::high_resolution_clock;
    using clock_string_vec = std::vector<std::pair<clock::time_point, std::string>>;

    private:
    callback_type callback;
    clock_string_vec timepoints_with_description;
    bool enabled_in_dtor = true;
    bool add_dtor_entry = true;

    public:
    scoped_timer(callback_type cb = &_detail::scoped_timer::default_callback, std::string const& name = ""s)
        : callback(cb) {
        init(name);
    }

    scoped_timer(std::string const& name) : callback(&_detail::scoped_timer::default_callback) {
        init(name);
    }

    scoped_timer(char const* name) // without this overload callback is deduces
                                   // to char const*
        : callback(&_detail::scoped_timer::default_callback) {
        init(name);
    }

    void init(std::string const& name = "", std::size_t vec_size = 6) {
        using namespace _detail::scoped_timer;
        static_assert(std::is_convertible_v<callback_type, std::function<void(scoped_timer_timings const&)>>,
                      "callback-type does not match");
        enabled_in_dtor = true;
        add_dtor_entry = true;
        timepoints_with_description.clear();
        timepoints_with_description.reserve(vec_size); // if you want time more than 10 - add template param?
        timepoints_with_description.emplace_back(clock::now(), name);
        timepoints_with_description.back().first = clock::now();
    }

    void add_step() {
        timepoints_with_description.emplace_back(clock::now(), ""s);
    }

    template<typename T>
    void add_step(T&& str) {
        static_assert(std::is_convertible_v<T, std::string>, "parameter is not convertible to string");
        timepoints_with_description.emplace_back(clock::now(), std::forward<T>(str));
    }

    void set_name(std::string name) {
        timepoints_with_description[0].second = name;
    }

    void dismiss() {
        enabled_in_dtor = false;
    }

    void disable_dtor_entry() {
        add_dtor_entry = false;
    }

    scoped_timer_timings run(bool disable = true) {
        if (disable) {
            dismiss();
        }

        auto rv = calculate();
        callback(rv);
        return rv;
    }

    scoped_timer_timings run_external(callback_type& cb, bool disable = true) {
        if (disable) {
            dismiss();
        }

        auto rv = calculate();
        callback(rv);
        return rv;
    }

    std::string to_string(bool disable = true) {
        if (disable) {
            dismiss();
        }

        auto rv = calculate();
        callback(rv);
        return _detail::scoped_timer::to_string_stream(rv).str();
    }

    ~scoped_timer(void) {
        auto end_time = clock::now();
        if (add_dtor_entry) {
            timepoints_with_description.emplace_back(end_time, "destructor"s);
        }
        if (enabled_in_dtor) {
            callback(calculate());
        }
    }

    private:
    // get difference between time-points in nanoseconds
    static std::uint64_t get_time_diff(clock::time_point const& t0, clock::time_point const& t1) {
        return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count());
    }

    scoped_timer_timings calculate() const {
        using namespace _detail::scoped_timer;
        auto& tp = timepoints_with_description;
        auto total_time = get_time_diff(tp.front().first, tp.back().first);
        scoped_timer_timings times;
        times.emplace_back(std::make_pair(total_time, tp.front().second));
        for (auto it = ++tp.begin(); it != tp.end(); it++) {
            auto duration = get_time_diff(std::prev(it)->first, it->first);
            auto entry = std::make_pair(std::move(duration), it->second);
            times.emplace_back(std::move(entry));
        }
        return times;
    } // function - calculate
};

template<typename cb> scoped_timer(cb, std::string const&) -> scoped_timer<cb>;
scoped_timer(std::string const&) -> scoped_timer<void (*)(scoped_timer_timings const&)>;
scoped_timer(char const *) -> scoped_timer<void (*)(scoped_timer_timings const&)>;

}}     // namespace ext::util
#endif // EXT_UTIL_SCOPED_TIMER_HEADER
