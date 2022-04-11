#pragma once
#include "ConstantBuffers.h"
#include "Drawable.h"

namespace Bind
{
	class WaterCbuf : public Bindable
	{
	protected:
		struct WaterParams
		{
			alignas(16) float time;
			DirectX::XMMATRIX modelView;
			DirectX::XMMATRIX modelViewProjection;
			DirectX::XMMATRIX model;
			DirectX::XMMATRIX view;
			DirectX::XMMATRIX projection;
		};
	public:
		WaterCbuf(Graphics& gfx, const Drawable& parent, UINT slot = 0u);
		void Bind(Graphics& gfx) noexcept override;
	protected:
		void UpdateBindImpl(Graphics& gfx, const WaterParams& tf) noexcept;
		WaterParams GetWaterParams(Graphics& gfx) noexcept;
	private:
		int getTime();
		static std::unique_ptr<VertexConstantBuffer<WaterParams>> pVcbuf;
		const Drawable& parent;
		int startTime;
	};
}