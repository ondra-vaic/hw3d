#include "TestPlane.h"
#include "Plane.h"
#include "BindableCommon.h"
#include "imgui/imgui.h"
#include "TransformCbufDoubleboi.h"

TestPlane::TestPlane( Graphics& gfx,float size)
	:
	pmc( { 0.1f, 5, 0, 0 } )
{
	using namespace Bind;
	namespace dx = DirectX;

	auto model = Plane::Make(10, size / 10.0f);
	model.Transform( dx::XMMatrixScaling( size,size,1.0f ) );
	const auto geometryTag = "$plane." + std::to_string( size );
	AddBind( VertexBuffer::Resolve( gfx,geometryTag,model.vertices ) );
	AddBind( IndexBuffer::Resolve( gfx,geometryTag,model.indices ) );

	auto pvs = VertexShader::Resolve( gfx,"PhongVSNormalMap.cso" );
	auto pvsbc = pvs->GetBytecode();
	AddBind( std::move( pvs ) );

	AddBind(Texture::Resolve(gfx, "Images\\angled-tiled-floor_albedo.png"));
	AddBind(Texture::Resolve(gfx, "Images\\angled-tiled-floor_normal-ogl.png", 1));

	defaultShader = PixelShader::Resolve(gfx, "PhongPSNormalMap.cso");
	depthShader = PixelShader::Resolve(gfx, "DefaultDepthPS.cso");
	currentShader = defaultShader;

	AddBind( std::make_shared<PixelConstantBuffer<PSMaterialConstant>>( gfx,pmc,1u ) );

	AddBind(std::make_shared<InputLayout>( gfx,model.vertices.GetLayout(),pvsbc ) );

	AddBind( Topology::Resolve( gfx,D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

	AddBind(Sampler::Resolve(gfx));

	AddBind( std::make_shared<TransformCbuf>( gfx,*this,0u ) );

	AddBind(std::make_shared<Blender>( gfx,true,1.0f ) );
	
	AddBind( Rasterizer::Resolve( gfx,false ) );
}

void TestPlane::SetPos( DirectX::XMFLOAT3 pos ) noexcept
{
	this->pos = pos;
}

void TestPlane::SetRotation( float roll,float pitch,float yaw ) noexcept
{
	this->roll = roll;
	this->pitch = pitch;
	this->yaw = yaw;
}

DirectX::XMMATRIX TestPlane::GetTransformXM() const noexcept
{
	return DirectX::XMMatrixRotationRollPitchYaw( roll,pitch,yaw ) *
		DirectX::XMMatrixTranslation( pos.x,pos.y,pos.z );
}

void TestPlane::SpawnControlWindow( Graphics& gfx,const std::string& name ) noexcept
{
	if( ImGui::Begin( name.c_str() ) )
	{
		ImGui::Text( "Position" );
		ImGui::SliderFloat( "X",&pos.x,-80.0f,80.0f,"%.1f" );
		ImGui::SliderFloat( "Y",&pos.y,-80.0f,80.0f,"%.1f" );
		ImGui::SliderFloat( "Z",&pos.z,-80.0f,80.0f,"%.1f" );
		ImGui::Text( "Orientation" );
		ImGui::SliderAngle( "Roll",&roll,-180.0f,180.0f );
		ImGui::SliderAngle( "Pitch",&pitch,-180.0f,180.0f );
		ImGui::SliderAngle( "Yaw",&yaw,-180.0f,180.0f );
		ImGui::Text( "Shading" );
		auto pBlender = QueryBindable<Bind::Blender>();
		float factor = pBlender->GetFactor();
		ImGui::SliderFloat( "Translucency",&factor,0.0f,1.0f );
		pBlender->SetFactor( factor );
	}
	ImGui::End();
}
