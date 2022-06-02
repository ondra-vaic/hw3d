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
		DirectX::XMFLOAT4 mainCloseColor = {30/255.0f, 46/255.0f, 54/ 255.0f, 1 };
		DirectX::XMFLOAT4 mainFarColor = { 1 / 255.0f, 20 / 255.0f, 38 / 255.0f, 1 };

		float width;
		float height;

		float normalMap0scrollSpeedX;
		float normalMap0scrollSpeedY;
		float normalMap1scrollSpeedX;
		float normalMap1scrollSpeedY;

		float time = 0;
		float planeY = 0;
		float specular = 0.5f;
		float specularPower = 36.1f;
		float skyBoxWeight = 0.7f;
		float refractionDistortionStrength = 0.1f;
		float reflectionDistortionStrength = 0.2f;
		float fresnelBias = 0.1f;
		float fresnelPower = 8.3f;
		float fresnelDistantScale = 18.3f;
		float causticsPower = 1.7f;
		float causticsLength = 2.0f;
		float causticsDepthFallOff = 0.25f;
		float depthFar = 444.0f;
		float depthCutOff = 0.5f;
		float normalMap0Scale = 0.05f;
		float normalMap1Scale = 0.016f;
		float pad[1];
	} pmc;

	float specular = 46.0f;
	float skyBoxWeight = 27.0f;
	float refractionDistortionStrength = 23.4f;
	float reflectionDistortionStrength = 20.4f;
	float causticsDepthFallOff = 3405.0f;

	float fresnelBias = 25.9f;
	float depthCutOff = 20.5f;
	float normalMap0Scale = 25.1f;
	float normalMap1Scale = 28.9f;
	DirectX::XMFLOAT2 normalMap0scrollSpeed = { 0.1f, -0.6f };
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
