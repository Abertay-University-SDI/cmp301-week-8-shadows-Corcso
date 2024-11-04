// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1()
{

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Create Mesh object and shader object
	mesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	cube = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());
	cubePosition = XMFLOAT3(1, 1, 1);
	model = new AModel(renderer->getDevice(), "res/teapot.obj");
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");

	// initial shaders
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	shadowShader = new ShadowShader(renderer->getDevice(), hwnd);

	// Variables for defining shadow map
	int shadowmapWidth = 4096;
	int shadowmapHeight = 4096;
	int sceneWidth = 100;
	int sceneHeight = 100;

	// This is your shadow map
	shadowMap = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);

	// Ortho mesh for debug view
	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight);


	// Configure directional light
	light = new Light();
	light->setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
	light->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	light->setDirection(0.0f, -0.7f, 0.7f);
	light->setPosition(0.f, 0.f, -20.f);
	light->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);
	lightSphereMesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());
	lightPosition = light->getPosition();
	lightDirection = light->getDirection();

}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.

}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	int sceneWidth = 100;
	int sceneHeight = 100;
	light->setPosition(lightPosition.x, lightPosition.y, lightPosition.z);
	light->setDirection(lightDirection.x, lightDirection.y, lightDirection.z);
	light->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);

	return true;
}

bool App1::render()
{

	// Perform depth pass
	depthPass();
	// Render scene
	finalPass();

	return true;
}

void App1::depthPass()
{
	// Set the render target to be the render to texture.
	shadowMap->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	// get the world, view, and projection matrices from the camera and d3d objects.
	light->generateViewMatrix();
	XMMATRIX lightViewMatrix = light->getViewMatrix();
	XMMATRIX lightProjectionMatrix = light->getOrthoMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0.f, 7.f, 5.f);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	// Render model
	model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// CUBE
	worldMatrix = XMMatrixTranslation(cubePosition.x, cubePosition.y, cubePosition.z);
	scaleMatrix = XMMatrixScaling(0.7f, 0.9f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	// Render model
	cube->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), cube->getIndexCount());

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	camera->update();

	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, 
		textureMgr->getTexture(L"brick"), shadowMap->getDepthMapSRV(), light);
	shadowShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// Render model
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0.f, 7.f, 5.f);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	model->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), shadowMap->getDepthMapSRV(), light);
	shadowShader->render(renderer->getDeviceContext(), model->getIndexCount());


	// CUBE
	worldMatrix = XMMatrixTranslation(cubePosition.x, cubePosition.y, cubePosition.z);
	scaleMatrix = XMMatrixScaling(0.7f, 0.9f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	// Render model
	cube->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), shadowMap->getDepthMapSRV(), light);
	shadowShader->render(renderer->getDeviceContext(), cube->getIndexCount());


	worldMatrix = XMMatrixTranslation(lightPosition.x, lightPosition.y, lightPosition.z);
	scaleMatrix = XMMatrixScaling(0.2f, 0.2f, 0.2f);
	worldMatrix = XMMatrixMultiply(scaleMatrix, worldMatrix);
	// Render model
	lightSphereMesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), shadowMap->getDepthMapSRV(), light);
	shadowShader->render(renderer->getDeviceContext(), lightSphereMesh->getIndexCount());
	
	if (viewFromLight) {
		worldMatrix = XMMatrixIdentity();
		viewMatrix = camera->getOrthoViewMatrix();
		projectionMatrix = renderer->getOrthoMatrix();

		orthoMesh->sendData(renderer->getDeviceContext());
		textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, shadowMap->getDepthMapSRV());
		textureShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	}
	gui();
	renderer->endScene();
}



void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	ImGui::Checkbox("Light View mode", &viewFromLight);
	ImGui::SliderFloat3("Cube Position", reinterpret_cast<float*>(&cubePosition), -10, 10);
	ImGui::SliderFloat3("Light Position", reinterpret_cast<float*>(&lightPosition), -20, 20);
	ImGui::SliderFloat3("Light Direction", reinterpret_cast<float*>(&lightDirection), -1, 1);

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

