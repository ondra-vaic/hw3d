#pragma once
#include "Drawable.h"

namespace Bind
{
	class Blender;
}

class RenderTexture;

class Water : public Drawable
{
public:
	Water(Graphics& gfx, float size, DirectX::XMFLOAT4 color = { 1.0f,1.0f,1.0f,0.0f });
	void SetPos(DirectX::XMFLOAT3 pos) noexcept;
	void SetRotation(float roll, float pitch, float yaw) noexcept;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	void SpawnControlWindow(Graphics& gfx, const std::string& name) noexcept;
	void SetWorldTexture(ID3D11ShaderResourceView* worldTextureResource);
	void Draw(Graphics& gfx) const noxnd;

	void SetDrawMask(bool drawMask);

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
	bool drawMask;

	ID3D11ShaderResourceView* worldTextureResource;
	std::shared_ptr<Bind::Bindable> waterShader;
	std::shared_ptr<Bind::Bindable> maskShader;
	std::shared_ptr<Bind::Blender> blender;
};
