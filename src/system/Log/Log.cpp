#include "StdAfx.h"

#include "LogChunk.h"
#include "StandardLogPlugins.h"

cLog::cLog(const cPath &pFileName,int pFlags): FileName(pFileName.ToString()), Flags(pFlags)
{
	ConstructionHelper();
}

cLog::cLog(int pFlags): FileName("N/A"), Flags(pFlags|NO_FILE)
{
	ConstructionHelper();
}

cLog::cLog(): FileName("N/A"), Flags(NO_FILE)
{
	ConstructionHelper();
}

void cLog::ConstructionHelper()
{
	LoggedBytesCount=0;
	ActiveChunk=new cChunk(DefaultChunkLength);
	if(Flags&ECHO)
		AddPlugin(new StandardLogPlugins::cEcho);
	if(!(Flags&NO_FILE))
		AddPlugin(new StandardLogPlugins::cFile);
	if(Flags&FLUSH_OVER_TIME)
	{
		TimerThread=cThread::GetCurrentThread();
		mTimerID=TimerThread->AddTimer([this]() { OnTimer(); }, cTimerRequest(5000));
	}
	else
	{
		TimerThread=NULL;
	}
}

cLog::~cLog()
{
	mTimerID.Unregister();
	WriteOutActiveChunk();
	for(cPlugins::iterator i=Plugins.begin(),iend=Plugins.end();i!=iend;++i)
	{
		cPlugin *Plugin=*i;
		::CallBack(Plugin->GetThread(),eCallbackType::Wait,Plugin,&cPlugin::Close);
		delete Plugin;
	}
}

void cLog::AddPlugin(cPlugin *Plugin)
{
	cMutexGuard Guard(Mutex,Flags&USE_MUTEX);

	Plugin->SetLog(this);
	Plugin->Open();
	Plugins.push_back(Plugin);
	if(Plugin->GetFlags()&cPlugin::LOGS_CHUNKS)
		ChunkLoggingPlugins.push_back(Plugin);
	if(Plugin->GetFlags()&cPlugin::LOGS_CHUNKPARTS)
		ChunkPartLoggingPlugins.push_back(Plugin);
}

void cLog::RemovePlugin(cPlugin *Plugin)
{
	cMutexGuard Guard(Mutex,Flags&USE_MUTEX);

	for(cPlugins::iterator i=Plugins.begin(),iend=Plugins.end();i!=iend;++i)
	{
		if(*i==Plugin)
		{
			Plugin->Close();
			Plugins.erase(i);
			delete Plugin;
			break;
		}
	}
}

void cLog::OnTimer()
{
	cMutexGuard Guard(Mutex,Flags&USE_MUTEX);

	WriteOutActiveChunk();
}

void cLog::Log(const char *FormatString,...)
{
	va_list Args;
	va_start(Args,FormatString);
	LogArgs(FormatString,Args);
	va_end(Args);
	if(Flags&AUTO_FLUSH)
		Flush();
}

void cLog::Flush()
{
	cMutexGuard Guard(Mutex,Flags&USE_MUTEX);

	WriteOutActiveChunk();
	for(cPlugins::iterator i=Plugins.begin(),iend=Plugins.end();i!=iend;++i)
	{
		cPlugin *Plugin=*i;
		::CallBack(Plugin->GetThread(),eCallbackType::Wait,Plugin,&cPlugin::Flush);
	}
}

void cLog::LogChunk(cChunk *Chunk)
{
	if(Chunk->GetOffset()==0)
		return;
	for(cPlugins::iterator i=ChunkLoggingPlugins.begin(),iend=ChunkLoggingPlugins.end();i!=iend;++i)
	{
		cPlugin *Plugin=*i;
		::CallBack(Plugin->GetThread(),eCallbackType::Normal,Plugin,&cPlugin::LogChunk,new cChunk(*Chunk));
	}
}

void cLog::WriteOutActiveChunk()
{
	LogChunk(ActiveChunk);
	delete ActiveChunk;
	ActiveChunk=new cChunk(DefaultChunkLength);
}

void cLog::LogArgs(const char *FormatString,va_list Args)
{
	cMutexGuard Guard(Mutex,Flags&USE_MUTEX);

	int WrittenLength,Offset;
	for(;;)
	{
		Offset=ActiveChunk->GetOffset();
		WrittenLength=ActiveChunk->LogArgs(FormatString,Args,Flags);
		if(WrittenLength!=-1)
			break;
		WriteOutActiveChunk();
	}
	LoggedBytesCount+=WrittenLength;
	for(cPlugins::iterator i=ChunkPartLoggingPlugins.begin(),iend=ChunkPartLoggingPlugins.end();i!=iend;++i)
	{
		cPlugin *Plugin=*i;
		::CallBack(Plugin->GetThread(),eCallbackType::Normal,Plugin,&cPlugin::LogChunkPart,new cChunkPart(ActiveChunk,Offset,WrittenLength));
	}
}

void cLog::LogBinary(const void *Data,int Length)
{
	cMutexGuard Guard(Mutex,Flags&USE_MUTEX);

	if(ActiveChunk->LogBinary((const char *)Data,Length)==-1)
	{
		WriteOutActiveChunk();
		if(Length>=DefaultChunkLength)
		{
			cChunk *ExtraChunk=new cChunk(Length);
			ExtraChunk->LogBinary((const char *)Data,Length);
			LogChunk(ExtraChunk);
			delete ExtraChunk;
		}
		else
		{
			ASSERT(ActiveChunk->LogBinary((const char *)Data,Length)!=-1);
		}
	}
	LoggedBytesCount+=Length;
}

void cLog::Seek(__int64 Offset,int Method)
{
	cMutexGuard Guard(Mutex,Flags&USE_MUTEX);

	Flush();
	cPlugin::cSeekParameters SeekParameters(Offset,Method);
	for(cPlugins::iterator i=ChunkLoggingPlugins.begin(),iend=ChunkLoggingPlugins.end();i!=iend;++i)
	{
		cPlugin *Plugin=*i;
		::CallBack(Plugin->GetThread(),eCallbackType::Wait,Plugin,&cPlugin::Seek,&SeekParameters);
	}
}

__int64 cLog::Tell()
{
	cMutexGuard Guard(Mutex,Flags&USE_MUTEX);

	WriteOutActiveChunk();
	__int64 Position=-1;
	for(cPlugins::iterator i=ChunkLoggingPlugins.begin(),iend=ChunkLoggingPlugins.end();i!=iend;++i)
	{
		cPlugin *Plugin=*i;
		::CallBack(Plugin->GetThread(),eCallbackType::Wait,Plugin,&cPlugin::Tell,&Position);
		if(Position!=-1)
			return Position;
	}
	return -1;
}


void cLog::cPlugin::SetLog(cLog *pLog)
{ 
	AttachedLog=pLog; 
	if(!(AttachedLog->GetFlags()&cLog::SINGLE_THREAD)&&PreferredThreadName)
		Thread=theThreadServer->GetThread(PreferredThreadName);
	else
		Thread=cThread::GetCurrentThread();
}


void cLog::cPlugin::LogChunk(cLog::cChunk *Chunk)
{
	delete Chunk;
}
void cLog::cPlugin::LogChunkPart(cLog::cChunkPart *ChunkPart)
{
	delete ChunkPart;
}