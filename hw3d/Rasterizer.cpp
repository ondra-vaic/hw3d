#include "Rasterizer.h"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.h"

namespace Bind
{
	Rasterizer::Rasterizer( Graphics& gfx,bool flippedSide )
		:
		flippedSide( flippedSide )
	{
		INFOMAN( gfx );

		D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC( CD3D11_DEFAULT{} );
		rasterDesc.CullMode = flippedSide ? D3D11_CULL_FRONT  : D3D11_CULL_BACK;

		GFX_THROW_INFO( GetDevice( gfx )->CreateRasterizerState( &rasterDesc,&pRasterizer ) );
	}

	void Rasterizer::Bind( Graphics& gfx ) noexcept
	{
		GetContext( gfx )->RSSetState( pRasterizer.Get() );
	}
	
	std::shared_ptr<Rasterizer> Rasterizer::Resolve( Graphics& gfx,bool flippedSide )
	{
		return Codex::Resolve<Rasterizer>( gfx,flippedSide );
	}
	std::string Rasterizer::GenerateUID( bool flippedSide )
	{
		using namespace std::string_literals;
		return typeid(Rasterizer).name() + "#"s + (flippedSide ? "2s" : "1s");
	}
	std::string Rasterizer::GetUID() const noexcept
	{
		return GenerateUID( flippedSide );
	}
}