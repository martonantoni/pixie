#include "StdAfx.h"

// http://en.wikipedia.org/wiki/UTF-8

//7 	U+0000      U+007F   	1 	0xxxxxxx
//11 	U+0080      U+07FF 	    2 	110xxxxx 	10xxxxxx
//16 	U+0800      U+FFFF 	    3 	1110xxxx 	10xxxxxx 	10xxxxxx
//21 	U+10000 	U+1FFFFF 	4 	11110xxx 	10xxxxxx 	10xxxxxx 	10xxxxxx
//26 	U+200000 	U+3FFFFFF 	5 	111110xx 	10xxxxxx 	10xxxxxx 	10xxxxxx 	10xxxxxx
//31 	U+4000000 	U+7FFFFFFF 	6 	1111110x 	10xxxxxx 	10xxxxxx 	10xxxxxx 	10xxxxxx 	10xxxxxx

namespace UTF8
{

	std::string Encode(const wchar_t *Source, size_t SourceLength)
	{
	// 	char *OutPos=std::string.GetBuffer((SourceLength+1)*6); // that's the max
	// 	const unsigned wchar_t *SrcPos=(const unsigned wchar_t *)Source;
	// 	for(size_t i=0;i<SourceLength;++i)
	// 	{
	// 		if(*Source<=0x7f)
	// 		{
	// 			*OutPos=*Source; ++OutPos;
	// 		}
	// 		else if(*Source<=0x7ff)
	// 		{
	// 			*OutPos=0xa0|(*Source>>
	// 		}
	// 	}
		return "not implemented yet"s;
	}

	wchar_t DecodeCharacter(const char *&SrcPos)
	{
		wchar_t Result=0;
		if(!(*SrcPos&0x80))
		{
			Result=*SrcPos;
			++SrcPos;
		}
		else
		{
			if(*SrcPos&0x20) // >=3 bytes
			{
				if(*SrcPos&0x10) // 4 bytes
				{
					Result=((SrcPos[0]&7)<<18)|((SrcPos[1]&63)<<12)|((SrcPos[2]&63)<<6)|(SrcPos[3]&63);
					SrcPos+=4;
				}
				else  // 3 bytes
				{
					Result=((SrcPos[0]&15)<<12)|((SrcPos[1]&63)<<6)|(SrcPos[2]&63);
					SrcPos+=3;
				}
			}
			else // 2 bytes
			{
				Result=((SrcPos[0]&31)<<6)|(SrcPos[1]&63);
				SrcPos+=2;
			}
		}
		return Result;
	}

	std::vector<wchar_t> Decode(const std::string &Source)
	{
		std::vector<wchar_t> Out;
		Out.resize(Source.length()+1);
		wchar_t *OutPos=Out.data();
		size_t Length=0;
		for(const char *SrcPos=Source.c_str(); *SrcPos;)
		{
			*OutPos=DecodeCharacter(SrcPos);
			++OutPos;
			++Length;
		}
		*OutPos=0;
		Out.resize(Length+1);
		return Out;
	}

	cDecodedString::cDecodedString(const std::string &EncodedString)
		: mDecoded(Decode(EncodedString))
	{
	}


}