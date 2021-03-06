#pragma once

class cStringTable: public tSingleton<cStringTable>
{
	struct cThreadData
	{
		typedef std::map<CString,const char *> cStringMap;
		typedef std::vector<cStringMap> cStringHashTable;
		cStringHashTable HashTable;
		cThreadData();
	};
	DWORD ThreadDataIndex;
	cThreadData *RootData;
	cMutex RootMutex;
public:
	cStringTable();
	~cStringTable() {}
	const char *GetUniqueTextInstance(const CString &String);
};