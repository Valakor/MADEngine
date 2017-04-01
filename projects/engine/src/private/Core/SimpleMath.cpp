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

	eastl::string ULinearTransform::ToString() const
	{
		eastl::string resultTransformString;

		resultTransformString.sprintf("s: %f, r: <%f, %f, %f, %f>, t: <%f, %f, %f>", m_scale,
			m_rotation.x, m_rotation.y, m_rotation.z, m_rotation.w,
			m_translation.x, m_translation.y, m_translation.z);

		return resultTransformString;
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

	bool ULinearTransform::operator!=(const ULinearTransform& inOtherTransform) const
	{
		return !(*this == inOtherTransform);
	}

	bool ULinearTransform::operator==(const ULinearTransform& inOtherTransform) const
	{
		return (FloatEqual(m_scale, inOtherTransform.m_scale)) && (m_rotation == inOtherTransform.m_rotation) && (m_translation == inOtherTransform.m_translation);
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

	float ClampAxis(float inAngle)
	{
		// returns Angle in the range (-360,360)
		inAngle = fmod(inAngle, 360.f);

		if (inAngle < 0.f)
		{
			// shift to [0,360) range
			inAngle += 360.f;
		}

		return inAngle;
	}

	float NormalizeAxis(float inAngle)
	{
		// returns Angle in the range [0,360)
		inAngle = ClampAxis(inAngle);

		if (inAngle > 180.f)
		{
			// shift to (-180,180]
			inAngle -= 360.f;
		}

		return inAngle;
	}

	// Assumes angles are in radians already
	Quaternion FromEulerAngles(float inPitch, float inYaw, float inRoll)
	{
		// Implementation taken from: http://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToQuaternion/
		// Heading = inYaw
		// Attitude = inRoll
		// Bank = inPitch

		Quaternion outputQuat;

		float c1 = cos(inYaw / 2.0f);
		float s1 = sin(inYaw / 2.0f);
		float c2 = cos(inRoll / 2.0f);
		float s2 = sin(inRoll / 2.0f);
		float c3 = cos(inPitch / 2.0f);
		float s3 = sin(inPitch / 2.0f);
		float c1c2 = c1*c2;
		float s1s2 = s1*s2;

		outputQuat.w = c1c2*c3 - s1s2*s3;
		outputQuat.x = c1c2*s3 + s1s2*c3;
		outputQuat.y = s1*c2*c3 + c1*s2*s3;
		outputQuat.z = c1*s2*c3 - s1*c2*s3;

		return outputQuat;
	}

	void GetEulerAngles(const Quaternion& inQ, float& outPitch, float& outYaw, float& outRoll)
	{
		const float SingularityTest = inQ.x * inQ.y + inQ.w * inQ.z;
		const float YawY = 2.0f * (inQ.w * inQ.y + inQ.z * inQ.x);
		const float YawX = (1.0f - 2.0f * (pow(inQ.z, 2.0f) + pow(inQ.x, 2.0f)));

		const float SINGULARITY_THRESHOLD = 0.4999995f;
		const float RAD_TO_DEG = (180.f) / DirectX::XM_PI;

		if (SingularityTest > SINGULARITY_THRESHOLD) // North pole singularity
		{
			outYaw = 2.0f * atan2f(inQ.x, inQ.w) * RAD_TO_DEG;
			outRoll = 90.f;
			outPitch = 0.0f;
		}
		else if (SingularityTest < -SINGULARITY_THRESHOLD) // South pole singularity
		{
			outYaw = -2.0f * atan2f(inQ.x, inQ.w) * RAD_TO_DEG;
			outRoll = -90.0f;
			outPitch = 0.0f;
		}
		else
		{
			double sqx = inQ.x * inQ.x;
			double sqy = inQ.y * inQ.y;
			double sqz = inQ.z * inQ.z;

			outYaw = atan2f(2.0f * inQ.y * inQ.w - 2 * inQ.x * inQ.z, 1.0f - 2.0f * sqy - 2.0f * sqz) * RAD_TO_DEG;
			outRoll = asinf(2.0f * SingularityTest) * RAD_TO_DEG;
			outPitch = atan2f(2.0f * inQ.x * inQ.w - 2.0f * inQ.y * inQ.z, 1.0f - 2.0f * sqx - 2.0f * sqz) * RAD_TO_DEG;
		}
	}

}
