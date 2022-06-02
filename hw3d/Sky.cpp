#include "Sky.h"
#include "Plane.h"
#include "BindableCommon.h"
#include "Cube.h"
#include "RenderTexture.h"
#include "TransformCbufDoubleboi.h"
#include "imgui/imgui.h"
#include "WaterCbuf.h"

Sky::Sky(Graphics& gfx, float size, DirectX::XMFLOAT4 lowColor, DirectX::XMFLOAT4 highColor)
	:
	pmc({ lowColor, highColor })
{
	using namespace Bind;
	namespace dx = DirectX;

	auto model = Cube::MakeIndependentTextured(1);
	model.Transform(dx::XMMatrixScaling(size, size, size));
	const auto geometryTag = "$plane." + std::to_string(size);
	AddBind(VertexBuffer::Resolve(gfx, geometryTag, model.vertices));
	AddBind(IndexBuffer::Resolve(gfx, geometryTag, model.indices));

	auto pvs = VertexShader::Resolve(gfx, "SkyVs.cso");
	auto pvsbc = pvs->GetBytecode();
	AddBind(std::move(pvs));

	defaultShader = PixelShader::Resolve(gfx, "skyPS.cso");
	depthShader = PixelShader::Resolve(gfx, "DefaultDepthPS.cso");
	currentShader = defaultShader;

	AddBind(PixelConstantBuffer<SkyPSMaterialConstant>::Resolve(gfx, pmc, 3u));

	AddBind(InputLayout::Resolve(gfx, model.vertices.GetLayout(), pvsbc));

	AddBind(Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

	auto tcbdb = std::make_shared<TransformCbufDoubleboi>(gfx, *this, 0u, 2u);
	AddBind(tcbdb);

	AddBind(Rasterizer::Resolve(gfx, true));
}

void Sky::SpawnControlWindow(Graphics& gfx, const std::string& name) noexcept
{
	if (ImGui::Begin(name.c_str()))
	{
		ImGui::ColorEdit3("Low color", &pmc.lowColor.x);
		ImGui::ColorEdit3("High color", &pmc.highColor.x);

		pmc.cameraPosition = gfx.GetCameraPosition();

		QueryBindable<Bind::PixelConstantBuffer<SkyPSMaterialConstant>>()->Update(gfx, pmc);
	}
	ImGui::End();
}

DirectX::XMMATRIX Sky::GetTransformXM() const noexcept
{
	return DirectX::XMMatrixRotationRollPitchYaw(0, 0, 0) *
		DirectX::XMMatrixTranslation(0, 0, 0);

}
