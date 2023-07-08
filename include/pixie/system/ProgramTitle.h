#pragma once

class cProgramTitle: public std::string
{
	cProgramTitle();
public:
	static cProgramTitle *Get();
	void SetInstanceName(const std::string &InstanceName);
};

extern const char *ProgramName;
extern const char *VersionString;
