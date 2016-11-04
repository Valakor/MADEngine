#include "Core\SimpleMath.h"

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

	void ULinearTransform::operator*=(const ULinearTransform& inOtherTransform)
	{
		m_scale *= inOtherTransform.m_scale;
		m_rotation *= inOtherTransform.m_rotation;
		m_translation += inOtherTransform.m_translation;

		UpdateCachedTransform();
	}

	ULinearTransform operator*(const ULinearTransform& inLeftTransform, const ULinearTransform& inRightTransform)
	{
		ULinearTransform outputTransform;

		outputTransform.m_scale = inLeftTransform.m_scale * inRightTransform.m_scale;
		outputTransform.m_rotation = inLeftTransform.m_rotation * inRightTransform.m_rotation;
		outputTransform.m_translation = inLeftTransform.m_translation + inRightTransform.m_translation;

		outputTransform.UpdateCachedTransform();

		return outputTransform;
	}
}