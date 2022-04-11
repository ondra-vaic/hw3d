#pragma once
#include "Drawable.h"

class Water : public Drawable
{
public:
	Water(Graphics& gfx, float size, DirectX::XMFLOAT4 color = { 1.0f,1.0f,1.0f,0.0f });
	void SetPos(DirectX::XMFLOAT3 pos) noexcept;
	void SetRotation(float roll, float pitch, float yaw) noexcept;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	void SpawnControlWindow(Graphics& gfx, const std::string& name) noexcept;
private:
	struct PSMaterialConstant
	{
		DirectX::XMFLOAT4 color;
	} pmc;
	struct VSMaterialConstant
	{
		DirectX::XMFLOAT4 color;
	} vmc;
	DirectX::XMFLOAT3 pos = { 0.0f,0.0f,0.0f };
	float roll = 0.0f;
	float pitch = 0.0f;
	float yaw = 0.0f;
};