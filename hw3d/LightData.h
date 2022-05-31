#pragma once
#include "Graphics.h"
#include "SolidSphere.h"
#include "ConstantBuffers.h"
#include "ConditionalNoexcept.h"

class LightData
{
public:
	LightData(Graphics& gfx, float radius = 0.5f);
	void SpawnControlWindow() noexcept;
	void Reset() noexcept;
	void Draw(Graphics& gfx) const noxnd;
	void Bind(Graphics& gfx, DirectX::FXMMATRIX view) const noexcept;
private:
	struct LightDataCBuf
	{
		struct PointLight
		{
			alignas(16) DirectX::XMFLOAT3 pos;
			alignas(16) DirectX::XMFLOAT3 diffuseColor;
			float diffuseIntensity;
			float attConst;
			float attLin;
			float attQuad;
		}point;
		struct DirectionalLight
		{	
			alignas(16) DirectX::XMFLOAT3 direction;
			alignas(16) DirectX::XMFLOAT3 ambient;
			alignas(16) DirectX::XMFLOAT3 diffuseColor;

			float diffuseIntensity;
		}directional;
	};
private:
	LightDataCBuf cbData;
	mutable SolidSphere mesh;
	mutable Bind::PixelConstantBuffer<LightDataCBuf> cbuf;
};