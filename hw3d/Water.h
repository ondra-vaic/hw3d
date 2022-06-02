#pragma once
#include "Drawable.h"

namespace Bind
{
	class WaterPixelCbuf;
	class Blender;
}

enum DrawMode
{
	WATER,
	MASK,
	DEPTH
};
class RenderTexture;

class Water : public Drawable
{
public:
	
	Water(Graphics& gfx, float size, DirectX::XMFLOAT3 mainCloseColor, DirectX::XMFLOAT3 mainFarColor);
	void Draw(Graphics& gfx) const noxnd;
	void SetPos(DirectX::XMFLOAT3 pos) noexcept;
	void SetRotation(float roll, float pitch, float yaw) noexcept;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	void SpawnControlWindow(Graphics& gfx, const std::string& name) noexcept;
	void SetWorldTexture(ID3D11ShaderResourceView* worldTextureResource);
	void SetReflectedWorldTexture(ID3D11ShaderResourceView* reflectedWorldTextureResource);
	void SetDepthTexture(ID3D11ShaderResourceView* depthTextureResource);
	void SetDrawMode(DrawMode drawMode);
	float GetY() const;

private:
	int getTime();

	struct WaterPSMaterialConstant
	{
		DirectX::XMFLOAT3 cameraPosition = {0, 0, 0};
		float padding;
		DirectX::XMFLOAT4 mainCloseColor = {23/255.0f, 54/255.0f, 70/ 255.0f, 1 };
		DirectX::XMFLOAT4 mainFarColor = { 21 / 255.0f, 11 / 255.0f, 48 / 255.0f, 1 };

		float width;
		float height;

		float normalMap0scrollSpeedX;
		float normalMap0scrollSpeedY;
		float normalMap1scrollSpeedX;
		float normalMap1scrollSpeedY;

		float time = 0;
		float planeY = 0;
		float specular = 0.5f;
		float specularPower = 13.f;
		float skyBoxWeight = 0.7f;
		float refractionDistortionStrength = 0.1f;
		float reflectionDistortionStrength = 0.2f;
		float fresnelBias = 0.1f;
		float fresnelPower = 0.8f;
		float fresnelDistantScale = 10.0f;
		float causticsPower = 0.5f;
		float causticsLength = 1.2f;
		float causticsDepthFallOff = 0.25f;
		float depthFar = 444.0f;
		float depthCutOff = 0.5f;
		float normalMap0Scale = 0.05f;
		float normalMap1Scale = 0.016f;
		float pad[1];
	} pmc;

	float specular = 33.0f;
	float skyBoxWeight = 55.0f;
	float refractionDistortionStrength = 8.4f;
	float reflectionDistortionStrength = 2.4f;
	float causticsDepthFallOff = 9455.0f;

	float fresnelBias = 30.9f;
	float depthCutOff = 86.5f;
	float normalMap0Scale = 55.1f;
	float normalMap1Scale = 28.9f;
	DirectX::XMFLOAT2 normalMap0scrollSpeed = { 1.1f, -1.1f };
	DirectX::XMFLOAT2 normalMap1scrollSpeed = { 0.7f,  0.6f };

	DirectX::XMFLOAT3 pos = { 0.0f,0.0f,0.0f };
	float roll = 0.0f;
	float pitch = 0.0f;
	float yaw = 0.0f;
	int startTime;

	DrawMode drawMode;

	ID3D11ShaderResourceView* worldTextureResource;
	ID3D11ShaderResourceView* reflectedWorldTextureResource;
	ID3D11ShaderResourceView* depthTextureResource;
	
	std::shared_ptr<Bind::Bindable> maskShader;
	std::shared_ptr<Bind::Blender> blender;
};
