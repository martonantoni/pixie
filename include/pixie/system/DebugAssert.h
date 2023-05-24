#pragma once

bool Global_HandleAssert(const std::string &Text,int *Ignored);
void ThrowLastError(const std::string &Text);

#define THE_ASSERT_MACRO(x,Info)\
	do \
		{\
		static int Ignored=false;\
		if(!Ignored&&!(x))\
			{\
			std::string DisplayedText=fmt::sprintf("Debug assertation failed\nFile: %s\nLine: %d\nCondition: "#x"\nInformation: %s\nSelect YES for break, NO for continue, CANCEL for ignoring this error",__FILE__,__LINE__,(Info)); \
			Global_HandleAssert(DisplayedText,&Ignored)&&(DebugBreak(),true); \
			}\
		}\
		while(0)

#ifdef ASSERT
#undef ASSERT
#endif

#ifdef _DEBUG
#define ASSERT(x) THE_ASSERT_MACRO(x,"N/A")
#define RELEASE_ASSERT(x) THE_ASSERT_MACRO(x,"N/A")
#define RELEASE_ASSERT_EXT(x,Info) THE_ASSERT_MACRO(x,Info)
#define ASSERTTRUE(x) ((x)?true:(Global_HandleAssert("ASSERTTRUE failed:\n" #x,NULL)&&(DebugBreak(),true),false))
#define ASSERTFALSE(x) ((x)?Global_HandleAssert("ASSERTFALSE failed\n" #x,NULL)&&(DebugBreak(),true),true:false)
#else 
#define ASSERT(x)
#define RELEASE_ASSERT(x) THE_ASSERT_MACRO(x,"N/A")
#define RELEASE_ASSERT_EXT(x,Info) THE_ASSERT_MACRO(x,Info)
#define ASSERTTRUE(x) (x)
#define ASSERTFALSE(x) (x)
#endif

#define THROW_DETAILED_EXCEPTION(x) RELEASE_ASSERT_EXT(false,x)