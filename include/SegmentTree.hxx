#pragma once

#include <vector>
#include <ctime>
#include <string>
#include <limits>

/**
 *  TimestampEntry
 *  A parsed timestamp paired with its log line ID.
 */
struct TimestampEntry
{
    time_t timestamp;
    size_t logID;
};

/**
 *  SegmentTree
 *  A min-max segment tree built over a sorted set of TimestampEntry values.
 *  Answers time-range queries in O(log N + results).
 */
class SegmentTree
{
private:

    std::vector<TimestampEntry> sorted_entries;
    std::vector<time_t>         seg_min;
    std::vector<time_t>         seg_max;
    size_t                      n = 0;

    /**
     *  build_tree
     *  Recursively fills seg_min and seg_max for the subtree at node,
     *  covering sorted_entries[lo..hi].
     */
    void build_tree(size_t node, size_t lo, size_t hi);

    /**
     *  query
     *  Recursively collects logIDs in [t_start, t_end], pruning subtrees
     *  whose [seg_min, seg_max] does not overlap the query range.
     */
    void query(size_t node, size_t lo, size_t hi,
               time_t t_start, time_t t_end,
               std::vector<size_t>& result) const;

public:

    /**
     *  build
     *  Sort entries by timestamp and construct the segment tree.
     */
    void build(std::vector<TimestampEntry>& entries);

    /**
     *  range_query
     *  Return all logIDs whose timestamp falls in [t_start, t_end] inclusive.
     */
    std::vector<size_t> range_query(time_t t_start, time_t t_end) const;

    /**
     *  is_built
     *  Returns true if build() has been called with at least one entry.
     */
    inline bool is_built() const
    {
        return !sorted_entries.empty();
    }

    /**
     *  size
     *  Number of timestamped entries indexed.
     */
    inline size_t size() const
    {
        return sorted_entries.size();
    }
};

/**
 *  parse_timestamp
 *  Parse a timestamp string using a strftime-style format and return time_t.
 *  Returns -1 on failure.
 */
time_t parse_timestamp(const std::string& ts_str, const std::string& format_str);
