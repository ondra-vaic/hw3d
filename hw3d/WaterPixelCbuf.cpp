#include <chrono>
#include "WaterPixelCbuf.h"

namespace Bind
{
	WaterPixelCbuf::WaterPixelCbuf(Graphics& gfx, const Drawable& parent, UINT slot)
		:
		parent(parent)
	{
		startTime = getTime();
		
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
		const int timeSinceStart = getTime() - startTime;
		return {
			gfx.GetCameraPosition(),
			mainCloseColor,
			mainFarColor,
			static_cast<float>(timeSinceStart),
			(float)gfx.GetViewportWidth(),
			(float)gfx.GetViewportHeight(),
		};
	}

	int WaterPixelCbuf::getTime()
	{
		using namespace std::chrono;
		auto ms = duration_cast<milliseconds>(
			system_clock::now().time_since_epoch()
			);

		return static_cast<int>(ms.count());
	}

	std::unique_ptr<PixelConstantBuffer<WaterPixelCbuf::WaterPixelParams>> WaterPixelCbuf::pPcbuf;
}