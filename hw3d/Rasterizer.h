#pragma once
#include "Bindable.h"
#include <array>

namespace Bind
{
	class Rasterizer : public Bindable
	{
	public:
		Rasterizer( Graphics& gfx,bool flippedSide );
		void Bind( Graphics& gfx ) noexcept override;
		static std::shared_ptr<Rasterizer> Resolve( Graphics& gfx,bool flippedSide );
		static std::string GenerateUID( bool flippedSide );
		std::string GetUID() const noexcept override;
	protected:
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterizer;
		bool flippedSide;
	};
}
