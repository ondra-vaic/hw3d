#include "Water.h"

#include <chrono>

#include "Plane.h"
#include "BindableCommon.h"
#include "RenderTexture.h"
#include "Stencil.h"
#include "imgui/imgui.h"
#include "TransformCbufDoubleboi.h"
#include "WaterCbuf.h"
#include "WaterPixelCbuf.h"

Water::Water(Graphics& gfx, float size, DirectX::XMFLOAT3 mainCloseColor, DirectX::XMFLOAT3 mainFarColor)
	:
	drawMode(WATER)
{
	using namespace Bind;
	namespace dx = DirectX;

	startTime = getTime();

	auto model = Plane::Make(250, size);
	model.Transform(dx::XMMatrixScaling(size, size, 1.0f));
	const auto geometryTag = "$plane." + std::to_string(size);
	AddBind(VertexBuffer::Resolve(gfx, geometryTag, model.vertices));
	AddBind(IndexBuffer::Resolve(gfx, geometryTag, model.indices));

	AddBind(Texture::Resolve(gfx, "Images\\waterNormal3.png"));
	AddBind(Texture::Resolve(gfx, "Images\\fig02-07.jpg", 4));
	AddBind(Sampler::Resolve(gfx));

	auto pvs = VertexShader::Resolve(gfx, "WaterVS.cso");
	auto pvsbc = pvs->GetBytecode();
	AddBind(std::move(pvs));

	defaultShader =  PixelShader::Resolve(gfx, "WaterPS.cso");
	maskShader = PixelShader::Resolve(gfx, "WaterMaskPS.cso");
	depthShader = PixelShader::Resolve(gfx, "WaterDepthPS.cso");

	AddBind(PixelConstantBuffer<WaterPSMaterialConstant>::Resolve(gfx, pmc, 1u));

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

		ImGui::ColorEdit3("Water close color", &pmc.mainCloseColor.x);
		ImGui::ColorEdit3("Water far color", &pmc.mainFarColor.x);

		ImGui::SliderFloat("Normal Map0 Scale", &normalMap0Scale, 0.0f, 100, "%.01f");
		ImGui::SliderFloat("Normal Map1 Scale", &normalMap1Scale, 0.0f, 100, "%.01f");
		ImGui::SliderFloat("Normal Map0 ScrollSpeed X", &normalMap0scrollSpeed.x, -10, 10, "%.01f");
		ImGui::SliderFloat("Normal Map0 ScrollSpeed Y", &normalMap0scrollSpeed.y, -10, 10, "%.01f");
		ImGui::SliderFloat("Normal Map1 ScrollSpeed X", &normalMap1scrollSpeed.x, -10, 10, "%.01f");
		ImGui::SliderFloat("Normal Map1 ScrollSpeed Y", &normalMap1scrollSpeed.y, -10, 10, "%.01f");

		ImGui::SliderFloat("Specular", &specular, 0.0f, 150, "%.01f");

		ImGui::SliderFloat("Specular Power", &pmc.specularPower, 0.0f, 8.0f, "%.01f");

		ImGui::SliderFloat("SkyBox Weight", &skyBoxWeight, 0.0f, 150, "%.01f");
		ImGui::SliderFloat("Refraction Distortion Strength", &refractionDistortionStrength, 0.0f, 150, "%.01f");
		ImGui::SliderFloat("Reflection Distortion Strength", &reflectionDistortionStrength, 0.0f, 150, "%.01f");
		ImGui::SliderFloat("Fresnel Bias", &fresnelBias, 0.0f, 100, "%.01f");

		ImGui::SliderFloat("Fresnel Power", &pmc.fresnelPower, 0.0f, 100.0f, "%.01f");
		ImGui::SliderFloat("Fresnel Distant Scale", &pmc.fresnelDistantScale, 0.0f, 35.0f, "%.01f");
		ImGui::SliderFloat("Caustics Power", &pmc.causticsPower, 0.0f, 35.0f, "%.01f");
		ImGui::SliderFloat("Caustics Length", &pmc.causticsLength, 0.0f, 35.0f, "%.01f");

		ImGui::SliderFloat("Caustics Depth FallOff", &causticsDepthFallOff, 0.0f, 100, "%.01f");

		ImGui::SliderFloat("Depth Cut Off Bias", &depthCutOff, 0.0f, 100, "%.01f");
		ImGui::SliderFloat("Depth Far", &pmc.depthFar, 0.0f, 600.0f, "%.01f");
		const int timeSinceStart = getTime() - startTime;

		pmc.specular = specular / 100;
		pmc.skyBoxWeight = skyBoxWeight / 100;
		pmc.refractionDistortionStrength = refractionDistortionStrength / 100;
		pmc.reflectionDistortionStrength = reflectionDistortionStrength / 100;
		pmc.depthCutOff = depthCutOff / 100;
		pmc.fresnelBias = fresnelBias / 100;
		pmc.causticsDepthFallOff = causticsDepthFallOff / 100;
		pmc.normalMap0Scale = normalMap0Scale / 1000;
		pmc.normalMap1Scale = normalMap1Scale / 1000;

		pmc.normalMap0scrollSpeedX= normalMap0scrollSpeed.x / 10000;
		pmc.normalMap0scrollSpeedY = normalMap0scrollSpeed.y / 10000;
		pmc.normalMap1scrollSpeedX = normalMap1scrollSpeed.y / 10000;
		pmc.normalMap1scrollSpeedY = normalMap1scrollSpeed.y / 10000;

		pmc.cameraPosition = gfx.GetCameraPosition();
		pmc.time = static_cast<float>(timeSinceStart);
		pmc.width = static_cast<float>(gfx.GetViewportWidth());
		pmc.height = static_cast<float>(gfx.GetViewportHeight());

		pmc.planeY = GetY();

		QueryBindable<Bind::PixelConstantBuffer<WaterPSMaterialConstant>>()->Update(gfx, pmc);
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

int Water::getTime()
{
	using namespace std::chrono;
	auto ms = duration_cast<milliseconds>(
		system_clock::now().time_since_epoch()
		);

	return static_cast<int>(ms.count());
}