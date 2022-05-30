#include "App.h"
#include <memory>
#include <algorithm>
#include "ChiliMath.h"
#include "Surface.h"
#include "imgui/imgui.h"
#include "VertexBuffer.h"
#include "ChiliUtil.h"
#include "Cube.h"
#include "RenderTexture.h"
#include "Testing.h"

namespace dx = DirectX;

App::App( const std::string& commandLine )
	:
	commandLine( commandLine ),
	wnd( 1280,720,"The Donkey Fart Box" ),
	scriptCommander( TokenizeQuoted( commandLine ) ),
	light(new PointLight(wnd.Gfx())),
	water(new Water{wnd.Gfx(), 50.0f, {0.3f, 0.3f, 1.0f, 0.5f}}),
	ground(new TestPlane{wnd.Gfx(), 50.0f, {0.6f, 0.6f, 0.6f, 1}}),
	cube(new TestCube(wnd.Gfx(), 25)),
	worldTexture(new RenderTexture()),
	reflectedWorldTexture(new RenderTexture())
{
	TestDynamicConstant();
	cube->SetPos({ 0, 15, 45});

	scene.emplace_back(ground);
	scene.emplace_back(cube);
	scene.emplace_back(water);

	worldTexture->Initialize(wnd.Gfx().GetDevice(), wnd.Gfx().GetViewportWidth(), wnd.Gfx().GetViewportHeight());
	reflectedWorldTexture->Initialize(wnd.Gfx().GetDevice(), wnd.Gfx().GetViewportWidth(), wnd.Gfx().GetViewportHeight());

	water->SetPos( {0, 0, 50} );
	water->SetRotation(PI / 2, PI, 0);
	ground->SetPos({ 0, -10, 50 });
	ground->SetRotation(PI / 2, 0, 0);

	//wnd.Gfx().SetProjection( dx::XMMatrixPerspectiveLH( 1.0f,9.0f / 16.0f,0.5f,400.0f ) );
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
		ground->SpawnControlWindow(wnd.Gfx(), "Ground");
		cube->SpawnControlWindow(wnd.Gfx(), "Cube");
	}
}

void App::RenderToTexture(RenderTexture* renderTexture)
{
	renderTexture->SetRenderTarget(wnd.Gfx().GetDeviceContext(), wnd.Gfx().GetDepthStencilView());
	renderTexture->ClearRenderTarget(wnd.Gfx().GetDeviceContext(), wnd.Gfx().GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);
	Render(true);
	wnd.Gfx().SetBackBufferRenderTarget();
}

void App::DoFrame()
{
	const auto dt = timer.Mark() * speed_factor;

	water->SetDrawMask(true);

	cam.SetReflectionPlaneY(water->GetY());
	cam.SetIsReflected(true);
	wnd.Gfx().SetCamera(cam);
	wnd.Gfx().SetObliqueClippingPlane(water->GetY());

	RenderToTexture(reflectedWorldTexture);
	water->SetReflectedWorldTexture(reflectedWorldTexture->GetShaderResourceView());

	cam.SetIsReflected(false);
	wnd.Gfx().SetCamera(cam);
	wnd.Gfx().SetStandardProjection();

	RenderToTexture(worldTexture);
	water->SetWorldTexture(worldTexture->GetShaderResourceView());

	water->SetDrawMask(false);

	wnd.Gfx().BeginFrame(0.07f, 0.0f, 0.12f);
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