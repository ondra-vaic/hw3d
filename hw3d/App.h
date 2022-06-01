#pragma once
#include "Window.h"
#include "ChiliTimer.h"
#include "ImguiManager.h"
#include "Camera.h"
#include "LightData.h"
#include "TestPlane.h"
#include "TestCube.h"
#include "Mesh.h"
#include <set>
#include "ScriptCommander.h"
#include "Stencil.h"
#include "Water.h"

class Sky;

class App
{
public:
	App( const std::string& commandLine = "" );
	// master frame / message loop
	int Go();
	~App();
private:
	void DoFrame();
	void ShowImguiDemoWindow();
	void Render(bool ignoreUi);
	void RenderToTexture(RenderTexture* renderTexture, float alphaClear);
private:
	std::string commandLine;
	bool showDemoWindow = false;
	ImguiManager imgui;
	Window wnd;
	ScriptCommander scriptCommander;
	ChiliTimer timer;
	float speed_factor = 1.0f;
	Camera cam;
	LightData* light;

	Water* water; //{ wnd.Gfx(),50.0f,{ 0.3f,0.3f,1.0f,0.5f } };
	TestCube* cube;
	Sky* sky;

	RenderTexture* worldTexture;
	RenderTexture* reflectedWorldTexture;
	std::vector<Drawable*> scene;

	void createPool();

	//Model sponza{ wnd.Gfx(),"Models\\sponza\\sponza.obj",1.0f / 20.0f };
	//TestPlane redPlane{ wnd.Gfx(),6.0f,{ 1.0f,0.3f,0.3f,0.0f } };
	//Model gobber{ wnd.Gfx(),"Models\\gobber\\GoblinX.obj",6.0f };
	//Model wall{ wnd.Gfx(),"Models\\brick_wall\\brick_wall.obj",6.0f };
	//TestPlane tp{ wnd.Gfx(),6.0 };
	//Model nano{ wnd.Gfx(),"Models\\nano_textured\\nanosuit.obj",2.0f };
};
