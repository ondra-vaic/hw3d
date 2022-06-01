#include "TestCube.h"
#include "Cube.h"
#include "BindableCommon.h"
#include "TransformCbufDoubleboi.h"
#include "imgui/imgui.h"
#include "Stencil.h"

TestCube::TestCube( Graphics& gfx,float size )
{
	using namespace Bind;
	namespace dx = DirectX;

	auto model = Cube::MakeIndependentTextured();
	model.Transform( dx::XMMatrixScaling( size,size,size ) );
	model.SetNormalsIndependentFlat();
	const auto geometryTag = "$cube." + std::to_string( size );
	AddBind( VertexBuffer::Resolve( gfx,geometryTag,model.vertices ) );
	AddBind( IndexBuffer::Resolve( gfx,geometryTag,model.indices ) );

	AddBind(Texture::Resolve(gfx, "Images\\brickwall_normal_obj.png"));
	AddBind(Texture::Resolve(gfx, "Images\\brickwall_normal.jpg", 1));
	AddBind( Sampler::Resolve( gfx ) );

	auto pvs = VertexShader::Resolve( gfx,"PhongVSNormalMap.cso" );
	auto pvsbc = pvs->GetBytecode();
	AddBind( std::move( pvs ) );

	defaultShader = PixelShader::Resolve(gfx, "PhongPSNormalMap.cso");
	depthShader = PixelShader::Resolve(gfx, "DefaultDepthPS.cso");
	currentShader = defaultShader;

	AddBind( PixelConstantBuffer<PSMaterialConstant>::Resolve( gfx,pmc,1u ) );

	AddBind( InputLayout::Resolve( gfx,model.vertices.GetLayout(),pvsbc ) );

	AddBind( Topology::Resolve( gfx,D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

	auto tcbdb = std::make_shared<TransformCbufDoubleboi>( gfx,*this,0u,2u );
	AddBind( tcbdb );
}

void TestCube::SetPos( DirectX::XMFLOAT3 pos ) noexcept
{
	this->pos = pos;
}

void TestCube::SetRotation( float roll,float pitch,float yaw ) noexcept
{
	this->roll = roll;
	this->pitch = pitch;
	this->yaw = yaw;
}

DirectX::XMMATRIX TestCube::GetTransformXM() const noexcept
{
	auto xf = DirectX::XMMatrixRotationRollPitchYaw( roll,pitch,yaw ) *
		DirectX::XMMatrixTranslation( pos.x,pos.y,pos.z );
	return xf;
}

void TestCube::SpawnControlWindow( Graphics& gfx,const char* name ) noexcept
{
	if( ImGui::Begin( name ) )
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
		bool changed0 = ImGui::SliderFloat( "Spec. Int.",&pmc.specularIntensity,0.0f,1.0f );
		bool changed1 = ImGui::SliderFloat( "Spec. Power",&pmc.specularPower,0.0f,100.0f );
		bool checkState = pmc.normalMappingEnabled == TRUE;
		bool changed2 = ImGui::Checkbox( "Enable Normal Map",&checkState );
		pmc.normalMappingEnabled = checkState ? TRUE : FALSE;
		if( changed0 || changed1 || changed2 )
		{
			QueryBindable<Bind::PixelConstantBuffer<PSMaterialConstant>>()->Update( gfx,pmc );
		}
	}
	ImGui::End();
}
