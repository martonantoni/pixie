#pragma once

class cProgramArguments: public std::map<std::string,std::string>
{
public:
	cProgramArguments(int ArgNo,char **Args);
	std::string GetString(const std::string &Key,const std::string &DefaultValue) const;
};

extern cProgramArguments *ProgramArguments;