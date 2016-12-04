#pragma once

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#include <d3d11_2.h>
__pragma(warning(push))
__pragma(warning(disable:4838))
#include <DirectXTK/SimpleMath.h>
__pragma(warning(pop))

#include <cmath>

namespace MAD
{
	using namespace DirectX::SimpleMath;

	inline constexpr float ConvertToRadians(float inDegrees)
	{
		return inDegrees * (DirectX::XM_PI / 180.0f);
	}

	inline constexpr float ConvertToDegrees(float inRadians)
	{
		return inRadians * (180.0f / DirectX::XM_PI);
	}

	inline bool FloatEqual(float a, float b, float epsilon = 0.0001f)
	{
		return fabs(a - b) <= epsilon;
	}

	class ULinearTransform
	{
	public:
		ULinearTransform();

		ULinearTransform(float inScale, const Quaternion& inRotation, const Vector3& inTranslation);

		float GetScale() const { return m_scale; }
		const Quaternion& GetRotation() const { return m_rotation; }
		const Vector3& GetTranslation() const { return m_translation; }
		const Matrix& GetMatrix() const { return m_cachedTransform; }
		
		Vector3 GetForward() const { Vector3 forward = m_cachedTransform.Forward(); forward.Normalize(); return forward; }
		Vector3 GetRight() const { Vector3 right = m_cachedTransform.Right(); right.Normalize(); return right; }
		Vector3 GetUp() const { Vector3 up = m_cachedTransform.Up(); up.Normalize(); return up; }

		void SetScale(float inScale);
		void SetRotation(const Quaternion& inRotation);
		void SetTranslation(const Vector3& inTranslation);

		friend ULinearTransform TransformRelative(const ULinearTransform& inRelativeTransform, const ULinearTransform& inParentTransform); // Transform is in this transforms local space
		friend ULinearTransform TransformAbsolute(const ULinearTransform& inAbsoluteTransformA, const ULinearTransform& inAbsoluteTransformB); // Transform is in the same space
	private:
		void UpdateCachedTransform();
	private:
		float m_scale;
		Quaternion m_rotation;
		Vector3 m_translation;

		Matrix m_cachedTransform;
	};
}