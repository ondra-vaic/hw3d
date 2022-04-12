#include <chrono>
#include "WaterCbuf.h"

namespace Bind
{
	WaterCbuf::WaterCbuf(Graphics& gfx, const Drawable& parent, UINT slot)
		:
		parent(parent)
	{
		if (!pVcbuf)
		{
			startTime = getTime();
			pVcbuf = std::make_unique<VertexConstantBuffer<WaterParams>>(gfx, slot);
		}
	}

	void WaterCbuf::Bind(Graphics& gfx) noexcept
	{
		UpdateBindImpl(gfx, GetWaterParams(gfx));
	}

	void WaterCbuf::UpdateBindImpl(Graphics& gfx, const WaterParams& tf) noexcept
	{
		pVcbuf->Update(gfx, tf);
		pVcbuf->Bind(gfx);
	}

	WaterCbuf::WaterParams WaterCbuf::GetWaterParams(Graphics& gfx) noexcept
	{
		auto model = parent.GetTransformXM();
		auto view = gfx.GetCameraTransform();
		auto projection = gfx.GetProjection();

		const auto modelView = parent.GetTransformXM() * gfx.GetCameraTransform();

		float timeSinceStart = getTime() - startTime;
		return {
			timeSinceStart,
			DirectX::XMMatrixTranspose(modelView),
			DirectX::XMMatrixTranspose(
				modelView *
				gfx.GetProjection()
			),
			DirectX::XMMatrixTranspose(model),
			DirectX::XMMatrixTranspose(view),
			DirectX::XMMatrixTranspose(projection)
		};
	}

	int WaterCbuf::getTime()
	{
		using namespace std::chrono;
		auto ms = duration_cast<milliseconds>(
			system_clock::now().time_since_epoch()
			);

		return static_cast<int>(ms.count());
	}

	std::unique_ptr<VertexConstantBuffer<WaterCbuf::WaterParams>> WaterCbuf::pVcbuf;
}