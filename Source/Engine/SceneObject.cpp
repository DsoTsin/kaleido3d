#include "Kaleido3D.h"
#include "SceneObject.h"

namespace k3d {

	SObject::~SObject() {

	}

	void SObject::SetNodeType(int) {

	}

	void SObject::SetVisible(bool v) {
		this->m_Visible = v;
	}

	bool SObject::IsVisible()
	{
		return m_Visible;
	}

	const kMath::AABB & SObject::GetBoundingBox() const
	{
		return m_BoundingBox;
	}
}