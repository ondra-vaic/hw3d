#include <chrono>
#include "WaterPixelCbuf.h"

namespace Bind
{
	WaterPixelCbuf::WaterPixelCbuf(Graphics& gfx, const Drawable& parent, UINT slot)
		:
		parent(parent)
	{
		if (!pPcbuf)
		{
			pPcbuf = std::make_unique<PixelConstantBuffer<WaterPixelParams>>(gfx, slot);
		}
	}

	void WaterPixelCbuf::Bind(Graphics& gfx) noexcept
	{
		UpdateBindImpl(gfx, GetWaterParams(gfx));
	}

	void WaterPixelCbuf::UpdateBindImpl(Graphics& gfx, const WaterPixelParams& tf) noexcept
	{
		pPcbuf->Update(gfx, tf);
		pPcbuf->Bind(gfx);
	}

	WaterPixelCbuf::WaterPixelParams WaterPixelCbuf::GetWaterParams(Graphics& gfx) noexcept
	{
		float x = gfx.GetCameraPosition().x;
		return {
			gfx.GetCameraPosition()
		};
	}

	std::unique_ptr<PixelConstantBuffer<WaterPixelCbuf::WaterPixelParams>> WaterPixelCbuf::pPcbuf;
}