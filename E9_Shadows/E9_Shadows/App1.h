// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "TextureShader.h"
#include "ShadowShader.h"
#include "DepthShader.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void depthPass();
	void finalPass();
	void gui();

private:
	TextureShader* textureShader;
	PlaneMesh* mesh;
	CubeMesh* cube;

	Light* light;
	AModel* model;
	ShadowShader* shadowShader;
	DepthShader* depthShader;
	
	OrthoMesh* orthoMesh;
	
	ShadowMap* shadowMap;

	XMFLOAT3 cubePosition;
	XMFLOAT3 lightPosition;
	XMFLOAT3 lightDirection;
	SphereMesh* lightSphereMesh;

	bool viewFromLight;
};

#endif