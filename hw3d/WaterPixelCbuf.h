#pragma once
#include "ConstantBuffers.h"
#include "Drawable.h"

namespace Bind
{
	class WaterPixelCbuf : public Bindable
	{
	protected:
		struct WaterPixelParams
		{
			DirectX::XMFLOAT3 cameraPosition;
			float time;
		};
	public:
		WaterPixelCbuf(Graphics& gfx, const Drawable& parent, UINT slot = 0u);
		void Bind(Graphics& gfx) noexcept override;
	protected:
		void UpdateBindImpl(Graphics& gfx, const WaterPixelParams& tf) noexcept;
		WaterPixelParams GetWaterParams(Graphics& gfx) noexcept;
	private:
		int getTime();
		static std::unique_ptr<PixelConstantBuffer<WaterPixelParams>> pPcbuf;
		const Drawable& parent;
		int startTime;
	};
}