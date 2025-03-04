#pragma once
#include <DirectXMath.h>

class Camera
{
public:
	Camera() noexcept;
	DirectX::XMMATRIX GetMatrix() const noexcept;
	void SpawnControlWindow() noexcept;
	void Reset() noexcept;
	void Rotate( float dx,float dy ) noexcept;
	void Translate( DirectX::XMFLOAT3 translation ) noexcept;
	void SetIsReflected(bool isReflected) noexcept;
	void SetReflectionPlaneY(float reflectionPlaneY) noexcept;
	DirectX::XMFLOAT3 GetPos() const noexcept;
private:
	DirectX::XMFLOAT3 pos;
	float pitch;
	float yaw;
	bool isReflected;
	float reflectionPlaneY;
	static constexpr float travelSpeed = 12.0f;
	static constexpr float rotationSpeed = 0.004f;
};
