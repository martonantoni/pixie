#pragma once

int DoesFileExist(const cPath &FileName);

void TruncateFileAt(const std::string &FileName,__int64 NewFileSize);

__int64 GetFileSize(const std::string &FileName);

inline const char *Bool2Text(int BoolValue) { return BoolValue?"true":"false"; }

void TrimRight(std::string &s);
void TrimLeft(std::string &s);
void Trim(std::string &s);
