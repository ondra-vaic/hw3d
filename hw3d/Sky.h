#pragma once
#include "Drawable.h"

class Sky : public Drawable
{
public:
	Sky(Graphics& gfx, float size, DirectX::XMFLOAT4 lowColor, DirectX::XMFLOAT4 highColor);
	void SpawnControlWindow(Graphics& gfx, const std::string& name) noexcept;

	DirectX::XMMATRIX GetTransformXM() const noexcept override;

private:

	struct SkyPSMaterialConstant
	{
		DirectX::XMFLOAT4 lowColor;
		DirectX::XMFLOAT4 highColor;
		alignas(16)DirectX::XMFLOAT3 cameraPosition;
	} pmc;
};

