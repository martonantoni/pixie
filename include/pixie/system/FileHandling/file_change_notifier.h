#pragma once

namespace Pixie
{

class cFileChangeNotifier final
{
public:
    using cCallback = std::function<void()>;
private:
    std::filesystem::path mFolder;
    std::regex mFilter;
    cCallback mCallback;
    cRegisteredID mTimerID;
    std::filesystem::file_time_type mLastWriteTime{};
    void checkFiles();
public:
    cFileChangeNotifier(const std::filesystem::path& folder, const std::string& filter, cCallback callback);
    cFileChangeNotifier(const std::filesystem::path& path, cCallback callback);
};

} // namespace Pixie