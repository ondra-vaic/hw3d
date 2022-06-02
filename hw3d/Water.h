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
		float aa;
		DirectX::XMFLOAT4 mainCloseColor = {0, 40/255.0f, 50 / 255.0f, 1 };
		DirectX::XMFLOAT4 mainFarColor = { 15 / 255.0f, 6 / 255.0f, 49 / 255.0f, 1 };

		float width;
		float height;

		float normalMap0scrollSpeedX;
		float normalMap0scrollSpeedY;
		float normalMap1scrollSpeedX;
		float normalMap1scrollSpeedY;

		float time = 0;
		float planeY = 0;
		float specular = 0.5f;
		float specularPower = 80.f;
		float skyBoxWeight = 0.7f;
		float refractionDistortionStrength = 0.1f;
		float reflectionDistortionStrength = 0.2f;
		float fresnelBias = 0.1f;
		float fresnelPower = 0.8f;
		float fresnelDistantScale = 10.0f;
		float causticsPower = 0.3f;
		float causticsLength = 14.1;
		float causticsDepthFallOff = 0.25f;
		float depthFar = 175.0f;
		float depthCutOff = 0.5f;
		float normalMap0Scale = 0.05f;
		float normalMap1Scale = 0.016f;
		float pad[1];
	} pmc;

	float specular = 0.7f;
	float skyBoxWeight = 0.7f;
	float refractionDistortionStrength = 5.4f;
	float reflectionDistortionStrength = 2.4f;
	float causticsDepthFallOff = 0.25f;

	float fresnelBias = 32.9f;
	float depthCutOff = 0.5f;
	float normalMap0Scale = 69.5f;
	float normalMap1Scale = 17.7f;
	DirectX::XMFLOAT2 normalMap0scrollSpeed = { 0.00001f,  0.00002f };
	DirectX::XMFLOAT2 normalMap1scrollSpeed = { 0.000007f,  0.00001f };

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
