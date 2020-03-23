#pragma once

class cTimerHandlerData;

class cLog
{
public:
	enum eFlags
	{
		FLUSH_OVER_TIME		= 0x001,
		USE_MUTEX			= 0x002,
		TRUNCATE			= 0x004,
		ECHO				= 0x008,
		NO_LINE_FEED		= 0x010,
		TIME_STAMP			= 0x020,
		NO_FILE				= 0x040,
		SINGLE_THREAD		= 0x080,
		NO_AUTO_SEEK_END	= 0x100,
		AUTO_FLUSH          = 0x200,
	};
	class cPlugin;
	class cChunk;
	struct cChunkPart;
private:
	enum { DefaultChunkLength =  30000 };
	cChunk *ActiveChunk;
	cThread *TimerThread;
	cMutex Mutex;
	int Flags;
	std::string FileName;
	typedef std::list<cPlugin *> cPlugins;
	cPlugins Plugins,ChunkLoggingPlugins,ChunkPartLoggingPlugins;
	__int64 LoggedBytesCount;
	void ConstructionHelper();
	void WriteOutActiveChunk();
	void LogChunk(cChunk *Chunk);
	cRegisteredID mTimerID;
	void OnTimer();
public:
	cLog(const cPath &pFileName,int pFlags=0);
	explicit cLog(int pFlags);
	cLog();
	virtual ~cLog();

	int GetFlags() const { return Flags; }
	const std::string &GetFileName() const { return FileName; }
	void AddPlugin(cPlugin *Plugin);
	void RemovePlugin(cPlugin *Plugin);

	void Log(const char *FormatString,...);
	void LogArgs(const char *FormatString,va_list Args);
	void LogBinary(const void *Data,int Length);
	void Flush();

	void Seek(__int64 Offset,int Method); // Also flushes, waits the execution.
	__int64 Tell(); // returns -1 if Tell is not available on any of the plugins, 
	//if there are more than one plugin which supports tell, then the return value is not defined ("randomly" selected
	//from the plugins)
	__int64 GetLoggedBytesCount() const { return LoggedBytesCount; }
	// This is about what were sent to the LogPlugins. It is not neccessary
	// how much bytes were actually written out, because the plugins can change that
	// Neither it is how long the log file is, because it doesn't count the original size of the file
	// It includes size that was not even sent to the plugins yet (in the active chunk)
};

class cLog::cPlugin
{
public:
	enum eFlags { LOGS_CHUNKS=1, LOGS_CHUNKPARTS=2 };
	typedef std::pair<__int64,int> cSeekParameters;
private:
	const char *PreferredThreadName;
	cThread *Thread;
	int Flags;
protected:
	cLog *AttachedLog;
public:
	cPlugin(const char *pPreferredThreadName,int pFlags): 
		PreferredThreadName(pPreferredThreadName), Thread(NULL), AttachedLog(NULL), Flags(pFlags) {}
	virtual ~cPlugin() {}
	void SetLog(cLog *pLog); // When the plugin gets attached to a cLog, this is called first
	cThread *GetThread() const { return Thread; }
	int GetFlags() const { return Flags; }
// All plugins have to override these:
	virtual void Open() abstract; // this is called right after SetLog
	virtual void Close() abstract; // When the cLog object is destroyed, or the plugin is removed from the cLog
	virtual void Flush() abstract;
// optional overridables:
	virtual void Seek(cSeekParameters *SeekPars) {}
	virtual void Tell(__int64 *Position) {}
// Overriding is controlled by PluginFlags
	virtual void LogChunkPart(cLog::cChunkPart *ChunkPart);
	virtual void LogChunk(cLog::cChunk *Chunk);
};

