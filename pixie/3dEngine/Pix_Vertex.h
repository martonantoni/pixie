#pragma once

struct cVertex
{
	FLOAT x, y, z;
	DWORD Color;
	enum { FVF = D3DFVF_DIFFUSE|D3DFVF_XYZ };
};

