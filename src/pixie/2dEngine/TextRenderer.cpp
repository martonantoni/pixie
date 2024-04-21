#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
void cTextRenderer::cDocument::SetAlignment(eAlignment Alignment)
{
	if(ASSERTFALSE(mParent.mMaxWidth==~0)) // alignment can be set only if MaxWidth was set
		return;
	mParagraphAlignment=Alignment;
}

void cTextRenderer::cDocument::NewLine(eNewLineType Type)
{
	auto &CurrentLine=mLines.back();
	switch(mParagraphAlignment)
	{
	case eAlignment::Left:
		break;
	case eAlignment::Right:
		CurrentLine.mWords.emplace(CurrentLine.mWords.begin(), mParent.mMaxWidth-CurrentLine.mSumWidth);
		break;
	case eAlignment::Center:
		CurrentLine.mWords.emplace(CurrentLine.mWords.begin(), (mParent.mMaxWidth-CurrentLine.mSumWidth)/2);
		break;
	case eAlignment::Justified:
		{
			if(Type==eNewLineType::MaxWidthReached)
			{
				int ExtraWidth=mParent.mMaxWidth-CurrentLine.mSumWidth;
				int NumberOfGaps=int(CurrentLine.mWords.size())-1;
				if(NumberOfGaps>0)
				{
					double ExtraPerGap=double(ExtraWidth)/double(NumberOfGaps);
					double ExtraSum=0;
					for(size_t i=0; i!=NumberOfGaps; ++i)
					{
						ExtraSum+=ExtraPerGap;
						int ThisGapExtra=ExtraSum;
						CurrentLine.mWords[i].mWidth+=ThisGapExtra;
						ExtraSum-=ThisGapExtra;
					}
				}
			}
			break;
		}
	}
	int AscenderMax=0, NextUnderBaseMax=0;
	for(auto &Word: CurrentLine.mWords)
	{
		if(CurrentLine.mNextLineYOffset<Word.mHeight)
			CurrentLine.mNextLineYOffset=Word.mHeight;
		if(AscenderMax<Word.mAscender)
			AscenderMax=Word.mAscender;
		auto NextUnderBase=Word.mHeight-Word.mAscender;
		if(NextUnderBaseMax<NextUnderBase)
			NextUnderBaseMax=NextUnderBase;
	}
	for(auto &Word: CurrentLine.mWords)
	{
		int YOffset=AscenderMax-Word.mAscender;
		if(YOffset)
		{
			for(auto &Sprite: Word.mSprites)
			{
				Sprite->SetPosition(Sprite->GetPosition()+cPoint{0, YOffset});
			}
		}
	}
	if(Type!=eNewLineType::SameY)
	{
		CurrentLine.mNextLineYOffset=AscenderMax+NextUnderBaseMax;
		if(!CurrentLine.mNextLineYOffset)
			CurrentLine.mNextLineYOffset=mFontStack.back()->GetHeight();
	}
	else
	{
		CurrentLine.mNextLineYOffset=0;
	}
	if(Type!=eNewLineType::EndOfDocument)
		mLines.emplace_back();
}

void cTextRenderer::cDocument::AddWord(cWord &&NewWord)
{
	mSpriteIndex+=NewWord.mSprites.size();
restart:
	auto &CurrentLine=mLines.back();
	switch(NewWord.mType)
	{
	case cWord::eType::LineBreak:
		{
			NewLine(eNewLineType::LineBreak);
			mParagraphAlignment=eAlignment::Left;
			return;
		}
	case cWord::eType::LineBreak_SameLine:
		{
			NewLine(eNewLineType::SameY);
			mParagraphAlignment=eAlignment::Left;
			return;
		}
	case cWord::eType::Tab:
		{
			if(mTabStops.size()<=CurrentLine.mTabStopCounter)
				break;
			if(mTabStops[CurrentLine.mTabStopCounter]<CurrentLine.mSumWidth)
				break;
			int Width=mTabStops[CurrentLine.mTabStopCounter]-CurrentLine.mSumWidth;
			CurrentLine.mSumWidth=mTabStops[CurrentLine.mTabStopCounter];
			CurrentLine.mWords.emplace_back(cWord(Width));
			++CurrentLine.mTabStopCounter;
			break;
		}
	case cWord::eType::Word:
		{
			if(unsigned int(CurrentLine.mSumWidth+NewWord.mWidth)>mParent.mMaxWidth&&!CurrentLine.mWords.empty())
			{
				NewLine(eNewLineType::MaxWidthReached);
				goto restart;
			}
			CurrentLine.mSumWidth+=NewWord.mWidth;
			CurrentLine.mWords.emplace_back(std::move(NewWord));
			break;
		}
	case cWord::eType::Change_Y:
		CurrentLine.mWords.emplace_back(std::move(NewWord));
		break;
	}
}

size_t cTextRenderer::cDocument::GetSpriteIndex() const
{
	return mSpriteIndex;
}


void cTextRenderer::cDocument::EndOfDocument()
{
	NewLine(eNewLineType::EndOfDocument);
}

cTextRenderer::cWord cTextRenderer::CreateWordFromTexture(const std::string &TextureName)
{
	auto Texture=theTextureManager.GetTexture(TextureName);
	if(ASSERTFALSE(!Texture)) // {i:texture_name} texture is not found
		return cWord(cWord::eType::Invalid);
	auto Sprite=std::make_unique<cSprite>();
	Sprite->SetTextureAndSize(Texture);
	cWord Word(cWord::eType::Word);
	Word.mSprites.emplace_back(std::move(Sprite));
	Word.mHeight=Word.mAscender=Texture->GetTextureHeight();
	Word.mWidth=Texture->GetTextureWidth();
	return Word;
}

std::vector<std::unique_ptr<cSpriteBase>> cTextRenderer::render(const std::string &Text, OUT cRenderInfo *RenderInfo) const
{
	if(Text.empty())
		return {};
	cDocument Document(*this);

	size_t mCurrentWordStart=0;
	auto EndOfCurrentWord=[&, this](size_t SeparatorIndex, bool IncludeSeparator)
	{
		if(SeparatorIndex==mCurrentWordStart&&!IncludeSeparator)
		{
			mCurrentWordStart=SeparatorIndex+1;
			return;
		}
		cPoint Position(0, 0);
		const cFont *Font=Document.mFontStack.back();
		std::vector<std::unique_ptr<cSprite>> LetterSprites;
		for(size_t i=mCurrentWordStart, LastIndex=IncludeSeparator?SeparatorIndex:SeparatorIndex-1; i<=LastIndex;)
		{
			auto DecodeIterator=Text.c_str()+i;
			auto DecodedCharacter=UTF8::DecodeCharacter(DecodeIterator);
			i=DecodeIterator-Text.c_str();
			const cFont::cLetterData &LetterData=Font->GetLetterData(DecodedCharacter);
			if(ASSERTFALSE(!LetterData.mTexture))
				continue;
			auto Sprite=std::make_unique<cSprite>();
			Sprite->SetTextureAndSize(LetterData.mTexture);
			Sprite->SetPosition(Position+cPoint{LetterData.mXOffset, LetterData.mYOffset});
			Sprite->SetRGBColor(Document.mColorStack.back());
			LetterSprites.emplace_back(std::move(Sprite));
			Position+=cPoint(LetterData.mAdvance>>6, 0);
		}
		Document.AddWord(cWord(std::move(LetterSprites), Position.x, *Font));
		mCurrentWordStart=SeparatorIndex+1;
	};
	for(size_t i=0, iend=Text.length();;)
	{
		switch(Text[i])
		{
		case ' ':
			EndOfCurrentWord(i,true);
			break;
		case '\n':
			EndOfCurrentWord(i,false);
			Document.AddWord(cWord(cWord::eType::LineBreak));
			break;
		case '\r':
			EndOfCurrentWord(i, false);
			Document.AddWord(cWord(cWord::eType::LineBreak_SameLine));
			break;
		case '\t':
			EndOfCurrentWord(i,false);
			Document.AddWord(cWord(cWord::eType::Tab));
			break;
		case '{':
			{
				if(i>0 && Text[i-1]=='\\') // \{ is not a tag
                    break;
				EndOfCurrentWord(i, false);
				auto EndTagPos=Text.find('}', i+1);
				if(ASSERTFALSE(EndTagPos==std::string::npos)) // missing }
					goto end_of_text;
				mCurrentWordStart=EndTagPos+1;
				if(ASSERTFALSE(EndTagPos==i+1)) // empty {}
				{
					i=EndTagPos;
					break;
				}
				bool IsClosing=EndTagPos==i+2;
				if(ASSERTFALSE(!IsClosing&&Text[i+2]!=':')) // not closing, but ':' is missing
				{
					i=EndTagPos;
					break;
				}
				std::string value=IsClosing?std::string():Text.substr(i+3, EndTagPos-i-3);
				switch(Text[i+1])
				{
				case 'c':
					if(IsClosing)
					{
						if(ASSERTTRUE(Document.mColorStack.size()>=2)) // {c} without matching {c:color}
						{
							Document.mColorStack.pop_back();
						}
					}
					else
					{
						Document.mColorStack.push_back(cColor(value));
					}
					break;
				case 'f':
					if(IsClosing)
					{
						if(ASSERTTRUE(Document.mFontStack.size()>=2)) // {f} without matching {f:font}
						{
							Document.mFontStack.pop_back();
						}
					}
					else
					{
						Document.mFontStack.push_back(theFontManager.GetFont(value));
					}
					break;
				case '>':
					Document.SetAlignment(cDocument::eAlignment::Right);
					break;
				case '|':
					Document.SetAlignment(cDocument::eAlignment::Center);
					break;
				case '=':
					Document.SetAlignment(cDocument::eAlignment::Justified);
					break;
				case 'y':
					Document.AddWord(cWord(cWord::eType::Change_Y, std::stoi(value)));
					break;
				case 'i':
					Document.AddWord(CreateWordFromTexture(value));
					break;
				case 't':
					Document.setTabStops(cStringVector(value, ",", false).ToIntVector());
					break;
				case 'g':
					if(!RenderInfo)
						break;
					if(IsClosing)
					{
						if(ASSERTTRUE(!Document.mGroupStack.empty())) // {g} without matching {g:group_index}
						{
							int ivalue=Document.mGroupStack.back();
							Document.mGroupStack.pop_back();
							RenderInfo->mGroups[ivalue].mLastSpriteIndex=Document.GetSpriteIndex()-1;
						}
					}
					else
					{
						int ivalue=atoi(value.c_str());
						if(RenderInfo->mGroups.size()<ivalue+1)
							RenderInfo->mGroups.resize(ivalue+1);
						RenderInfo->mGroups[ivalue].mFirstSpriteIndex=Document.GetSpriteIndex();
						Document.mGroupStack.push_back(ivalue);
					}
					break;
				default:
					ASSERT(false); // unknown command
					break;
				}
				i=EndTagPos;
			}
		}
		if(++i==iend)
		{
			EndOfCurrentWord(i,false);
			Document.AddWord(cWord(cWord::eType::LineBreak_SameLine));
			break;
		}
	}
end_of_text:
	Document.EndOfDocument();
	std::vector<std::unique_ptr<cSpriteBase>> AllLetters;
	cPoint CursorPos;
	size_t TabIndex=0;
	for(auto &Line: Document.mLines)
	{
		for(cWord &Word: Line.mWords)
		{
			switch(Word.mType)
			{
			case cWord::eType::Word:
				{
					for(auto &Sprite: Word.mSprites)
					{
						Sprite->SetPosition(Sprite->GetPosition()+CursorPos);
					}
					CursorPos.x+=Word.mWidth;
					std::move(Word.mSprites.begin(), Word.mSprites.end(), std::back_inserter(AllLetters));
					break;
				}
			case cWord::eType::Change_Y:
				CursorPos.y+=Word.mChangeValue;
				break;
			case cWord::eType::LineBreak:
			case cWord::eType::Tab:
			case cWord::eType::Invalid:
				ASSERT(false);
				break;
			}
		}
		CursorPos.y+=Line.mNextLineYOffset;
		CursorPos.x=0;
		TabIndex=0;
	}
	if(RenderInfo)
		RenderInfo->mNextLineY=CursorPos.y;

	return AllLetters;
}

std::string cTextRenderer::escapeText(const std::string& Text)
{
	// change any { to \{
	std::string escapedText;
	escapedText.reserve(Text.size());
	for(size_t i=0, iend=Text.size(); i!=iend; ++i)
    {
        if(Text[i]=='{')
			escapedText.push_back('\\');
		escapedText.push_back(Text[i]);
    }
	return escapedText;
}
