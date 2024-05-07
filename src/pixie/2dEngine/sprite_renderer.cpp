#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
#include "pixie/pixie/2dEngine/sprite_renderer.h"
#include "pixie/pixie/ui/Window2_SpriteIterator.h"

void cSpriteRenderer::Init()
{
	mMaxSpritesPerFlush = theGlobalConfig->get<int>("pixie_system.sprite_renderer.max_sprites_per_flush");
	ASSERT(mMaxSpritesPerFlush);

	mPixieDevice=cDevice::Get();
	mDevice=mPixieDevice->GetD3DObject();

	//Create vertex buffer
	D3V(mDevice->CreateVertexBuffer(mMaxSpritesPerFlush * 4 * sizeof(cSpriteVertexData),
		D3DUSAGE_WRITEONLY, cSpriteVertexData::FVF, D3DPOOL_MANAGED, &mVertexBuffer, NULL));
	D3V(mDevice->CreateIndexBuffer(mMaxSpritesPerFlush * 4 * 3, D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16, D3DPOOL_MANAGED, &mIndexBuffer, NULL));

	short *indices=NULL;
	D3V(mIndexBuffer->Lock(0, mMaxSpritesPerFlush * 4  * 3, (void**)&indices, 0));

	for(int vertex = 0, index=0; vertex < mMaxSpritesPerFlush * 4; vertex += 4, index += 6)
	{
		indices[index] = vertex;
		indices[index + 1] = vertex + 2;
		indices[index + 2] = vertex + 3;
		indices[index + 3] = vertex;
		indices[index + 4] = vertex + 1;
		indices[index + 5] = vertex + 2;
	}

	//Unlock index buffer
	D3V(mIndexBuffer->Unlock());

	mIsInitDone=true;
}

void cSpriteRenderer::UpdateBlending(cSpriteRenderInfo::eBlendingMode BlendingMode)
{
	D3V(mDevice->SetRenderState(D3DRS_LIGHTING, FALSE));
	switch(BlendingMode)
	{
	case cSpriteRenderInfo::Blend_Normal:
		D3V(mDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
		D3V(mDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE));
		D3V(mDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
		D3V(mDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));
		D3V(mDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE));
		D3V(mDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE));
		break;
	case cSpriteRenderInfo::Blend_LikeLight:
		D3V(mDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
		D3V(mDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE));
		D3V(mDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
		D3V(mDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTALPHA));
		break;
	case cSpriteRenderInfo::Blend_CopySource:
		D3V(mDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE));
		D3V(mDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE));
		break;
	case cSpriteRenderInfo::Invalid_Blend_Mode:
		ASSERT(false);
		break;
		//	case cSpriteRenderInfo::Blend_Solid:

	}
}

cSpriteRenderer::~cSpriteRenderer()
{
	mIsUnderDestruction=true;
	if(mVertexBuffer)
		mVertexBuffer->Release();
	if(mIndexBuffer)
		mIndexBuffer->Release();
}

void cSpriteRenderer::Rotate(cFloatPoint &Point, cFloatPoint Center, float s, float c)
{
	Point-=Center;
	cFloatPoint Rotated(Point.x * c - Point.y * s, Point.x * s + Point.y * c);
	Point=Rotated;
	Point+=Center;
}

// 				auto IsCCW=[](auto p1, auto p2, auto p3)
// 				{
// 					return (p2.x - p1.x)*(p3.y - p1.y) - (p2.y - p1.y)*(p3.x - p1.x) >0;
// 				};



void cSpriteRenderer::RenderSprites()
{
	cRenderState renderState;
	D3V(mVertexBuffer->Lock(0, mMaxSpritesPerFlush * 4 * sizeof(cSpriteVertexData), (void **)&renderState.batchVertices, 0));
	D3V(mDevice->SetStreamSource(0, mVertexBuffer, 0, sizeof(cSpriteVertexData)));
	D3V(mDevice->SetIndices(mIndexBuffer));
	for(cPixieWindow::cSpriteIterator i=mBaseWindow.CreateSpriteIterator(); !i.IsAtEnd(); )
	{
		cSpriteBase &Sprite=*i;
		cSpriteRenderInfo RenderInfo=Sprite.GetRenderInfo();
		auto batchVertices = renderState.batchVertices;
		auto& NumberOfBatchedVertices = renderState.NumberOfBatchedVertices;
		float Z = 0.5f;
		if(RenderInfo.mTexture)
		{
			if(RenderInfo.mBlendingMode!= renderState.LastBlendingMode)
			{
				++renderState.StateChangeCount;
				FlushBuffer(batchVertices, NumberOfBatchedVertices, true);
				renderState.LastBlendingMode=RenderInfo.mBlendingMode;
				UpdateBlending(renderState.LastBlendingMode);
			}
			if(RenderInfo.mTexture->mTexture!= renderState.Texture)
			{
				++renderState.TextureChangeCount;
				FlushBuffer(batchVertices, NumberOfBatchedVertices, true);
				renderState.Texture=RenderInfo.mTexture->mTexture;
				D3V(mDevice->SetTexture(0, renderState.Texture));
			}
			++renderState.SpriteCount;
			cFloatPoint TopLeft(RenderInfo.mRect.TopLeft());
			cFloatPoint TopRight(RenderInfo.mRect.TopRight());
			cFloatPoint BottomLeft(RenderInfo.mRect.BottomLeft());
			cFloatPoint BottomRight(RenderInfo.mRect.BottomRight());
			TopLeft+=cFloatPoint(-0.5, -0.5);       // https://msdn.microsoft.com/en-us/library/windows/desktop/bb219690%28v=vs.85%29.aspx
			TopRight+=cFloatPoint(-0.5+1.0, -0.5);   // (Directly Mapping Texels to Pixels (Direct3D 9))
			BottomLeft+=cFloatPoint(-0.5, -0.5+1.0);
			BottomRight+=cFloatPoint(-0.5+1.0, -0.5+1.0);

			if(RenderInfo.mRotation)
			{
				cFloatPoint Center(RenderInfo.mRect.GetCenter());
				float Rad=RenderInfo.mRotation*3.1415/180.0;
				float s=sin(Rad);
				float c=cos(Rad);
				Rotate(TopLeft, Center, s, c);
				Rotate(TopRight, Center, s, c);
				Rotate(BottomLeft, Center, s, c);
				Rotate(BottomRight, Center, s, c);
			}


			//Setup vertices in buffer

			batchVertices[NumberOfBatchedVertices].color =Sprite.GetSpriteColor().GetColor_ByCorner(cSpriteColor::Corner_TopLeft).GetARGBColor();
			batchVertices[NumberOfBatchedVertices].x = TopLeft.x;
			batchVertices[NumberOfBatchedVertices].y = TopLeft.y;
			batchVertices[NumberOfBatchedVertices].z = Z;
			batchVertices[NumberOfBatchedVertices].u = RenderInfo.mTexture->GetTextureInfo().mLeft;
			batchVertices[NumberOfBatchedVertices].v = RenderInfo.mTexture->GetTextureInfo().mTop;

			batchVertices[NumberOfBatchedVertices + 1].color =Sprite.GetSpriteColor().GetColor_ByCorner(cSpriteColor::Corner_TopRight).GetARGBColor();
			batchVertices[NumberOfBatchedVertices + 1].x = TopRight.x;
			batchVertices[NumberOfBatchedVertices + 1].y = TopRight.y;
			batchVertices[NumberOfBatchedVertices + 1].z = Z;
			batchVertices[NumberOfBatchedVertices + 1].u = RenderInfo.mTexture->GetTextureInfo().mRight;
			batchVertices[NumberOfBatchedVertices + 1].v = RenderInfo.mTexture->GetTextureInfo().mTop;

			batchVertices[NumberOfBatchedVertices + 2].color =Sprite.GetSpriteColor().GetColor_ByCorner(cSpriteColor::Corner_BottomRight).GetARGBColor();
			batchVertices[NumberOfBatchedVertices + 2].x = BottomRight.x;
			batchVertices[NumberOfBatchedVertices + 2].y = BottomRight.y;
			batchVertices[NumberOfBatchedVertices + 2].z = Z;
			batchVertices[NumberOfBatchedVertices + 2].u = RenderInfo.mTexture->GetTextureInfo().mRight;
			batchVertices[NumberOfBatchedVertices + 2].v = RenderInfo.mTexture->GetTextureInfo().mBottom;

			batchVertices[NumberOfBatchedVertices + 3].color =Sprite.GetSpriteColor().GetColor_ByCorner(cSpriteColor::Corner_BottomLeft).GetARGBColor();
			batchVertices[NumberOfBatchedVertices + 3].x = BottomLeft.x;
			batchVertices[NumberOfBatchedVertices + 3].y = BottomLeft.y;
			batchVertices[NumberOfBatchedVertices + 3].z = Z;
			batchVertices[NumberOfBatchedVertices + 3].u = RenderInfo.mTexture->GetTextureInfo().mLeft;
			batchVertices[NumberOfBatchedVertices + 3].v = RenderInfo.mTexture->GetTextureInfo().mBottom;

			batchVertices[NumberOfBatchedVertices].rhw=1.0f;
			batchVertices[NumberOfBatchedVertices+1].rhw=1.0f;
			batchVertices[NumberOfBatchedVertices+2].rhw=1.0f;
			batchVertices[NumberOfBatchedVertices+3].rhw=1.0f;
			//Increase vertex count
			NumberOfBatchedVertices += 4;
			//Flush buffer if it's full or no more sprite to draw
		}
		++i;
		if(NumberOfBatchedVertices == mMaxSpritesPerFlush * 4  || i.IsAtEnd())
		{
			FlushBuffer(batchVertices, NumberOfBatchedVertices, !i.IsAtEnd());
		}
	}
}

void cSpriteRenderer::FlushBuffer(cSpriteVertexData* batchVertices, int &NumberOfBatchedVertices, bool RelockBuffer)
{
	if(!NumberOfBatchedVertices)
		return;

	D3V(mVertexBuffer->Unlock());

	//Draw quads in the buffer
	D3V(mDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, NumberOfBatchedVertices, 0, NumberOfBatchedVertices / 2));

	//Reset vertex count        
	NumberOfBatchedVertices = 0;

	//Lock vertex buffer again
	if(RelockBuffer)
	{
		D3V(mVertexBuffer->Lock(0, mMaxSpritesPerFlush * 4 * sizeof(cSpriteVertexData), (void **)&batchVertices, 0));
	}
}

void cSpriteRenderer::UpdateUsedTextures()
{
 	for(cPixieWindow::cSpriteIterator i=mBaseWindow.CreateSpriteIterator(); !i.IsAtEnd(); ++i)
 	{
		cSpriteRenderInfo RenderInfo=i->GetRenderInfo();
		if(RenderInfo.mTexture&&RenderInfo.mTexture->DoesNeedUpdateBeforeUse())
		{
			const_cast<cTexture *>(RenderInfo.mTexture)->Update();  // todo... this might be considered a late init pattern... 
		}
 	}
}

void cSpriteRenderer::Render()
{
	if(!mIsInitDone)
		Init();
	
	UpdateUsedTextures();

	StopOnError(mDevice->SetRenderTarget(0, mRenderSurface));
	if(mClearBeforeRender)
	{
		StopOnError(theDevice->GetD3DObject()->Clear(0,  //Number of rectangles to clear, we're clearing everything so set it to 0
			nullptr, //Pointer to the rectangles to clear, NULL to clear whole display
			D3DCLEAR_TARGET,   //What to clear.  We don't have a Z Buffer or Stencil Buffer
			0, //Colour to clear to (AARRGGBB)
			1.0f,  //Value to clear ZBuffer to, doesn't matter since we don't have one
			0));   //Stencil clear value, again, we don't have one, this value doesn't matter
	}
	D3V(mDevice->SetVertexShader(NULL));
	D3V(mDevice->SetFVF(cSpriteVertexData::FVF));
	D3V(mDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE));
	mDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
 	mDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
 	mDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);

	mBaseWindow.CheckOwnerlessSprites();
	RenderSprites();
}

