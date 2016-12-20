#include "Core/SimpleMath.h"

namespace MAD
{
	ULinearTransform::ULinearTransform() : m_scale(1.0f) {}

	ULinearTransform::ULinearTransform(float inScale, const Quaternion& inRotation, const Vector3& inTranslation)
		: m_scale(inScale)
		, m_rotation(inRotation)
		, m_translation(inTranslation)
	{
		UpdateCachedTransform();
	}

	void ULinearTransform::SetScale(float inScale)
	{
		m_scale = inScale;
		UpdateCachedTransform();
	}

	void ULinearTransform::SetRotation(const Quaternion& inRotation)
	{
		m_rotation = inRotation;
		UpdateCachedTransform();
	}

	void ULinearTransform::SetTranslation(const Vector3& inTranslation)
	{
		m_translation = inTranslation;
		UpdateCachedTransform();
	}

	void ULinearTransform::UpdateCachedTransform()
	{
		m_cachedTransform = Matrix::CreateScale(m_scale) * Matrix::CreateFromQuaternion(m_rotation) * Matrix::CreateTranslation(m_translation);
	}

	ULinearTransform ULinearTransform::Lerp(const ULinearTransform& a, const ULinearTransform& b, float t)
	{
		ULinearTransform ret;

		ret.m_scale = ::MAD::Lerp(a.m_scale, b.m_scale, t);
		Vector3::Lerp(a.m_translation, b.m_translation, t, ret.m_translation);
		Quaternion::Lerp(a.m_rotation, b.m_rotation, t, ret.m_rotation);

		ret.UpdateCachedTransform();
		return ret;
	}

	ULinearTransform ULinearTransform::TransformRelative(const ULinearTransform& inRelativeTransform, const ULinearTransform& inParentTransform)
	{
		ULinearTransform resultTransform;

		resultTransform.m_scale = inRelativeTransform.m_scale * inParentTransform.m_scale;
		resultTransform.m_rotation = inRelativeTransform.m_rotation * inParentTransform.m_rotation;

		// Since the rotation is relative to the parent, we need to transform it into the space of the parent transform, before translating the parent by the relative translation
		Vector3 t = Vector3::Transform(inRelativeTransform.m_translation, inParentTransform.m_rotation);
		resultTransform.m_translation = t + inParentTransform.m_translation;

		resultTransform.UpdateCachedTransform();

		return resultTransform;
	}

	ULinearTransform ULinearTransform::TransformAbsolute(const ULinearTransform& inAbsoluteTransformA, const ULinearTransform& inAbsoluteTransformB)
	{
		ULinearTransform resultTransform;

		resultTransform.m_scale = inAbsoluteTransformA.m_scale * inAbsoluteTransformB.m_scale;
		resultTransform.m_rotation = inAbsoluteTransformA.m_rotation * inAbsoluteTransformB.m_rotation;
		resultTransform.m_translation = inAbsoluteTransformA.m_translation + inAbsoluteTransformB.m_translation;

		resultTransform.UpdateCachedTransform();

		return resultTransform;
	}
}