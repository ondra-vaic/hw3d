#pragma once
#include "Graphics.h"
#include <DirectXMath.h>
#include "ConditionalNoexcept.h"
#include <memory>

namespace Bind
{
	class Bindable;
	class IndexBuffer;
}

class Drawable
{
public:
	Drawable() = default;
	Drawable( const Drawable& ) = delete;
	virtual DirectX::XMMATRIX GetTransformXM() const noexcept = 0;
	virtual void Draw( Graphics& gfx ) const noxnd;
	virtual ~Drawable() = default;
	template<class T>
	T* QueryBindable() noexcept
	{
		for( auto& pb : binds )
		{
			if( auto pt = dynamic_cast<T*>(pb.get()) )
			{
				return pt;
			}
		}
		return nullptr;
	}

	void SetRenderDepth(bool renderDepth);

protected:
	void AddBind( std::shared_ptr<Bind::Bindable> bind ) noxnd;

	const Bind::IndexBuffer* pIndexBuffer = nullptr;
	std::vector<std::shared_ptr<Bind::Bindable>> binds;

	std::shared_ptr<Bind::Bindable> currentShader;
	std::shared_ptr<Bind::Bindable> defaultShader;
	std::shared_ptr<Bind::Bindable> depthShader;
};