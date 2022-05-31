#include "LightData.h"
#include "imgui/imgui.h"

LightData::LightData( Graphics& gfx,float radius )
	:
	mesh( gfx,radius ),
	cbuf( gfx )
{
	Reset();
}

void LightData::SpawnControlWindow() noexcept
{
	if( ImGui::Begin( "Light" ) )
	{
		ImGui::Text(" Directional ");

		ImGui::SliderFloat("DirX", &cbData.directional.direction.x, -1.0f, 1.0f, "%.1f");
		ImGui::SliderFloat("DirY", &cbData.directional.direction.y, -1.0f, 1.0f, "%.1f");
		ImGui::SliderFloat("DirZ", &cbData.directional.direction.z, -1.0f, 1.0f, "%.1f");

		ImGui::ColorEdit3("Dir Diffuse Color", &cbData.directional.diffuseColor.x);
		ImGui::SliderFloat("Dir Diffuse Intensity", &cbData.directional.diffuseIntensity, 0.0f, 5.0f, "%.1f");

		ImGui::ColorEdit3("Ambient", &cbData.directional.ambient.x);

		ImGui::Text( "Position" );
		ImGui::SliderFloat( "PosX",&cbData.point.pos.x,-60.0f,60.0f,"%.1f" );
		ImGui::SliderFloat( "PosY",&cbData.point.pos.y,-60.0f,60.0f,"%.1f" );
		ImGui::SliderFloat( "PosZ",&cbData.point.pos.z,-60.0f,60.0f,"%.1f" );
		
		ImGui::Text( "Intensity/Color" );
		ImGui::SliderFloat( "Pos Intensity",&cbData.point.diffuseIntensity,0.01f,2.0f,"%.2f",2 );
		ImGui::ColorEdit3( "Pos Diffuse Color",&cbData.point.diffuseColor.x );
	
		ImGui::Text( "Falloff" );
		ImGui::SliderFloat( "Constant",&cbData.point.attConst,0.05f,10.0f,"%.2f",4 );
		ImGui::SliderFloat( "Linear",&cbData.point.attLin,0.0001f,4.0f,"%.4f",8 );
		ImGui::SliderFloat( "Quadratic",&cbData.point.attQuad,0.0000001f,10.0f,"%.7f",10 );
		
		if( ImGui::Button( "Reset" ) )
		{
			Reset();
		}
	}
	ImGui::End();
}

void LightData::Reset() noexcept
{
	cbData = {
		{
			{ 0.0f,20.0f,25.0f },
			{ 0.8f,0.8f,0.8f },
			0.5f,
			0.75f,
			0.0001f,
			0.00000015f,
		},
		{
			{0.7f, 0.3f, -0.3f},
			{ 0.012f,0.02f,0.05f },
			{ 0.8f,0.8f,0.8f },
			1.5f
		}
	};
}

void LightData::Draw( Graphics& gfx ) const noxnd
{
	mesh.SetPos( cbData.point.pos ); 
	mesh.Draw( gfx );
}

void LightData::Bind( Graphics& gfx,DirectX::FXMMATRIX view ) const noexcept
{
	auto dataCopy = cbData;
	const auto pos = DirectX::XMLoadFloat3( &cbData.point.pos );
	DirectX::XMStoreFloat3( &dataCopy.point.pos, pos);
	cbuf.Update( gfx,dataCopy );
	cbuf.Bind( gfx );
}
