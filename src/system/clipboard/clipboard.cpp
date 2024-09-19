#include "StdAfx.h"

#include "pixie/system/clipboard/clipboard.h"

cClipboard theClipboard;

std::string cClipboard::retrieveText() const
{
    // Open the clipboard
    if (!OpenClipboard(nullptr)) {
        return "";
    }

    // Ensure the clipboard contains text
    if (!IsClipboardFormatAvailable(CF_TEXT)) {
        CloseClipboard();
        return "";
    }

    // Get the handle to the clipboard text
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == nullptr) {
        CloseClipboard();
        return "";
    }

    // Lock the handle to retrieve the text
    char* pszText = static_cast<char*>(GlobalLock(hData));
    if (pszText == nullptr) {
        CloseClipboard();
        return "";
    }

    // Copy the text into a string
    std::string text(pszText);

    // Unlock the handle and close the clipboard
    GlobalUnlock(hData);
    CloseClipboard();

    return text;
}

void cClipboard::storeText(const std::string& text)
{
    // Open the clipboard
    if (!OpenClipboard(nullptr)) {
        return;
    }

    // Empty the clipboard
    EmptyClipboard();

    // Allocate memory for the text
    HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
    if (hData == nullptr) {
        CloseClipboard();
        return;
    }

    // Lock the handle to copy the text
    char* pszText = static_cast<char*>(GlobalLock(hData));
    if (pszText == nullptr) {
        CloseClipboard();
        return;
    }
    strcpy(pszText, text.c_str());
    GlobalUnlock(hData);

    // Set the clipboard data
    SetClipboardData(CF_TEXT, hData);

    // Close the clipboard
    CloseClipboard();
}