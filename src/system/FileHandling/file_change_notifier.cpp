#include "StdAfx.h"

namespace Pixie
{

cFileChangeNotifier::cFileChangeNotifier(const std::filesystem::path& folder, const std::string& filter, cCallback callback)
    : mFolder(folder)
    , mCallback(std::move(callback))
{
    mTimerID = theMainThread->AddTimer([this]() { checkFiles(); }, cTimerRequest(1000, true));
    std::string pattern;
    pattern.reserve(filter.size() * 2);
    for (auto c : filter)
    {
        switch (c)
        {
        case '*': pattern += ".*"; break;
        case '?': pattern += "."; break;
        case '+': pattern += "\\+"; break;
        case '(': pattern += "\\("; break;
        case ')': pattern += "\\)"; break;
        case '[': pattern += "\\["; break;
        case ']': pattern += "\\]"; break;
        case '.': pattern += "\\."; break;
        case '\\': pattern += "\\\\"; break;
        case '^': pattern += "\\^"; break;
        case '$': pattern += "\\$"; break;
        case '{': pattern += "\\{"; break;
        case '}': pattern += "\\}"; break;
        case '|': pattern += "\\|"; break;
        default:  pattern += c; break;
        }
    }
    mFilter = std::regex(pattern, std::regex_constants::icase);
}

cFileChangeNotifier::cFileChangeNotifier(const std::filesystem::path& path, cCallback callback)
    : cFileChangeNotifier(
        path.has_parent_path() ? path.parent_path() : std::filesystem::path("."), 
        path.filename().string(), 
        std::move(callback))
{
}

void cFileChangeNotifier::checkFiles()
{
    std::filesystem::file_time_type latestWriteTime;
    for (const auto& entry : std::filesystem::directory_iterator(mFolder))
    {
        if (!entry.is_regular_file())
            continue;
        const std::string filename = entry.path().filename().string();
        if (!std::regex_match(filename, mFilter))
            continue;
        latestWriteTime = std::max(latestWriteTime, entry.last_write_time());
    }
    if (latestWriteTime > mLastWriteTime)
    {
        mLastWriteTime = latestWriteTime;
        mCallback();
    }
}


} // namespace Pixie