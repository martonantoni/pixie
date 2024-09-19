#pragma once

class cClipboard
{
public:
    std::string retrieveText() const;
    void storeText(const std::string& text);
};

extern cClipboard theClipboard;