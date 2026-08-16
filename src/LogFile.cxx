#include "LogFile.hxx"
#include <fstream>

bool LogFile::load_from_disk(const std::string& filepath)
{
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) 
    {
        return false;
    }

    // 1. Get exact file size and allocate memory once
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    raw_file_data.resize(size);

    // 2. Read the entire file into RAM in a single OS call
    if (!file.read(raw_file_data.data(), size)) 
    {
        return false;
    }

    // 3. Prepare the views vector. 
    // We reserve estimated space to avoid vector reallocations.
    logs.clear();
    logs.reserve(size / 60); // Assuming average line length of 60 chars

    // 4. Slice the raw data into string_views
    size_t start = 0;
    size_t end = raw_file_data.find('\n');

    while (end != std::string::npos)
    {
        size_t length = end - start;
        
        // Handle Windows \r\n line endings cleanly
        if (length > 0 && raw_file_data[start + length - 1] == '\r') 
        {
            length--;
        }
        
        logs.emplace_back(raw_file_data.data() + start, length);
        
        start = end + 1;
        end = raw_file_data.find('\n', start);
    }

    // Capture the final line if the file lacks a trailing newline
    if (start < raw_file_data.size())
    {
        logs.emplace_back(raw_file_data.data() + start, raw_file_data.size() - start);
    }

    return true;
}

std::string_view LogFile::get_log(std::size_t index) const
{
    return logs.at(index);
}

const std::vector<std::string_view>& LogFile::get_all_logs() const
{
    return logs;
}