#include "Core/GameInput.h"

#include "Core/GameEngine.h"
#include "Core/GameWindow.h"
#include "Misc/Logging.h"
#include "Misc/Remotery.h"

using eastl::string;

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogGameInput);

	SControlScheme& SControlScheme::RegisterEvent(const string& inEventName, key_t inKey)
	{
		if (StringToKeyBindings.count(inEventName) == 0)
		{
			KeyToActionBindings[inKey].push_back(SActionBinding(inEventName, inKey));
			StringToKeyBindings[inEventName] = inKey;
		}
		else
		{
			LOG(LogGameInput, Warning, "Cannot register multiple events or axis of the same name to a Control Scheme.\n");
		}

		return *this;
	}

	SControlScheme& SControlScheme::RegisterAxis(const string& inAxisName, key_t inPosKey, key_t inNegKey)
	{
		if (StringToKeyBindings.count(inAxisName) == 0)
		{
			auto newBinding = eastl::make_shared<SAxisBinding>(inAxisName, inPosKey, inNegKey);
			KeyToAxisBindings[inPosKey].push_back(newBinding);
			KeyToAxisBindings[inNegKey].push_back(newBinding);
			StringToKeyBindings[inAxisName] = inPosKey;
		}
		else
		{
			LOG(LogGameInput, Warning, "Cannot register multiple events or axis of the same name to a Control Scheme.\n");
		}

		return *this;
	}

	SControlScheme& SControlScheme::RegisterAxis(const string& inAxisName, EInputAxis inAxisType)
	{
		if (StringToKeyBindings.count(inAxisName) == 0)
		{
			key_t key = static_cast<key_t>(inAxisType);
			auto newBinding = eastl::make_shared<SAxisBinding>(inAxisName, inAxisType);
			KeyToAxisBindings[key].push_back(newBinding);
			StringToKeyBindings[inAxisName] = key;
		}
		else
		{
			LOG(LogGameInput, Warning, "Cannot register multiple events or axis of the same name to a Control Scheme.\n");
		}

		return *this;
	}

	SControlScheme& SControlScheme::Finalize(bool bMakeActive) const
	{
		auto retScheme = UGameInput::Get().AddControlScheme(*this, bMakeActive);
		MAD_ASSERT_DESC(retScheme != nullptr, "Failed to finalize control scheme, probably because it was added twice.");
		return *retScheme;
	}

	void SControlScheme::ClearAxis()
	{
		for (const auto& keyBindingPair : KeyToAxisBindings)
		{
			for (auto& binding : keyBindingPair.second)
			{
				binding->Value = 0.0f;
			}
		}
	}

	bool SControlScheme::BindEvent(const string& inEventName, EInputEvent inType, InputEventDelegate inCallback)
	{
		auto keyIter = StringToKeyBindings.find(inEventName);
		if (keyIter != StringToKeyBindings.end())
		{
			key_t key = keyIter->second;
			for (auto& binding : KeyToActionBindings[key])
			{
				if (binding.Action == inEventName)
				{
					switch (inType)
					{
					case EInputEvent::IE_KeyDown:
						binding.OnPressed.BindDelegate(inCallback);
						break;
					case EInputEvent::IE_KeyUp:
						binding.OnReleased.BindDelegate(inCallback);
						break;
					case EInputEvent::IE_KeyHeld:
						binding.OnHeld.BindDelegate(inCallback);
						break;
					default:
						LOG(LogGameInput, Warning, "Control Scheme [%s]: Input Binding failed: Unknown EInputEvent\n", mName.c_str());
						return false;
					}
					return true;
				}
			}
		}
		
		LOG(LogGameInput, Warning, "Control Scheme [%s]: Input Binding failed: Unrecognized event `%s`\n", mName.c_str(), inEventName.c_str());
		return false;
	}

	bool SControlScheme::BindAxis(const string& inAxisName, AxisDelegate inCallback)
	{
		auto keyIter = StringToKeyBindings.find(inAxisName);
		if (keyIter != StringToKeyBindings.end())
		{
			key_t key = keyIter->second;
			for (auto& binding : KeyToAxisBindings[key])
			{
				if (binding->Axis == inAxisName)
				{
					binding->OnAxis.BindDelegate(inCallback);
					return true;
				}
			}
		}

		LOG(LogGameInput, Warning, "Control Scheme [%s]: Input Binding failed: Unrecognized axis `%s`\n", mName.c_str(), inAxisName.c_str());
		return false;
	}

	void SControlScheme::UnBindObject(void* inObj)
	{
		if (!inObj)
		{
			LOG(LogGameInput, Warning, "Unbinding null object is meaningless\n");
			return;
		}

		for (auto& keyBindingList : KeyToActionBindings)
		{
			for (auto& keyBinding : keyBindingList.second)
			{
				keyBinding.OnPressed.UnBind(inObj);
				keyBinding.OnHeld.UnBind(inObj);
				keyBinding.OnReleased.UnBind(inObj);
			}
		}

		for (auto& axisBindingList : KeyToAxisBindings)
		{
			for (auto& axisBinding : axisBindingList.second)
			{
				axisBinding->OnAxis.UnBind(inObj);
			}
		}
	}

	bool SControlScheme::OnKeyDown(key_t inKey, bool bInRepeat)
	{
		bool keyEaten = false;

		if (KeyToActionBindings.count(inKey) != 0 && KeyToActionBindings[inKey].size() != 0)
		{
			// Maps to an action
			for (auto& binding : KeyToActionBindings[inKey])
			{
				if (!bInRepeat)
				{
					binding.OnPressed.Execute();
				}
				else
				{
					binding.OnHeld.Execute();
				}
			}
			keyEaten = true;
		}

		if (bInRepeat)
		{
			// Don't send key held actions to axis bindings
			return keyEaten;
		}

		if (KeyToAxisBindings.count(inKey) != 0 && KeyToAxisBindings[inKey].size() != 0)
		{
			// Maps to an axis
			for (auto& binding : KeyToAxisBindings[inKey])
			{
				if (inKey == binding->PosKey)
				{
					binding->Value += 1.0f;
				}
				else
				{
					binding->Value -= 1.0f;
				}
			}
			keyEaten = true;
		}

		return keyEaten;
	}

	bool SControlScheme::OnKeyUp(key_t inKey)
	{
		bool keyEaten = false;

		if (KeyToActionBindings.count(inKey) != 0 && KeyToActionBindings[inKey].size() != 0)
		{
			// Maps to an action
			for (auto& binding : KeyToActionBindings[inKey])
			{
				binding.OnReleased.Execute();
			}
			keyEaten = true;
		}

		if (KeyToAxisBindings.count(inKey) != 0 && KeyToAxisBindings[inKey].size() != 0)
		{
			// Maps to an axis
			for (auto& binding : KeyToAxisBindings[inKey])
			{
				if (inKey == binding->PosKey)
				{
					binding->Value -= 1.0f;
				}
				else
				{
					binding->Value += 1.0f;
				}
			}
			keyEaten = true;
		}

		return keyEaten;
	}

	bool SControlScheme::OnAxis(EInputAxis inAxis, float inValue)
	{
		key_t key = static_cast<key_t>(inAxis);
		if (KeyToAxisBindings.count(key) != 0 && KeyToAxisBindings[key].size() != 0)
		{
			// Maps to an axis
			for (auto& binding : KeyToAxisBindings[key])
			{
				binding->Value = inValue;
			}
			return true;
		}

		return false;
	}

	void SControlScheme::TickAxis()
	{
		for (auto& keyIter : KeyToAxisBindings)
		{
			auto& bindings = keyIter.second;
			for (auto& binding : bindings)
			{
				if (!binding->bTickedThisFrame)
				{
					binding->OnAxis.Execute(binding->Value);
					binding->bTickedThisFrame = true;
				}
			}
		}

		for (auto& keyIter : KeyToAxisBindings)
		{
			auto& bindings = keyIter.second;
			for (auto& binding : bindings)
			{
				binding->bTickedThisFrame = false;
			}
		}
	}

	UGameInput::UGameInput() :
		mMousePosX(0),
		mMousePosY(0),
		mMouseDeltaX(0),
		mMouseDeltaY(0),
		bIgnoreInput(false),
		mMouseMode(EMouseMode::MM_UI)
	{ }

	void UGameInput::Tick()
	{
		rmt_ScopedCPUSample(Input_Tick, 0);

		if (bIgnoreInput)
		{
			return;
		}

		// Detect mouse axis changes
		UpdateMouse();

		// Report all axis changes every frame
		for (auto iter = mControlStack.rbegin(); iter != mControlStack.rend(); ++iter)
		{
			if (iter->bActive)
			{
				iter->TickAxis();
			}
		}
	}

	void UGameInput::UpdateMouse()
	{
		if (bIgnoreInput)
		{
			return;
		}

		static const float MouseAxisMultiplier = 0.07f;

		auto mousePos = gEngine->GetWindow().GetCursorPos();
		auto center = gEngine->GetWindow().GetWindowCenter();
		mMousePosX = mousePos.x;
		mMousePosY = mousePos.y;

		int32_t dX, dY;

		if (mMouseMode == EMouseMode::MM_Game)
		{
			dX = mMouseDeltaX = mMousePosX - center.x;
			dY = mMouseDeltaY = mMousePosY - center.y;
			gEngine->GetWindow().CenterCursor();
		}
		else if (mMouseMode == EMouseMode::MM_UI)
		{
			// Repurpose mMouseDeltaX/Y as a cache for last frame's mouse position
			dX = mMousePosX - mMouseDeltaX;
			dY = mMousePosY - mMouseDeltaY;
			mMouseDeltaX = mMousePosX;
			mMouseDeltaY = mMousePosY;
		}
		else
		{
			MAD_ASSERT_DESC(false, "Unrecognized EInputMode");
			return;
		}

		OnAxis(EInputAxis::IA_MouseX, static_cast<float>(dX) * MouseAxisMultiplier);
		OnAxis(EInputAxis::IA_MouseY, static_cast<float>(dY) * MouseAxisMultiplier);
	}

	void UGameInput::OnChar(char inChar, bool bInRepeat) const
	{
		(void)inChar;
		(void)bInRepeat;

		if (bIgnoreInput)
		{
			return;
		}
	}

	void UGameInput::OnKeyDown(key_t inKey, bool bInRepeat)
	{
		if (bIgnoreInput)
		{
			return;
		}

		for (auto iter = mControlStack.rbegin(); iter != mControlStack.rend(); ++iter)
		{
			if (iter->bActive && iter->OnKeyDown(inKey, bInRepeat))
			{
				return;
			}
		}
	}

	void UGameInput::OnKeyUp(key_t inKey)
	{
		if (bIgnoreInput)
		{
			return;
		}

		for (auto iter = mControlStack.rbegin(); iter != mControlStack.rend(); ++iter)
		{
			if (iter->bActive && iter->OnKeyUp(inKey))
			{
				return;
			}
		}
	}

	void UGameInput::OnFocusChanged(bool bHasFocus)
	{
		static EMouseMode cachedMouseMode = EMouseMode::MM_UI;

		bIgnoreInput = !bHasFocus;

		if (!bHasFocus)
		{
			cachedMouseMode = mMouseMode;
			SetMouseMode(EMouseMode::MM_UI);
			ResetAxis();
		}
		else
		{
			SetMouseMode(cachedMouseMode);
		}
	}

	SControlScheme* UGameInput::AddControlScheme(const SControlScheme& inControlScheme, bool bMakeActive)
	{
		for (const auto& controlScheme : mControlStack)
		{
			if (controlScheme.mName == inControlScheme.mName)
			{
				LOG(LogGameInput, Warning, "Cannot add multiple control schemes of the same name.\n");
				return nullptr;
			}
		}

		mControlStack.push_back(inControlScheme);
		mControlStack.back().bActive = bMakeActive;
		return &mControlStack.back();
	}

	SControlScheme* UGameInput::GetControlScheme(const string& inName)
	{
		for (auto& controlScheme : mControlStack)
		{
			if (controlScheme.mName == inName)
			{
				return &controlScheme;
			}
		}

		LOG(LogGameInput, Warning, "Control scheme '%s' not found\n", inName.c_str());
		return nullptr;
	}

	void UGameInput::SetMouseMode(EMouseMode inMouseMode)
	{
		if (mMouseMode == inMouseMode)
		{
			return;
		}

		mMouseMode = inMouseMode;
		switch (mMouseMode)
		{
		case EMouseMode::MM_UI:
			gEngine->GetWindow().CaptureCursor(false);
			UGameWindow::ShowCursor(true);
			break;
		case EMouseMode::MM_Game:
			gEngine->GetWindow().CaptureCursor(true);
			gEngine->GetWindow().CenterCursor();
			UGameWindow::ShowCursor(false);
			break;
		default:
			MAD_ASSERT_DESC(false, "Unrecognized EInputMode");
			return;
		}

		ResetMouseInfo();
	}

	void UGameInput::OnAxis(EInputAxis inAxis, float inValue)
	{
		for (auto iter = mControlStack.rbegin(); iter != mControlStack.rend(); ++iter)
		{
			if (iter->bActive && iter->OnAxis(inAxis, inValue))
			{
				return;
			}
		}
	}

	void UGameInput::ResetMouseInfo()
	{
		POINT mousePos = gEngine->GetWindow().GetCursorPos();
		mMousePosX = mousePos.x;
		mMousePosY = mousePos.y;

		if (mMouseMode == EMouseMode::MM_Game)
		{
			mMouseDeltaX = 0;
			mMouseDeltaY = 0;
		}
		else if (mMouseMode == EMouseMode::MM_UI)
		{
			mMouseDeltaX = mMousePosX;
			mMouseDeltaY = mMousePosY;
		}
	}

	void UGameInput::ResetAxis()
	{

		for (auto iter = mControlStack.rbegin(); iter != mControlStack.rend(); ++iter)
		{
			iter->ClearAxis();
		}
	}

	void UGameInput::UnBindObject(void* inObj)
	{
		for (auto iter = mControlStack.begin(); iter != mControlStack.end(); ++iter)
		{
			iter->UnBindObject(inObj);
		}
	}
}
