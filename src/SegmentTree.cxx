#include "SegmentTree.hxx"

#include <algorithm>
#include <sstream>
#include <iomanip>

time_t parse_timestamp(const std::string& ts_str, const std::string& format_str)
{
    // Empty format string means the captured group is a raw Unix epoch integer.
    if(format_str.empty())
    {
        try
        {
            return static_cast<time_t>(std::stoll(ts_str));
        }
        catch(...)
        {
            return static_cast<time_t>(-1);
        }
    }

    // ── Greedy-fix for %y%m%d with no separator ───────────────────────
    // MinGW's std::get_time reads as many digits as possible for each
    // numeric specifier when there is no separator between them.  When the
    // format starts with %y%m%d (MySQL Error Log: "240101 10:00:00"),
    // %y consumes all 6 leading digits and leaves nothing for %m / %d.
    // Fix: insert dashes so the parser sees "%y-%m-%d…" instead.
    if(format_str.size() >= 6 && format_str.substr(0, 6) == "%y%m%d")
    {
        if(ts_str.size() >= 6)
        {
            bool all_digits = true;
            for(int i = 0; i < 6; ++i)
                if(!std::isdigit((unsigned char)ts_str[i]))
                { all_digits = false; break; }

            if(all_digits)
            {
                std::string patched_ts  = ts_str.substr(0, 2) + "-"
                                        + ts_str.substr(2, 2) + "-"
                                        + ts_str.substr(4, 2)
                                        + ts_str.substr(6);

                std::string patched_fmt = "%y-%m-%d" + format_str.substr(6);

                std::tm tm2 = {};
                std::istringstream ss2(patched_ts);
                ss2 >> std::get_time(&tm2, patched_fmt.c_str());

                if(!ss2.fail())
                {
                    if(tm2.tm_year == 0)
                        tm2.tm_year = 124;   // 2024 − 1900

                    tm2.tm_isdst = -1;
                    return std::mktime(&tm2);
                }
            }
        }
    }
    // ─────────────────────────────────────────────────────────────────

    std::tm tm_buf = {};

    std::istringstream ss(ts_str);
    ss >> std::get_time(&tm_buf, format_str.c_str());

    if(ss.fail())
        return static_cast<time_t>(-1);

    // Year-less formats (Syslog, Android, Proxifier) leave tm_year = 0 (= 1900).
    // Windows mktime() rejects dates before 1970, so pin to a fixed reference
    // year.  Both index-building and query use the same function, so range
    // comparisons stay consistent.
    if(tm_buf.tm_year == 0)
        tm_buf.tm_year = 124;   // 2024 − 1900

    tm_buf.tm_isdst = -1;

    return std::mktime(&tm_buf);
}

void SegmentTree::build(std::vector<TimestampEntry>& entries)
{
    if(entries.empty())
        return;

    sorted_entries = entries;

    std::sort(sorted_entries.begin(), sorted_entries.end(),
        [](const TimestampEntry& a, const TimestampEntry& b)
        {
            return a.timestamp < b.timestamp;
        }
    );

    n = sorted_entries.size();

    // 1-indexed binary tree: node i has children 2i and 2i+1.
    // 4*n nodes is sufficient for any N.
    seg_min.assign(4 * n, std::numeric_limits<time_t>::max());
    seg_max.assign(4 * n, std::numeric_limits<time_t>::min());

    build_tree(1, 0, n - 1);
}

void SegmentTree::build_tree(size_t node, size_t lo, size_t hi)
{
    if(lo == hi)
    {
        seg_min[node] = sorted_entries[lo].timestamp;
        seg_max[node] = sorted_entries[lo].timestamp;
        return;
    }

    size_t mid   = lo + (hi - lo) / 2;
    size_t left  = 2 * node;
    size_t right = 2 * node + 1;

    build_tree(left,  lo,      mid);
    build_tree(right, mid + 1, hi);

    seg_min[node] = std::min(seg_min[left], seg_min[right]);
    seg_max[node] = std::max(seg_max[left], seg_max[right]);
}

std::vector<size_t> SegmentTree::range_query(time_t t_start, time_t t_end) const
{
    std::vector<size_t> result;

    if(!is_built() || t_start > t_end)
        return result;

    result.reserve(128);

    query(1, 0, n - 1, t_start, t_end, result);

    return result;
}

void SegmentTree::query(size_t node, size_t lo, size_t hi,
                        time_t t_start, time_t t_end,
                        std::vector<size_t>& result) const
{
    // Prune: entire subtree is outside the query range
    if(seg_max[node] < t_start || seg_min[node] > t_end)
        return;

    // Leaf: check this single entry
    if(lo == hi)
    {
        if(sorted_entries[lo].timestamp >= t_start &&
           sorted_entries[lo].timestamp <= t_end)
        {
            result.push_back(sorted_entries[lo].logID);
        }
        return;
    }

    size_t mid   = lo + (hi - lo) / 2;
    size_t left  = 2 * node;
    size_t right = 2 * node + 1;

    query(left,  lo,      mid, t_start, t_end, result);
    query(right, mid + 1, hi,  t_start, t_end, result);
}
