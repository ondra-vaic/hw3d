#include "Water.h"
#include "Plane.h"
#include "BindableCommon.h"
#include "RenderTexture.h"
#include "Stencil.h"
#include "imgui/imgui.h"
#include "TransformCbufDoubleboi.h"
#include "WaterCbuf.h"
#include "WaterPixelCbuf.h"

Water::Water(Graphics& gfx, float size, DirectX::XMFLOAT3 mainColor)
	:
	drawMode(WATER)
{
	using namespace Bind;
	namespace dx = DirectX;

	auto model = Plane::Make(250, size);
	model.Transform(dx::XMMatrixScaling(size, size, 1.0f));
	const auto geometryTag = "$plane." + std::to_string(size);
	AddBind(VertexBuffer::Resolve(gfx, geometryTag, model.vertices));
	AddBind(IndexBuffer::Resolve(gfx, geometryTag, model.indices));

	AddBind(Texture::Resolve(gfx, "Images\\waterNormal3.png"));
	AddBind(Sampler::Resolve(gfx));

	auto pvs = VertexShader::Resolve(gfx, "WaterVS.cso");
	auto pvsbc = pvs->GetBytecode();
	AddBind(std::move(pvs));

	defaultShader =  PixelShader::Resolve(gfx, "WaterPS.cso");
	maskShader = PixelShader::Resolve(gfx, "WaterMaskPS.cso");
	depthShader = PixelShader::Resolve(gfx, "WaterDepthPS.cso");

	waterPixelCBuf = std::make_shared<WaterPixelCbuf>(gfx, *this, 1u);
	waterPixelCBuf->mainColor = mainColor;
	AddBind(waterPixelCBuf);

	AddBind(std::make_shared<Stencil>(gfx, Stencil::Mode::Off));

	AddBind(InputLayout::Resolve(gfx, model.vertices.GetLayout(), pvsbc));

	AddBind(Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

	AddBind(std::make_shared<WaterCbuf>(gfx, *this, 0u));

	blender = std::make_shared<Blender>(gfx, true, 1.f);

	AddBind(Rasterizer::Resolve(gfx, false));
}

void Water::SetPos(DirectX::XMFLOAT3 pos) noexcept
{
	this->pos = pos;
}

void Water::SetRotation(float roll, float pitch, float yaw) noexcept
{
	this->roll = roll;
	this->pitch = pitch;
	this->yaw = yaw;
}

DirectX::XMMATRIX Water::GetTransformXM() const noexcept
{
	return DirectX::XMMatrixRotationRollPitchYaw(roll, pitch, yaw) *
		DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
}

void Water::SpawnControlWindow(Graphics& gfx, const std::string& name) noexcept
{
	if (ImGui::Begin(name.c_str()))
	{
		ImGui::Text("Position");
		ImGui::SliderFloat("X", &pos.x, -80.0f, 80.0f, "%.1f");
		ImGui::SliderFloat("Y", &pos.y, -80.0f, 80.0f, "%.1f");
		ImGui::SliderFloat("Z", &pos.z, -80.0f, 80.0f, "%.1f");
		ImGui::Text("Orientation");
		ImGui::SliderAngle("Roll", &roll, -180.0f, 180.0f);
		ImGui::SliderAngle("Pitch", &pitch, -180.0f, 180.0f);
		ImGui::SliderAngle("Yaw", &yaw, -180.0f, 180.0f);
		ImGui::Text("Shading");

		ImGui::ColorEdit3("Water color", &waterPixelCBuf->mainColor.x);
	}
	ImGui::End();
}

void Water::SetWorldTexture(ID3D11ShaderResourceView* worldTextureResource)
{
	this->worldTextureResource = worldTextureResource;
}

void Water::SetReflectedWorldTexture(ID3D11ShaderResourceView* reflectedWorldTextureResource)
{
	this->reflectedWorldTextureResource = reflectedWorldTextureResource;
}

void Water::SetDepthTexture(ID3D11ShaderResourceView* depthTextureResource)
{
	this->depthTextureResource = depthTextureResource;
}

void Water::Draw(Graphics& gfx) const noexcept(!true)
{
	blender->Bind(gfx);

	if(worldTextureResource)
	{
		gfx.GetDeviceContext()->PSSetShaderResources(1, 1u, &worldTextureResource);
	}

	if (reflectedWorldTextureResource)
	{
		gfx.GetDeviceContext()->PSSetShaderResources(2, 1u, &reflectedWorldTextureResource);
	}

	if (depthTextureResource)
	{

		gfx.GetDeviceContext()->PSSetShaderResources(3, 1u, &depthTextureResource);
	}

	Drawable::Draw(gfx);
}

void Water::SetDrawMode(DrawMode drawMode)
{
	blender->SetFactor(!(drawMode != WATER));
	this->drawMode = drawMode;

	switch (drawMode)
	{
	case MASK:
		currentShader = maskShader;
		break;

	case DEPTH:
		currentShader = depthShader;
		break;

	case WATER:
		currentShader = defaultShader;
		break;
	}
}

float Water::GetY() const
{
	return pos.y;
}
