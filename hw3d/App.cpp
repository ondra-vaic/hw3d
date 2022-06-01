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
	water(new Water{wnd.Gfx(), 250.0f, {37 / 255.0f, 163/ 255.0f, 189/ 255.0f}, {0 / 255.0f, 21 / 255.0f, 70 / 255.0f} }),
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

	water->SetPos( {0, 0, 0} );
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

	scene.emplace_back(back);
	scene.emplace_back(ground);
	scene.emplace_back(right);
	scene.emplace_back(left);
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
	wnd.Gfx().SetObliqueClippingPlane(water->GetY());

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