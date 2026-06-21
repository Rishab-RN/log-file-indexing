#include "LogFile.hxx"

std::size_t LogFile::add_log(const std::string& line)
{
    logs.push_back(line);
    return logs.size() - 1;
}

const std::string& LogFile::get_log(std::size_t index) const
{
    return logs.at(index);
}

const std::vector<std::string>& LogFile::get_all_logs() const
{
    return logs;
}