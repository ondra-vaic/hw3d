#include "App.h"
#include <memory>
#include <algorithm>
#include <sstream>

#include "ChiliMath.h"
#include "Surface.h"
#include "imgui/imgui.h"
#include "VertexBuffer.h"
#include "ChiliUtil.h"
#include "Cube.h"
#include "RenderTexture.h"
#include "Sky.h"
#include "Testing.h"

namespace dx = DirectX;

App::App( const std::string& commandLine )
	:
	commandLine( commandLine ),
	wnd( 1280,720,"The Donkey Fart Box" ),
	scriptCommander( TokenizeQuoted( commandLine ) ),
	light(new LightData(wnd.Gfx())),
	water(new Water{wnd.Gfx(), 250.0f, {190 / 255.0f, 195 / 255.0f, 196 / 255.0f}, {11 / 255.0f, 7/ 255.0f, 25 / 255.0f} }),
	cube(new TestCube(wnd.Gfx(), 25)),
	sky(new Sky(wnd.Gfx(), 350, {219/255.0f, 203/255.0f, 201/ 255.0f, 1}, { 73/ 255.0f, 124/ 255.0f, 161 / 255.0f, 1})),
	worldTexture(new RenderTexture()),
	reflectedWorldTexture(new RenderTexture()),
	depthTexture(new RenderTexture())
{
	TestDynamicConstant();
	cube->SetPos({ 0, 15, 45});

	createPool();
	//scene.emplace_back(cube);
	scene.emplace_back(sky);
	scene.emplace_back(water);

	worldTexture->Initialize(wnd.Gfx().GetDevice(), wnd.Gfx().GetViewportWidth(), wnd.Gfx().GetViewportHeight());
	reflectedWorldTexture->Initialize(wnd.Gfx().GetDevice(), wnd.Gfx().GetViewportWidth(), wnd.Gfx().GetViewportHeight());
	depthTexture->Initialize(wnd.Gfx().GetDevice(), wnd.Gfx().GetViewportWidth(), wnd.Gfx().GetViewportHeight());

	water->SetPos( {0, -2.7f, 0} );
	water->SetRotation(PI / 2, PI, 0);
}

void App::createPool()
{
	TestCube* ground = new TestCube{ wnd.Gfx(), 250.0f };

	ground->SetPos({ 0, -125, -40 });
	ground->SetRotation(PI / 2 + 0.35f, 0, 0);

	TestCube* right = new TestCube(wnd.Gfx(), 150);

	right->SetPos({ 40 + 75, -75 + 0.5f, 5 });
	right->SetRotation(0, PI / 2, 0);

	TestCube* left = new TestCube(wnd.Gfx(), 150);

	left->SetPos({ -40 - 75, -75 + 0.5f, 5 });
	left->SetRotation(0, -PI / 2, 0);

	TestCube* back = new TestCube(wnd.Gfx(), 450);

	back->SetPos({ 0, -450 * .5f + 0.5f, 450 * .5f + 80 });
	back->SetRotation(0, -PI / 2, 0);

	TestCube* down = new TestCube(wnd.Gfx(), 25);

	down->SetPos({ 0, -23, 35 });
	down->SetRotation(0, -PI / 4, PI / 4);

	TestCube* rightBack = new TestCube(wnd.Gfx(), 45);

	rightBack->SetPos({ 35, -40, 75 });
	rightBack->SetRotation(PI / 4,  PI/4, PI / 4);

	TestCube* leftTop = new TestCube(wnd.Gfx(), 5.6);

	leftTop->SetPos({ -15.5f, 3, 20 });
	leftTop->SetRotation(PI / 4, PI / 4, PI / 4);

	TestCube* leftTop1 = new TestCube(wnd.Gfx(), 5.6);

	leftTop1->SetPos({ -24.5f, 4, 14 });
	leftTop1->SetRotation(PI / 4, 3 * PI / 4, PI / 4);

	TestCube* backTriangle = new TestCube(wnd.Gfx(), 25);

	backTriangle->SetPos({ 0, 0, 80 + 0.03f + 25 *.5f });
	backTriangle->SetRotation(PI / 4, PI/2, 0);

	TestCube* underwaterCube = new TestCube(wnd.Gfx(), 10);

	underwaterCube->SetPos({ 0, -4, 10});
	underwaterCube->SetRotation(0, 0, 0);

	TestCube* underwaterCube1 = new TestCube(wnd.Gfx(), 50);

	underwaterCube1->SetPos({ 40 + 25 - 10, -25 - 4.5f, 16 });
	underwaterCube1->SetRotation(0, 0, 0);

	TestCube* underwaterCube2 = new TestCube(wnd.Gfx(), 50);

	underwaterCube2->SetPos({ 40 + 25 - 12, -25 - 5.5f, 12 });
	underwaterCube2->SetRotation(0.06f, -0.15f, 0);

	TestCube* underwaterCube3 = new TestCube(wnd.Gfx(), 50);

	underwaterCube3->SetPos({ 40 + 25 - 12.5f, -25 - 6.5f, 9 });
	underwaterCube3->SetRotation(0.18f, -0.25f, 0);

	
	scene.emplace_back(rightBack);
	scene.emplace_back(leftTop);
	scene.emplace_back(leftTop1);
	
	scene.emplace_back(down);
	scene.emplace_back(back);
	scene.emplace_back(ground);
	scene.emplace_back(right);
	scene.emplace_back(left);
	scene.emplace_back(backTriangle);
	scene.emplace_back(underwaterCube);
	scene.emplace_back(underwaterCube1);
	scene.emplace_back(underwaterCube2);
	scene.emplace_back(underwaterCube3);
}

void App::Render(bool ignoreUi)
{
	light->Bind(wnd.Gfx(), cam.GetMatrix());

	light->Draw(wnd.Gfx());
	for (auto drawable : scene)
	{
		drawable->Draw(wnd.Gfx());
	}

	if(!ignoreUi)
	{
		// imgui windows
		cam.SpawnControlWindow();
		light->SpawnControlWindow();
		ShowImguiDemoWindow();
		water->SpawnControlWindow(wnd.Gfx(), "Water");
		cube->SpawnControlWindow(wnd.Gfx(), "Cube");
		sky->SpawnControlWindow(wnd.Gfx(), "Sky");
		/*
		for (auto element : scene)
		{
			TestPlane* plane = static_cast<TestPlane*>(element);
			if(plane != nullptr)
			{
				std::stringstream name;
				name << "element + " << (int)&element;
				static_cast<TestPlane*>(element)->SpawnControlWindow(wnd.Gfx(), name.str());
			}
		}
		*/
	}
}

void App::RenderToTexture(RenderTexture* renderTexture, float alphaClear)
{
	renderTexture->SetRenderTarget(wnd.Gfx().GetDeviceContext(), wnd.Gfx().GetDepthStencilView());
	renderTexture->ClearRenderTarget(wnd.Gfx().GetDeviceContext(), wnd.Gfx().GetDepthStencilView(), 0, 0, 0, alphaClear);
	Render(true);
	wnd.Gfx().SetBackBufferRenderTarget();
}

void App::DoFrame()
{
	const auto dt = timer.Mark() * speed_factor;

	water->SetDrawMode(DEPTH);
	wnd.Gfx().SetCamera(cam);
	for (auto element : scene)
	{
		if(element == water) continue;

		element->SetRenderDepth(true);
	}
	RenderToTexture(depthTexture, 0);

	for (auto element : scene)
	{
		element->SetRenderDepth(false);
	}

	water->SetDepthTexture(depthTexture->GetShaderResourceView());

	water->SetDrawMode(MASK);
	cam.SetReflectionPlaneY(water->GetY());
	cam.SetIsReflected(true);
	wnd.Gfx().SetCamera(cam);
	wnd.Gfx().SetObliqueClippingPlane(water->GetY() - 2.7f);

	RenderToTexture(reflectedWorldTexture, 0);
	water->SetReflectedWorldTexture(reflectedWorldTexture->GetShaderResourceView());

	cam.SetIsReflected(false);
	wnd.Gfx().SetCamera(cam);
	wnd.Gfx().SetStandardProjection();

	RenderToTexture(worldTexture, 1);
	water->SetWorldTexture(worldTexture->GetShaderResourceView());

	water->SetDrawMode(WATER);

	wnd.Gfx().BeginFrame(0.0f, 0.0f, 0.0f);
	Render(false);
	wnd.Gfx().EndFrame();

	while( const auto e = wnd.kbd.ReadKey() )
	{
		if( !e->IsPress() )
		{
			continue;
		}

		switch( e->GetCode() )
		{
		case VK_ESCAPE:
			if( wnd.CursorEnabled() )
			{
				wnd.DisableCursor();
				wnd.mouse.EnableRaw();
			}
			else
			{
				wnd.EnableCursor();
				wnd.mouse.DisableRaw();
			}
			break;
		case VK_F1:
			showDemoWindow = true;
			break;
		}
	}

	if( !wnd.CursorEnabled() )
	{
		if( wnd.kbd.KeyIsPressed( 'W' ) )
		{
			cam.Translate( { 0.0f,0.0f,dt } );
		}
		if( wnd.kbd.KeyIsPressed( 'A' ) )
		{
			cam.Translate( { -dt,0.0f,0.0f } );
		}
		if( wnd.kbd.KeyIsPressed( 'S' ) )
		{
			cam.Translate( { 0.0f,0.0f,-dt } );
		}
		if( wnd.kbd.KeyIsPressed( 'D' ) )
		{
			cam.Translate( { dt,0.0f,0.0f } );
		}
		if( wnd.kbd.KeyIsPressed( 'R' ) )
		{
			cam.Translate( { 0.0f,dt,0.0f } );
		}
		if( wnd.kbd.KeyIsPressed( 'F' ) )
		{
			cam.Translate( { 0.0f,-dt,0.0f } );
		}
	}

	while( const auto delta = wnd.mouse.ReadRawDelta() )
	{
		if( !wnd.CursorEnabled() )
		{
			cam.Rotate( (float)delta->x,(float)delta->y );
		}
	}
}

void App::ShowImguiDemoWindow()
{
	if( showDemoWindow )
	{
		ImGui::ShowDemoWindow( &showDemoWindow );
	}
}

App::~App()
{}


int App::Go()
{
	while( true )
	{
		// process all messages pending, but to not block for new messages
		if( const auto ecode = Window::ProcessMessages() )
		{
			// if return optional has value, means we're quitting so return exit code
			return *ecode;
		}
		DoFrame();
	}
}