#pragma once

class cPixieSystem final
{
    std::unique_ptr<cPixieMainLoop> mMainLoop;
public:
    cPixieSystem() = default;
    ~cPixieSystem() = default;
    void init();
};

extern cPixieSystem thePixieSystem;