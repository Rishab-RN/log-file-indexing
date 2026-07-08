#pragma once

#include <vector>
#include <string>
#include <string_view>

/*
 * This class acts as a data storage class for a log file.
 */
class LogFile
{
private:
    // The single contiguous memory block. 
    // Do NOT modify this string after it is populated.
    std::string raw_file_data;

    // Lightweight windows pointing to lines inside raw_file_data.
    std::vector<std::string_view> logs;

public:
    /**
     * load_file
     * @filepath - Path to the file to load.
     * * Reads the entire file into a single contiguous memory block 
     * and generates string_views for each line.
     */
    bool load_from_disk(const std::string& filepath);

    /**
     * get_log
     * @index - The index of the log to get.
     * * Gets the log at a particular index.
     */
    std::string_view get_log(std::size_t index) const;

    const std::vector<std::string_view>& get_all_logs() const;

    inline size_t size() const
    {
        return logs.size();
    }
};