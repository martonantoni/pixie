#pragma once


namespace StandardLogPlugins
{
	class cEcho: public cLog::cPlugin
	{
		void Open() override;
		void Close() override;
		void Flush() override;
		void LogChunkPart(cLog::cChunkPart *ChunkPart) override;
	public:
		cEcho(): cPlugin("Log_Echo",LOGS_CHUNKPARTS) {}
	};

	class cFile: public cLog::cPlugin
	{
		HANDLE FileHandle;
		void Open() override;
		void Close() override;
		void Flush() override;
		void LogChunk(cLog::cChunk *Chunk) override;
		void Seek(cSeekParameters *SeekPars) override;
		void Tell(__int64 *Position) override;
	public:
		cFile(): cPlugin("Log_File",LOGS_CHUNKS) {}
	};

// 	class cDebugFile: public cLog::cPlugin
// 	{
// 		void Open() override;
// 		void Close() override;
// 		void Flush() override;
// 		void LogChunkPart(cLog::cChunkPart *ChunkPart) override;
// 	public:
// 		cDebugFile(): cPlugin("Log_DebugFile", LOGS_CHUNKPARTS) {}
// 	};

}