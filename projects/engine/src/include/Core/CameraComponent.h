#pragma once

#include "Core/Component.h"
#include "Core/SimpleMath.h"
#include "Rendering/CameraInstance.h"


namespace MAD
{
	class CCameraComponent : public UComponent
	{
		MAD_DECLARE_PRIORITIZED_COMPONENT(CCameraComponent, UComponent, EPriorityLevelReference::EPriorityLevel_Physics + 1)
	public:
		explicit CCameraComponent(OGameWorld* inOwningWorld);
		virtual ~CCameraComponent();

		virtual void Load(const UGameWorldLoader& inLoader) override;
		virtual void UpdateComponent(float inDeltaTime) override;

	private:
		void MoveRight(float inVal);
		void MoveForward(float inVal);
		void MoveUp(float inVal);

		void LookRight(float inVal);
		void LookUp(float inVal);

		void OnMouseRightClickDown();
		void OnMouseRightClickUp();

		void OnReset();

		SCameraInstance m_cameraInstance;

		Vector3 m_cameraPos;
		Quaternion m_cameraRot;

		Vector3 m_cameraPosInitial;
		Quaternion m_cameraRotInitial;

		float m_cameraMoveSpeed;
		float m_cameraLookSpeed;

		bool m_mouseRightClickDown;
	};
}