#pragma once

#include <EASTL/deque.h>
#include <EASTL/hash_map.h>
#include <EASTL/list.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/string.h>

#include "Misc/Delegate.h"

namespace MAD
{
	DECLARE_DELEGATE(InputEventDelegate, void);
	DECLARE_DELEGATE(AxisDelegate, void, float);

	typedef uint32_t key_t;

	enum class EInputEvent
	{
		IE_KeyDown,
		IE_KeyUp,
		IE_KeyHeld,
	};

	enum class EInputAxis: key_t
	{
		IA_None		= 0x100,
		IA_MouseX	= 0x101,
		IA_MouseY	= 0x102,
	};

	enum class EMouseMode
	{
		MM_Game,
		MM_UI,
	};

	struct SActionBinding
	{
		DECLARE_MULTICAST_DELEGATE(InputEventMulticastDelegate);

		eastl::string Action;
		key_t Key;
		InputEventMulticastDelegate OnPressed;
		InputEventMulticastDelegate OnHeld;
		InputEventMulticastDelegate OnReleased;

		SActionBinding(const eastl::string& inAction, key_t inKey) :
			Action(inAction),
			Key(inKey) { }
	};

	struct SAxisBinding
	{
		DECLARE_MULTICAST_DELEGATE(AxisMulticastDelegate, float);

		eastl::string Axis;
		key_t PosKey;
		key_t NegKey;
		float Value;
		AxisMulticastDelegate OnAxis;
		bool bTickedThisFrame;

		SAxisBinding(const eastl::string& inAxis, key_t inPosKey, key_t inNegKey) :
			Axis(inAxis),
			PosKey(inPosKey),
			NegKey(inNegKey),
			Value(0.0f),
			bTickedThisFrame(false) { }

		SAxisBinding(const eastl::string& inAxisName, EInputAxis inAxis) :
			Axis(inAxisName),
			PosKey(static_cast<key_t>(inAxis)),
			NegKey(static_cast<key_t>(inAxis)),
			Value(0.0f),
			bTickedThisFrame(false) { }
	};

	struct SControlScheme
	{
		SControlScheme(const eastl::string& inName) :
			mName(inName),
			bActive(false) { }

		SControlScheme& RegisterEvent(const eastl::string& inEventName, key_t inKey);
		SControlScheme& RegisterAxis(const eastl::string& inAxisName, key_t inPosKey, key_t inNegKey);
		SControlScheme& RegisterAxis(const eastl::string& inAxisName, EInputAxis inAxisType);
		SControlScheme& Finalize(bool bMakeActive = false) const;

		const eastl::string& GetName() const { return mName; }
		bool IsActive() const { return bActive; }
		void SetActive(bool bNewActive) { bActive = bNewActive; }

		void ClearAxis();

		template<class T, void(T::*Function)()>
		bool BindEvent(const eastl::string& inEventName, EInputEvent inType, T* inObj);
		template<class T, void(T::*Function)() const>
		bool BindEvent(const eastl::string& inEventName, EInputEvent inType, T* inObj);
		template<void(*Function)()>
		bool BindEvent(const eastl::string& inEventName, EInputEvent inType);
		bool BindEvent(const eastl::string& inEventName, EInputEvent inType, InputEventDelegate inCallback);

		template<class T, void(T::*Function)(float)>
		bool BindAxis(const eastl::string& inAxisName, T* inObj);
		template<class T, void(T::*Function)(float) const>
		bool BindAxis(const eastl::string& inAxisName, T* inObj);
		template<void(*Function)(float)>
		bool BindAxis(const eastl::string& inAxisName);
		bool BindAxis(const eastl::string& inAxisName, AxisDelegate inCallback);

	private:
		friend class UGameInput;

		eastl::string mName;
		bool bActive;

		eastl::hash_map<key_t, eastl::list<SActionBinding>> KeyToActionBindings;
		eastl::hash_map<key_t, eastl::list<eastl::shared_ptr<SAxisBinding>>> KeyToAxisBindings;

		eastl::hash_map<eastl::string, key_t> StringToKeyBindings;

		bool OnKeyDown(key_t inKey, bool bInRepeat);
		bool OnKeyUp(key_t inKey);

		bool OnAxis(EInputAxis inAxis, float inValue);
		void TickAxis();
	};

	class UGameInput
	{
	public:
		static UGameInput& Get()
		{
			static UGameInput sInstance;
			return sInstance;
		}

		void Tick();

		void OnChar(char inChar, bool bInRepeat) const;

		void OnKeyDown(key_t inKey, bool bInRepeat);
		void OnKeyUp(key_t inKey);

		void OnFocusChanged(bool bHasFocus);

		SControlScheme* AddControlScheme(const SControlScheme& inControlScheme, bool bMakeActive = false);
		SControlScheme* GetControlScheme(const eastl::string& inName);

		EMouseMode GetMouseMode() const { return mMouseMode; }
		void SetMouseMode(EMouseMode inMouseMode);

		void GetMousePos(int32_t& outX, int32_t& outY) const { outX = mMousePosX; outY = mMousePosY; }

	private:
		UGameInput();

		void UpdateMouse();

		void OnAxis(EInputAxis inAxis, float inValue);

		void ResetMouseInfo();

		void ResetAxis();

		eastl::deque<SControlScheme> mControlStack;

		int32_t mMousePosX;
		int32_t mMousePosY;

		int32_t mMouseDeltaX;
		int32_t mMouseDeltaY;

		bool bIgnoreInput;

		EMouseMode mMouseMode;
	};

	template<class T, void(T::*Function)()>
	inline bool SControlScheme::BindEvent(const eastl::string& inEventName, EInputEvent inType, T* inObj)
	{
		return BindEvent(inEventName, inType, InputEventDelegate::Create<T, Function>(inObj));
	}

	template<class T, void(T::*Function)() const>
	inline bool SControlScheme::BindEvent(const eastl::string& inEventName, EInputEvent inType, T* inObj)
	{
		return BindEvent(inEventName, inType, InputEventDelegate::Create<T, Function>(inObj));
	}

	template<void(*Function)()>
	inline bool SControlScheme::BindEvent(const eastl::string& inEventName, EInputEvent inType)
	{
		return BindEvent(inEventName, inType, InputEventDelegate::Create<Function>());
	}

	template<class T, void(T::*Function)(float)>
	inline bool SControlScheme::BindAxis(const eastl::string & inAxisName, T* inObj)
	{
		return BindAxis(inAxisName, AxisDelegate::Create<T, Function>(inObj));
	}

	template<class T, void(T::*Function)(float) const>
	inline bool SControlScheme::BindAxis(const eastl::string & inAxisName, T* inObj)
	{
		return BindAxis(inAxisName, AxisDelegate::Create<T, Function>(inObj));
	}

	template<void(*Function)(float)>
	inline bool SControlScheme::BindAxis(const eastl::string & inAxisName)
	{
		return BindAxis(inAxisName, AxisDelegate::Create<Function>());
	}
}
