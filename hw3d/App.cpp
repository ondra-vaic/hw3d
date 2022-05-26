#include "App.h"
#include <memory>
#include <algorithm>
#include "ChiliMath.h"
#include "Surface.h"
#include "imgui/imgui.h"
#include "VertexBuffer.h"
#include "ChiliUtil.h"
#include "RenderTexture.h"
#include "Testing.h"

namespace dx = DirectX;

App::App( const std::string& commandLine )
	:
	commandLine( commandLine ),
	wnd( 1280,720,"The Donkey Fart Box" ),
	scriptCommander( TokenizeQuoted( commandLine ) )
{
	TestDynamicConstant();
	water = new Water{ wnd.Gfx(),50.0f,{ 0.3f,0.3f,1.0f,0.5f } };
	light = new PointLight(wnd.Gfx());
	ground = new TestPlane{ wnd.Gfx(), 50.0f, { 0.6f,0.6f,0.6f,1 } };

	scene.emplace_back(water);
	scene.emplace_back(ground);

	worldTexture = new RenderTexture();
	worldTexture->Initialize(wnd.Gfx().GetDevice(), wnd.Gfx().GetViewportWidth(), wnd.Gfx().GetViewportHeight());

	water->SetPos( {0, 0, 50} );
	water->SetRotation(PI / 2, PI, 0);
	ground->SetPos({ 0, -10, 50 });
	ground->SetRotation(PI / 2, 0, 0);
	wnd.Gfx().SetProjection( dx::XMMatrixPerspectiveLH( 1.0f,9.0f / 16.0f,0.5f,400.0f ) );
}


void App::Render(bool ignoreWater)
{
	light->Bind(wnd.Gfx(), cam.GetMatrix());

	light->Draw(wnd.Gfx());
	for (auto drawable : scene)
	{
		if(ignoreWater && drawable == water) continue;

		drawable->Draw(wnd.Gfx());
	}

	if(!ignoreWater)
	{
		// imgui windows
		cam.SpawnControlWindow();
		light->SpawnControlWindow();
		ShowImguiDemoWindow();
		water->SpawnControlWindow(wnd.Gfx(), "Water");
		ground->SpawnControlWindow(wnd.Gfx(), "Ground");
	}
}

void App::RenderToTexture()
{
	// first pass
	worldTexture->SetRenderTarget(wnd.Gfx().GetDeviceContext(), wnd.Gfx().GetDepthStencilView());
	worldTexture->ClearRenderTarget(wnd.Gfx().GetDeviceContext(), wnd.Gfx().GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);
	Render(true);
	wnd.Gfx().SetBackBufferRenderTarget();
}

void App::DoFrame()
{
	wnd.Gfx().SetCamera(cam);
	const auto dt = timer.Mark() * speed_factor;

	RenderToTexture();

	wnd.Gfx().BeginFrame(0.07f, 0.0f, 0.12f);
	water->SetWorldTexture(wnd.Gfx(), worldTexture->GetShaderResourceView());

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