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
#include <EASTL/string.h>

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

	inline constexpr float Clamp(float x, float min, float max)
	{
		return (x < min) ? min : (x > max) ? max : x;
	}

	inline constexpr float Saturate(float x)
	{
		return Clamp(x, 0.0f, 1.0f);
	}

	/*inline float Lerp(float a, float b, float t)
	{
		return a + (b - a) * Saturate(t);
	}*/

	template <typename T>
	inline T Lerp(const T& inA, const T& inB, float inT)
	{
		return inA + (inB - inA) * Saturate(inT);
	}

	float ClampAxis(float inAngle);
	float NormalizeAxis(float inAngle);
	Quaternion FromEulerAngles(float inPitch, float inYaw, float inRoll);
	void GetEulerAngles(const Quaternion& inQ, float& outPitch, float& outYaw, float& outRoll);

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
		eastl::string ToString() const;

		void SetScale(float inScale);
		void SetRotation(const Quaternion& inRotation);
		void SetTranslation(const Vector3& inTranslation);

		bool operator!=(const ULinearTransform& inOtherTransform) const;
		bool operator==(const ULinearTransform& inOtherTransform) const;


		static ULinearTransform Lerp(const ULinearTransform& a, const ULinearTransform& b, float t); // Both transforms must be in the same space

		/*
			inRelativeTransform - Relative (local) transform to the transform space of inParentTransform
			inParentTransform - Parent transform of inRelativeTransform

			Calculates the transform that results from transforming inParentTransform by inRelativeTransform
		*/
		static ULinearTransform TransformRelative(const ULinearTransform& inRelativeTransform, const ULinearTransform& inParentTransform);

		/*
			inAbsoluteTransformA - First absolute transform
			inAbsoluteTransformB - Second absolute transform

			**** WARNING ****
			Assumes that that inAbsoluteTransformA and inAbsoluteTransformB are in the same transform space

			Calculates the transform that results from transforming inAbsoluteTransformA by inAbsoluteTransformB
		*/
		static ULinearTransform TransformAbsolute(const ULinearTransform& inAbsoluteTransformA, const ULinearTransform& inAbsoluteTransformB);
		static ULinearTransform CreateScale(float inScale);
	private:
		void UpdateCachedTransform();
	private:
		float m_scale;
		Quaternion m_rotation;
		Vector3 m_translation;

		Matrix m_cachedTransform;
	};
}