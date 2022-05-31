#pragma once
#include "Drawable.h"

class TestPlane : public Drawable
{
public:
	TestPlane( Graphics& gfx,float size);
	void SetPos( DirectX::XMFLOAT3 pos ) noexcept;
	void SetRotation( float roll,float pitch,float yaw ) noexcept;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	void SpawnControlWindow( Graphics& gfx,const std::string& name ) noexcept;
private:
	struct PSMaterialConstant
	{
		float specularIntensity;
		float specularPower;
		float padding[2];
	} pmc;

	DirectX::XMFLOAT3 pos = { 0.0f,0.0f,0.0f };
	float roll = 0.0f;
	float pitch = 0.0f;
	float yaw = 0.0f;
};