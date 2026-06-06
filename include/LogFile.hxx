#pragma once

#include <vector>
#include <string>

/*
 *  This class acts as a data storage class for a log file.
 */

class LogFile
{
private:
    std::vector<std::string> logs;

public:
    /**
     *  add_log
     *  @line - A line of the log file in the form of an std::string.
     *  
     *  Adds @line to member $logs.
     */
    std::size_t add_log(const std::string& line);

    /**
     *  get_log
     *  @index - The index of the log to get.
     *  
     *  Gets the log at a particular index of member $logs.
     */
    const std::string& get_log(std::size_t index) const;

    inline size_t size() const
    {
        return logs.size();
    }
};