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
	model = new AModel(renderer->getDevice(), "res/drone.obj");
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
	shadowMaps = new ShadowMap * [8];
	shadowMaps[0] = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);
	shadowMaps[1] = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);

	// Ortho mesh for debug view
	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight);


	// Configure directional light
	lights = new Light * [8];
	lights[0] = new Light();

	lights[0]->setAmbientColour(0.1f, 0.02f, 0.02f, 1.0f);
	lights[0]->setDiffuseColour(0.7f, 0.14f, 0.14f, 1.0f);
	lights[0]->setDirection(0.0f, -0.7f, 0.7f);
	lights[0]->setPosition(0.f, 0.f, -20.f);
	lights[0]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);
	lights[0]->generateProjectionMatrix(0.1f, 100.0f);
	
	lights[1] = new Light();
	lights[1]->setAmbientColour(0.02f, 0.02f, 0.1f, 1.0f);
	lights[1]->setDiffuseColour(0.14f, 0.14f, 0.7f, 1.0f);
	lights[1]->setDirection(0.0f, -0.7f, -0.7f);
	lights[1]->setPosition(0.f, 0.f, 40.f);
	lights[1]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);
	lights[1]->generateProjectionMatrix(0.1f, 100.0f);
	lightSphereMesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());
	lightPosition = lights[0]->getPosition();
	lightDirection = lights[0]->getDirection();

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
	lights[0]->setPosition(lightPosition.x, lightPosition.y, lightPosition.z);
	lights[0]->setDirection(lightDirection.x, lightDirection.y, lightDirection.z);
	lights[0]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);
	lights[0]->generateProjectionMatrix(SCREEN_NEAR, SCREEN_DEPTH);

	return true;
}

bool App1::render()
{

	// Perform depth pass
	depthPass(0);
	depthPass(1);
	// Render scene
	finalPass();

	return true;
}

void App1::depthPass(int i)
{
	// Set the render target to be the render to texture.
	shadowMaps[i]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	// get the world, view, and projection matrices from the camera and d3d objects.
	lights[i]->generateViewMatrix();
	XMMATRIX lightViewMatrix = lights[i]->getViewMatrix();
	XMMATRIX lightProjectionMatrix = lights[i]->getOrthoMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0.f, 2.f, 5.f);
	XMMATRIX scaleMatrix = XMMatrixScaling(2.5f, 2.5f, 2.5f);
	worldMatrix = XMMatrixMultiply(scaleMatrix, worldMatrix);
	// Render model
	model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// CUBE
	worldMatrix = XMMatrixTranslation(cubePosition.x, cubePosition.y, cubePosition.z);
	scaleMatrix = XMMatrixScaling(0.7f, 0.9f, 0.5f);
	worldMatrix = XMMatrixMultiply(scaleMatrix, worldMatrix);
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

	ID3D11ShaderResourceView** mapPointers = new ID3D11ShaderResourceView*[8];
	mapPointers[0] = shadowMaps[0]->getDepthMapSRV();
	mapPointers[1] = shadowMaps[1]->getDepthMapSRV();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, 
		textureMgr->getTexture(L"brick"), mapPointers, lights, 2);
	shadowShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// Render model
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0.f, 2.f, 5.f);
	XMMATRIX scaleMatrix = XMMatrixScaling(2.5f, 2.5f, 2.5f);
	worldMatrix = XMMatrixMultiply(scaleMatrix, worldMatrix);
	model->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
		textureMgr->getTexture(L"brick"), mapPointers, lights, 2); 
	shadowShader->render(renderer->getDeviceContext(), model->getIndexCount());


	// CUBE
	worldMatrix = XMMatrixTranslation(cubePosition.x, cubePosition.y, cubePosition.z);
	scaleMatrix = XMMatrixScaling(0.7f, 0.9f, 0.5f);
	worldMatrix = XMMatrixMultiply(scaleMatrix, worldMatrix);
	// Render model
	cube->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
		textureMgr->getTexture(L"brick"), mapPointers, lights, 2); 
	shadowShader->render(renderer->getDeviceContext(), cube->getIndexCount());


	worldMatrix = XMMatrixTranslation(lightPosition.x, lightPosition.y, lightPosition.z);
	scaleMatrix = XMMatrixScaling(0.2f, 0.2f, 0.2f);
	worldMatrix = XMMatrixMultiply(scaleMatrix, worldMatrix);
	// Render model
	lightSphereMesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
		textureMgr->getTexture(L"brick"), mapPointers, lights, 2); 
	shadowShader->render(renderer->getDeviceContext(), lightSphereMesh->getIndexCount());
	
	if (viewFromLight) {
		worldMatrix = XMMatrixIdentity();
		viewMatrix = camera->getOrthoViewMatrix();
		projectionMatrix = renderer->getOrthoMatrix();

		orthoMesh->sendData(renderer->getDeviceContext());
		textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, shadowMaps[0]->getDepthMapSRV());
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

