/**
 *  test_formats.cxx
 *  Automated regression tests for all 13 timestamp format presets.
 *
 *  For each preset the test:
 *    1. Parses 10 synthetic log lines (timestamps 10 s apart).
 *    2. Builds a SegmentTree.
 *    3. Verifies three range queries:
 *         a. All-range  (lines 0–9)   → 10 results
 *         b. Mid-range  (lines 2–6)   →  5 results
 *         c. Out-of-range             →  0 results
 *
 *  Usage:  .\build\test_formats.exe
 *  Exit 0 = all pass,  Exit 1 = at least one failure.
 */
#include "SegmentTree.hxx"

#include <cstdio>
#include <cstring>
#include <iomanip>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

// ── Test counters ────────────────────────────────────────────────────────

static int g_passed = 0;
static int g_failed = 0;

// ── Helpers ──────────────────────────────────────────────────────────────

static void check(
    const char* preset,
    const char* label,
    std::size_t got,
    std::size_t expected)
{
    if(got == expected)
    {
        std::printf("  [PASS] %-42s  %s\n", label, preset);
        ++g_passed;
    }
    else
    {
        std::printf("  [FAIL] %-42s  %s  (got %zu, expected %zu)\n",
                    label, preset, got, expected);
        ++g_failed;
    }
}

/**
 *  build_tree_from_lines
 *  Scans each log line with the preset regex, parses the captured timestamp,
 *  and inserts a TimestampEntry.  Returns a fully constructed SegmentTree.
 */
static SegmentTree build_tree_from_lines(
    const std::vector<std::string>& lines,
    const char*                     regex_str,
    const char*                     format_str)
{
    std::regex               re(regex_str);
    std::smatch              m;
    std::vector<TimestampEntry> entries;

    for(std::size_t i = 0; i < lines.size(); ++i)
    {
        if(std::regex_search(lines[i], m, re) && m.size() >= 2)
        {
            time_t ts = parse_timestamp(m[1].str(), format_str);

            if(ts != static_cast<time_t>(-1))
                entries.push_back({ts, i});
        }
    }

    SegmentTree tree;
    tree.build(entries);
    return tree;
}

/**
 *  run_test
 *  Core test driver for one preset.
 *  Lines must be in chronological order with ts_* strings parseable by FORMAT.
 *
 *  @param preset       Human-readable preset name for output.
 *  @param regex_str    The preset's ECMAScript regex.
 *  @param format_str   The preset's strftime format (empty = epoch).
 *  @param lines        Exactly 10 log lines, timestamps 10 s apart.
 *  @param ts_line0     Timestamp string of line 0  (all-range lower bound).
 *  @param ts_line2     Timestamp string of line 2  (mid-range lower bound).
 *  @param ts_line6     Timestamp string of line 6  (mid-range upper bound).
 *  @param ts_line9     Timestamp string of line 9  (all-range upper bound).
 *  @param ts_after     Timestamp string clearly past line 9 (empty-range test).
 */
static void run_test(
    const char*                     preset,
    const char*                     regex_str,
    const char*                     format_str,
    const std::vector<std::string>& lines,
    const char*                     ts_line0,
    const char*                     ts_line2,
    const char*                     ts_line6,
    const char*                     ts_line9,
    const char*                     ts_after)
{
    std::printf("\n[%s]\n", preset);

    SegmentTree tree = build_tree_from_lines(lines, regex_str, format_str);

    check(preset, "index: 10 lines parsed",
          tree.size(), 10u);

    if(!tree.is_built())
    {
        std::printf("  (skipping queries – index is empty)\n");
        g_failed += 3;     // fail the three query sub-tests
        return;
    }

    const std::string fmt(format_str);

    time_t t0    = parse_timestamp(ts_line0, fmt);
    time_t t2    = parse_timestamp(ts_line2, fmt);
    time_t t6    = parse_timestamp(ts_line6, fmt);
    time_t t9    = parse_timestamp(ts_line9, fmt);
    time_t taft  = parse_timestamp(ts_after, fmt);

    // a) All-range: lines 0-9 → 10
    check(preset, "all-range  (lines 0-9)   → 10",
          tree.range_query(t0, t9).size(),  10u);

    // b) Mid-range: lines 2-6 → 5
    check(preset, "mid-range  (lines 2-6)   →  5",
          tree.range_query(t2, t6).size(),   5u);

    // c) Out-of-range: after last line → 0
    check(preset, "out-of-range (past last) →  0",
          tree.range_query(taft, taft + 3600).size(), 0u);
}

// ── Test definitions ─────────────────────────────────────────────────────

int main()
{
    std::printf("=== Timestamp Format Regression Tests ===\n");

    // ── 1. Apache Error Log ──────────────────────────────────────────────
    run_test(
        "Apache Error Log",
        "\\[[A-Za-z]{3} ([A-Za-z]{3}\\s+\\d{1,2} \\d{2}:\\d{2}:\\d{2} \\d{4})\\]",
        "%b %d %H:%M:%S %Y",
        {
            "[Thu Jan  1 10:00:00 2024] [notice] Server started",
            "[Thu Jan  1 10:00:10 2024] [notice] LDAP loaded",
            "[Thu Jan  1 10:00:20 2024] [notice] suEXEC enabled",
            "[Thu Jan  1 10:00:30 2024] [notice] Digest done",
            "[Thu Jan  1 10:00:40 2024] [notice] Apache ready",
            "[Thu Jan  1 10:00:50 2024] [notice] Worker spawned",
            "[Thu Jan  1 10:01:00 2024] [error]  Connection refused",
            "[Thu Jan  1 10:01:10 2024] [warn]   Slow request",
            "[Thu Jan  1 10:01:20 2024] [notice] Graceful shutdown",
            "[Thu Jan  1 10:01:30 2024] [notice] Stopped",
        },
        "Jan  1 10:00:00 2024",
        "Jan  1 10:00:20 2024",
        "Jan  1 10:01:00 2024",
        "Jan  1 10:01:30 2024",
        "Jan  1 10:02:00 2024");

    // ── 2. Apache / Nginx Access Log ─────────────────────────────────────
    run_test(
        "Apache / Nginx Access Log",
        "\\[(\\d{2}/[A-Za-z]{3}/\\d{4}:\\d{2}:\\d{2}:\\d{2})",
        "%d/%b/%Y:%H:%M:%S",
        {
            "127.0.0.1 - alice [01/Jan/2024:10:00:00 +0530] \"GET /\" 200 1024",
            "127.0.0.1 - alice [01/Jan/2024:10:00:10 +0530] \"GET /img\" 200 2048",
            "127.0.0.1 - bob   [01/Jan/2024:10:00:20 +0530] \"POST /login\" 302 0",
            "127.0.0.1 - bob   [01/Jan/2024:10:00:30 +0530] \"GET /dashboard\" 200 4096",
            "127.0.0.1 - eve   [01/Jan/2024:10:00:40 +0530] \"GET /admin\" 403 256",
            "127.0.0.1 - alice [01/Jan/2024:10:00:50 +0530] \"GET /api\" 200 512",
            "127.0.0.1 - alice [01/Jan/2024:10:01:00 +0530] \"POST /cart\" 200 128",
            "127.0.0.1 - bob   [01/Jan/2024:10:01:10 +0530] \"GET /checkout\" 200 1024",
            "127.0.0.1 - alice [01/Jan/2024:10:01:20 +0530] \"POST /order\" 200 256",
            "127.0.0.1 - alice [01/Jan/2024:10:01:30 +0530] \"GET /logout\" 302 0",
        },
        "01/Jan/2024:10:00:00",
        "01/Jan/2024:10:00:20",
        "01/Jan/2024:10:01:00",
        "01/Jan/2024:10:01:30",
        "01/Jan/2024:10:02:00");

    // ── 3. ISO 8601 with T ───────────────────────────────────────────────
    run_test(
        "ISO 8601 with T",
        "(\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2})",
        "%Y-%m-%dT%H:%M:%S",
        {
            "2024-01-01T10:00:00Z INFO  Server started",
            "2024-01-01T10:00:10Z INFO  DB connected",
            "2024-01-01T10:00:20Z INFO  User login",
            "2024-01-01T10:00:30Z WARN  High CPU: 85%",
            "2024-01-01T10:00:40Z INFO  Backup started",
            "2024-01-01T10:00:50Z INFO  Backup done",
            "2024-01-01T10:01:00Z ERROR API timeout",
            "2024-01-01T10:01:10Z INFO  Retry ok",
            "2024-01-01T10:01:20Z INFO  Deployment done",
            "2024-01-01T10:01:30Z INFO  Shutdown",
        },
        "2024-01-01T10:00:00",
        "2024-01-01T10:00:20",
        "2024-01-01T10:01:00",
        "2024-01-01T10:01:30",
        "2024-01-01T10:02:00");

    // ── 4. Datetime with space ────────────────────────────────────────────
    run_test(
        "Datetime with space",
        "(\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2})",
        "%Y-%m-%d %H:%M:%S",
        {
            "2024-01-01 10:00:00,123 INFO  Server started",
            "2024-01-01 10:00:10,456 INFO  DB connected",
            "2024-01-01 10:00:20,789 INFO  User login",
            "2024-01-01 10:00:30,012 WARN  Slow query: 3200ms",
            "2024-01-01 10:00:40,345 INFO  Cache warmed",
            "2024-01-01 10:00:50,678 INFO  Cache miss",
            "2024-01-01 10:01:00,901 ERROR OOM",
            "2024-01-01 10:01:10,234 INFO  GC done",
            "2024-01-01 10:01:20,567 INFO  Restarting",
            "2024-01-01 10:01:30,890 INFO  Ready",
        },
        "2024-01-01 10:00:00",
        "2024-01-01 10:00:20",
        "2024-01-01 10:01:00",
        "2024-01-01 10:01:30",
        "2024-01-01 10:02:00");

    // ── 5. Syslog / Linux / OpenSSH / Cisco ──────────────────────────────
    run_test(
        "Syslog / Linux / OpenSSH",
        "([A-Za-z]{3}\\s+\\d{1,2} \\d{2}:\\d{2}:\\d{2})",
        "%b %d %H:%M:%S",
        {
            "Jan  1 10:00:00 host sshd[123]: Accepted password for alice",
            "Jan  1 10:00:10 host sudo[456]: alice became root",
            "Jan  1 10:00:20 host cron[789]: CRON job started",
            "Jan  1 10:00:30 host kernel: OOM killer invoked",
            "Jan  1 10:00:40 host sshd[124]: Invalid user bob",
            "Jan  1 10:00:50 host sshd[124]: Failed password for bob",
            "Jan  1 10:01:00 host sshd[125]: Disconnected from 10.0.0.1",
            "Jan  1 10:01:10 host systemd[1]: Service started",
            "Jan  1 10:01:20 host systemd[1]: Service stopped",
            "Jan  1 10:01:30 host syslogd: restart",
        },
        "Jan  1 10:00:00",
        "Jan  1 10:00:20",
        "Jan  1 10:01:00",
        "Jan  1 10:01:30",
        "Jan  1 10:02:00");

    // ── 6. Nginx Error Log ────────────────────────────────────────────────
    run_test(
        "Nginx Error Log",
        "(\\d{4}/\\d{2}/\\d{2} \\d{2}:\\d{2}:\\d{2})",
        "%Y/%m/%d %H:%M:%S",
        {
            "2024/01/01 10:00:00 [notice] 1#0: nginx/1.25 started",
            "2024/01/01 10:00:10 [notice] 1#0: built by gcc 12",
            "2024/01/01 10:00:20 [notice] 1#0: OS: Linux 6.5",
            "2024/01/01 10:00:30 [notice] 1#0: start worker process",
            "2024/01/01 10:00:40 [notice] 1#0: start worker process",
            "2024/01/01 10:00:50 [error]  *1 connect() failed",
            "2024/01/01 10:01:00 [warn]   *2 upstream response slow",
            "2024/01/01 10:01:10 [notice] 1#0: signal 15 (SIGTERM)",
            "2024/01/01 10:01:20 [notice] 1#0: graceful shutdown",
            "2024/01/01 10:01:30 [notice] 1#0: exit",
        },
        "2024/01/01 10:00:00",
        "2024/01/01 10:00:20",
        "2024/01/01 10:01:00",
        "2024/01/01 10:01:30",
        "2024/01/01 10:02:00");

    // ── 7. Apache Spark ──────────────────────────────────────────────────
    run_test(
        "Apache Spark",
        "(\\d{2}/\\d{2}/\\d{2} \\d{2}:\\d{2}:\\d{2})",
        "%y/%m/%d %H:%M:%S",
        {
            "24/01/01 10:00:00 INFO  SparkContext: Spark 3.5 started",
            "24/01/01 10:00:10 INFO  BlockManager: Initialized",
            "24/01/01 10:00:20 INFO  MemoryStore: 3.5 GB allocated",
            "24/01/01 10:00:30 INFO  DAGScheduler: Stage 0 submitted",
            "24/01/01 10:00:40 INFO  TaskScheduler: Task 0.0 started",
            "24/01/01 10:00:50 INFO  Executor: Running task 0.0",
            "24/01/01 10:01:00 INFO  DAGScheduler: Stage 0 done",
            "24/01/01 10:01:10 INFO  SparkContext: Collecting results",
            "24/01/01 10:01:20 INFO  SparkContext: Job 0 finished",
            "24/01/01 10:01:30 INFO  SparkContext: Stopping",
        },
        "24/01/01 10:00:00",
        "24/01/01 10:00:20",
        "24/01/01 10:01:00",
        "24/01/01 10:01:30",
        "24/01/01 10:02:00");

    // ── 8. HDFS / NameNode ────────────────────────────────────────────────
    run_test(
        "HDFS / NameNode",
        "(\\d{6} \\d{6})",
        "%m%d%y %H%M%S",
        {
            "010124 100000 blk_1001 DataNode: Block reported",
            "010124 100010 blk_1002 DataNode: Block reported",
            "010124 100020 blk_1003 DataNode: Block reported",
            "010124 100030 blk_1004 DataNode: Block reported",
            "010124 100040 blk_1005 DataNode: Block reported",
            "010124 100050 blk_1006 DataNode: Block reported",
            "010124 100100 blk_1007 NameNode: Heartbeat",
            "010124 100110 blk_1008 NameNode: Heartbeat",
            "010124 100120 blk_1009 DataNode: Block replicated",
            "010124 100130 blk_1010 DataNode: Block replicated",
        },
        "010124 100000",
        "010124 100020",
        "010124 100100",
        "010124 100130",
        "010124 100200");

    // ── 9. MySQL Error Log ────────────────────────────────────────────────
    run_test(
        "MySQL Error Log",
        "(\\d{6} \\d{2}:\\d{2}:\\d{2})",
        "%y%m%d %H:%M:%S",
        {
            "240101 10:00:00 1 [Note] MySQL 8.0 started",
            "240101 10:00:10 1 [Note] InnoDB initialized",
            "240101 10:00:20 1 [Note] Server hostname: db01",
            "240101 10:00:30 1 [Note] Binlog enabled",
            "240101 10:00:40 1 [Note] Ready for connections",
            "240101 10:00:50 2 [Warning] Slow query: 3200ms",
            "240101 10:01:00 1 [Note] User alice connected",
            "240101 10:01:10 3 [Warning] Disk usage 80%",
            "240101 10:01:20 1 [Note] Backup started",
            "240101 10:01:30 1 [Note] Shutdown initiated",
        },
        "240101 10:00:00",
        "240101 10:00:20",
        "240101 10:01:00",
        "240101 10:01:30",
        "240101 10:02:00");

    // ── 10. Android Logcat ────────────────────────────────────────────────
    run_test(
        "Android Logcat",
        "(\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2})",
        "%m-%d %H:%M:%S",
        {
            "01-01 10:00:00.000 D/System : Boot started",
            "01-01 10:00:10.000 I/ActivityManager: App launched",
            "01-01 10:00:20.000 W/InputDispatcher: No focused window",
            "01-01 10:00:30.000 I/WindowManager: Display added",
            "01-01 10:00:40.000 D/dalvikvm: GC freed 512 objects",
            "01-01 10:00:50.000 I/Zygote: Fork app process",
            "01-01 10:01:00.000 E/ANRManager: ANR in com.example",
            "01-01 10:01:10.000 I/Zygote: Fork service process",
            "01-01 10:01:20.000 D/System : Low memory warning",
            "01-01 10:01:30.000 I/System : Shutdown requested",
        },
        "01-01 10:00:00",
        "01-01 10:00:20",
        "01-01 10:01:00",
        "01-01 10:01:30",
        "01-01 10:02:00");

    // ── 11. HealthApp ─────────────────────────────────────────────────────
    run_test(
        "HealthApp",
        "(\\d{8}-\\d{2}:\\d{2}:\\d{2})",
        "%Y%m%d-%H:%M:%S",
        {
            "20240101-10:00:00:000|INFO|HeartRate|72",
            "20240101-10:00:10:000|INFO|Steps|100",
            "20240101-10:00:20:000|WARN|HeartRate|120",
            "20240101-10:00:30:000|INFO|Steps|200",
            "20240101-10:00:40:000|INFO|HeartRate|80",
            "20240101-10:00:50:000|INFO|Steps|300",
            "20240101-10:01:00:000|ERROR|GPS|No signal",
            "20240101-10:01:10:000|INFO|Steps|400",
            "20240101-10:01:20:000|INFO|HeartRate|75",
            "20240101-10:01:30:000|INFO|Steps|500",
        },
        "20240101-10:00:00",
        "20240101-10:00:20",
        "20240101-10:01:00",
        "20240101-10:01:30",
        "20240101-10:02:00");

    // ── 12. Proxifier ─────────────────────────────────────────────────────
    run_test(
        "Proxifier",
        "\\[(\\d{2}\\.\\d{2} \\d{2}:\\d{2}:\\d{2})\\]",
        "%d.%m %H:%M:%S",
        {
            "[01.01 10:00:00] chrome.exe  - HTTPS 8.8.8.8:443",
            "[01.01 10:00:10] chrome.exe  - HTTPS 8.8.4.4:443",
            "[01.01 10:00:20] node.exe    - TCP  127.0.0.1:3000",
            "[01.01 10:00:30] node.exe    - TCP  127.0.0.1:3001",
            "[01.01 10:00:40] python.exe  - TCP  10.0.0.1:8080",
            "[01.01 10:00:50] python.exe  - UDP  10.0.0.1:5353",
            "[01.01 10:01:00] chrome.exe  - HTTPS 1.1.1.1:443",
            "[01.01 10:01:10] chrome.exe  - HTTPS 1.0.0.1:443",
            "[01.01 10:01:20] node.exe    - TCP  127.0.0.1:3002",
            "[01.01 10:01:30] node.exe    - TCP  127.0.0.1:3003",
        },
        "01.01 10:00:00",
        "01.01 10:00:20",
        "01.01 10:01:00",
        "01.01 10:01:30",
        "01.01 10:02:00");

    // ── 13. Unix Epoch (BGL / Thunderbird) ───────────────────────────────
    // 1704067200 = 2024-01-01 00:00:00 UTC
    run_test(
        "Unix Epoch (seconds)",
        "(\\d{9,10})(?!\\d)",
        "",   // empty format → parsed as stoll
        {
            "1704067200 node1 INFO  Server started",
            "1704067210 node1 INFO  DB connected",
            "1704067220 node1 INFO  User alice login",
            "1704067230 node1 WARN  High CPU",
            "1704067240 node1 INFO  Cache warmed",
            "1704067250 node1 INFO  Cache full",
            "1704067260 node1 ERROR Disk 90%",
            "1704067270 node1 INFO  Cleanup done",
            "1704067280 node1 INFO  Replica sync",
            "1704067290 node1 INFO  Shutdown",
        },
        "1704067200",
        "1704067220",
        "1704067260",
        "1704067290",
        "1704067300");

    // ── Edge cases ────────────────────────────────────────────────────────

    std::printf("\n[Edge cases — ISO 8601]\n");

    {
        const char* REGEX  = "(\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2})";
        const char* FORMAT = "%Y-%m-%dT%H:%M:%S";

        std::vector<std::string> ten_lines = {
            "2024-01-01T10:00:00 line 0",
            "2024-01-01T10:00:10 line 1",
            "2024-01-01T10:00:20 line 2",
            "2024-01-01T10:00:30 line 3",
            "2024-01-01T10:00:40 line 4",
            "2024-01-01T10:00:50 line 5",
            "2024-01-01T10:01:00 line 6",
            "2024-01-01T10:01:10 line 7",
            "2024-01-01T10:01:20 line 8",
            "2024-01-01T10:01:30 line 9",
        };

        SegmentTree tree = build_tree_from_lines(ten_lines, REGEX, FORMAT);
        std::string fmt(FORMAT);

        // Exact single-second point query
        time_t t = parse_timestamp("2024-01-01T10:00:30", fmt);
        check("ISO 8601", "single-point query (exact ts)  → 1",
              tree.range_query(t, t).size(), 1u);

        // Reversed range (from > to) → 0
        time_t ta = parse_timestamp("2024-01-01T10:01:00", fmt);
        time_t tb = parse_timestamp("2024-01-01T10:00:00", fmt);
        check("ISO 8601", "reversed range (from > to)     → 0",
              tree.range_query(ta, tb).size(), 0u);

        // Bad timestamp string → parse_timestamp returns -1
        time_t bad = parse_timestamp("not-a-timestamp", fmt);
        check("ISO 8601", "malformed timestamp → -1       → 0",
              tree.range_query(bad, bad).size(), 0u);
    }

    // ── Summary ───────────────────────────────────────────────────────────

    std::printf("\n=== Summary: %d passed, %d failed ===\n\n",
                g_passed, g_failed);

    return g_failed > 0 ? 1 : 0;
}
