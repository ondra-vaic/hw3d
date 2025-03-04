#include "Camera.h"
#include <algorithm>
#include "imgui/imgui.h"
#include "ChiliMath.h"

namespace dx = DirectX;

Camera::Camera() noexcept
{
	Reset();
}

DirectX::XMMATRIX Camera::GetMatrix() const noexcept
{
	using namespace dx;

	const dx::XMVECTOR forwardBaseVector = XMVectorSet( 0.0f,0.0f,1.0f,0.0f );
	// apply the cameraTransform rotations to a base vector
	auto lookVector = XMVector3Transform( forwardBaseVector,
		XMMatrixRotationRollPitchYaw( pitch,yaw,0.0f )
	);

	// generate cameraTransform transform (applied to all objects to arrange them relative
	// to cameraTransform position/orientation in world) from cam position and direction
	// cameraTransform "top" always faces towards +Y (cannot do a barrel roll)
	auto camPosition = XMLoadFloat3( &pos );

	if (isReflected)
	{
		lookVector = XMVectorSetY(lookVector, XMVectorGetY(lookVector) * -1);
		camPosition = XMVectorSetY(camPosition, reflectionPlaneY * 2 - XMVectorGetY(camPosition));
	}
	
	const auto camTarget = camPosition + lookVector;
	return XMMatrixLookAtLH(camPosition, camTarget, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
}

void Camera::SpawnControlWindow() noexcept
{
	if( ImGui::Begin( "Camera" ) )
	{
		ImGui::Text( "Position" );
		ImGui::SliderFloat( "X",&pos.x,-100.0f, 100.0f,"%.1f" );
		ImGui::SliderFloat( "Y",&pos.y,-100.0f, 100.0f,"%.1f" );
		ImGui::SliderFloat( "Z",&pos.z,-100.0f, 100.0f,"%.1f" );
		ImGui::Text( "Orientation" );
		ImGui::SliderAngle( "Pitch",&pitch,0.995f * -90.0f,0.995f * 90.0f );
		ImGui::SliderAngle( "Yaw",&yaw,-180.0f,180.0f );
		if( ImGui::Button( "Reset" ) )
		{
			Reset();
		}
	}
	ImGui::End();
}

void Camera::Reset() noexcept
{
	pos = { 0, 18.5f, -43.9f};
	pitch = 0.24f;
	yaw = 0.0f;
}

void Camera::Rotate( float dx,float dy ) noexcept
{
	yaw = wrap_angle( yaw + dx * rotationSpeed );
	pitch = std::clamp( pitch + dy * rotationSpeed,0.995f * -PI / 2.0f,0.995f * PI / 2.0f );
}

void Camera::Translate( DirectX::XMFLOAT3 translation ) noexcept
{
	dx::XMStoreFloat3( &translation,dx::XMVector3Transform(
		dx::XMLoadFloat3( &translation ),
		dx::XMMatrixRotationRollPitchYaw( pitch,yaw,0.0f ) *
		dx::XMMatrixScaling( travelSpeed,travelSpeed,travelSpeed )
	) );
	pos = {
		pos.x + translation.x,
		pos.y + translation.y,
		pos.z + translation.z
	};
}

void Camera::SetIsReflected(bool isReflected) noexcept
{
	this->isReflected = isReflected;
}

void Camera::SetReflectionPlaneY(float reflectionPlaneY) noexcept
{
	this->reflectionPlaneY = reflectionPlaneY;
}

DirectX::XMFLOAT3 Camera::GetPos() const noexcept
{
	return pos;
}
