#pragma once

/** 
 * Super-cool C++11 delegates by Matt Pohlmann.
 *
 * For functions of the form `int X::Foo(float)` use:
 *     MyClass myObj;
 *     typedef SDelegate<int, float> Delegate_int_float;
 *     Delegate_int_float myDelegate;
 *     myDelegate.BindMember<MyClass, &MyClass::MyFoo>(&myObj);
 *     myDelegate.Execute(1.5f); // Calls myObj.MyFoo(1.5f);
 *
 * For static functions of the form `int Foo(float)` use:
 *     typedef SDelegate<int, float> Delegate_int_float;
 *     Delegate_int_float myDelegate;
 *     myDelegate.BindStatic<&MyClass::MyFoo>();
 *     myDelegate.Execute(1.5f); // Calls MyClass::MyFoo(1.5f);
 *
 * TODO: Figure out how to safely iterate over multicast delegates/events
 *       when bound functions/methods may add/remove to the delegate during
 *       iteration. (Currently I make a copy of the entire delegate list,
 *       but there must be a better way.)
 *       Idea: Use Vector/LList instead of Dictionary and iterate backwards
 *       over the list on Execute. Makes all additions safe, but probably
 *       doesn't fix removals? Look into re-entrancy, copying the current
 *       Delegate of execution, etc.
 *       For removals, possibly keep track if currently executing and rather
 *       than immediate removal, add to list to remove after current execution
 *       completes?
 *
 */

#include <EASTL/hash_map.h>

#include "Assert.h"

namespace MAD
{
	template <typename Ret, typename ...Params>
	struct SDelegate
	{
	private:
		using FunctionStub_t = Ret(*)(void*, Params...);

		template<Ret(Func)(Params...)>
		static inline Ret StaticFuncStub(void* inObj, Params... p)
		{
			(void)inObj;
			return (Func)(p...);
		}

		template<class T, Ret(T::*Func)(Params...)>
		static inline Ret MemberFuncStub(void* inObj, Params... p)
		{
			return (static_cast<T*>(inObj)->*Func)(p...);
		}

		template<class T, Ret(T::*Func)(Params...) const>
		static inline Ret ConstMemberFuncStub(void* inObj, Params... p)
		{
			return (static_cast<T*>(inObj)->*Func)(p...);
		}

	public:
		SDelegate() : mObj(nullptr), mFuncStub(nullptr) { }
		SDelegate(SDelegate const&) = default;
		SDelegate(SDelegate&&) = default;
		SDelegate(nullptr_t const) noexcept : SDelegate() { }

		SDelegate& operator=(SDelegate const&) = default;
		SDelegate& operator=(SDelegate&&) = default;

		inline bool operator==(const SDelegate& rhs) const noexcept
		{
			return mObj == rhs.mObj && mFuncStub == rhs.mFuncStub;
		}

		inline bool operator!=(const SDelegate& rhs) const noexcept
		{
			return !operator==(rhs);
		}

		inline bool operator==(nullptr_t const) const noexcept
		{
			return !IsBound();
		}

		inline bool operator!=(nullptr_t const) const noexcept
		{
			return IsBound();
		}

		explicit inline operator bool() const noexcept { return IsBound(); }


		template <class T, Ret(T::* Func)(Params...)>
		inline void BindMember(T* inObj)
		{
			MAD_ASSERT_DESC(inObj != nullptr, "Cannot bind a delegate to nullptr.");
			mObj = inObj;
			mFuncStub = &MemberFuncStub<T, Func>;
		}

		template <class T, Ret(T::* Func)(Params...) const>
		inline void BindMember(T* inObj)
		{
			MAD_ASSERT_DESC(inObj != nullptr, "Cannot bind a delegate to nullptr.");
			mObj = inObj;
			mFuncStub = &ConstMemberFuncStub<T, Func>;
		}

		template <Ret Func(Params...)>
		inline void BindStatic()
		{
			mObj = nullptr;
			mFuncStub = &StaticFuncStub<Func>;
		}

		inline void UnBind()
		{
			mObj = nullptr;
			mFuncStub = nullptr;
		}

		inline bool IsBound() const
		{
			return (mFuncStub != nullptr);
		}

		inline Ret Execute(Params... p) const
		{
			MAD_ASSERT_DESC(IsBound(), "Cannot execute an unbound delegate.");
			return (mFuncStub)(mObj, p...);
		}

		inline Ret ExecuteIfBound(Params... p) const
		{
			if (IsBound())
			{
				return Execute(p...);
			}

			return Ret();
		}

		template <class T, Ret(T::*Function)(Params...)>
		inline static SDelegate<Ret, Params...> Create(T* inObj)
		{
			SDelegate<Ret, Params...> result;
			result.template BindMember<T, Function>(inObj);
			return result;
		}

		template <class T, Ret(T::*Function)(Params...) const>
		inline static SDelegate<Ret, Params...> Create(T* inObj)
		{
			SDelegate<Ret, Params...> result;
			result.template BindMember<T, Function>(inObj);
			return result;
		}

		template <Ret(*Function)(Params...)>
		inline static SDelegate<Ret, Params...> Create()
		{
			SDelegate<Ret, Params...> result;
			result.BindStatic<Function>();
			return result;
		}

	private:
		template <typename ...Ps> friend struct SMulticastDelegateBase;

		void* mObj;
		FunctionStub_t mFuncStub;
	};

	struct SDelegateHandle
	{
		SDelegateHandle() :
			mHandle(UINT64_MAX)
		{ }

	private:
		template <typename ...Params> friend struct SMulticastDelegateBase;
		template <typename ...Params> friend struct SMulticastDelegate;
		template <typename ...Params> friend struct SMulticastEvent;
		typedef uint64_t HandleType;

		explicit SDelegateHandle(HandleType inHandle) :
			mHandle(inHandle)
		{ }

		HandleType mHandle;

	public:
		inline bool operator==(const SDelegateHandle& rhs) const noexcept
		{
			return mHandle == rhs.mHandle;
		}

		inline bool operator!=(const SDelegateHandle& rhs) const noexcept
		{
			return !operator==(rhs);
		}

		inline bool operator==(HandleType h) const noexcept
		{
			return mHandle == h;
		}

		inline bool operator!=(HandleType h) const noexcept
		{
			return !operator==(h);
		}
	};

	template <typename ...Params>
	struct SMulticastDelegateBase
	{
		using DelegateType = SDelegate<void, Params...>;

		SMulticastDelegateBase() = default;
		SMulticastDelegateBase(SMulticastDelegateBase const&) = default;
		SMulticastDelegateBase(SMulticastDelegateBase&&) = default;

		SMulticastDelegateBase& operator=(SMulticastDelegateBase const&) = default;
		SMulticastDelegateBase& operator=(SMulticastDelegateBase&&) = default;

		inline bool operator==(const SMulticastDelegateBase& rhs) const noexcept
		{
			return mUserFuncs == rhs.mUserFuncs;
		}

		inline bool operator!=(const SMulticastDelegateBase& rhs) const noexcept
		{
			return !operator==(rhs);
		}

		template <class T, void(T::*Function)(Params...)>
		inline SDelegateHandle BindMember(T* inObj)
		{
			auto d = DelegateType::template Create<T, Function>(inObj);
			MAD_ASSERT_DESC(!IsDuplicate(d), "Cannot bind the same function to a multicast delegate more than once.");
			auto h = HandleGen();
			mUserFuncs.emplace(h.mHandle, d);
			return h;
		}

		template <class T, void(T::*Function)(Params...) const>
		inline SDelegateHandle BindMember(T* inObj)
		{
			auto d = DelegateType::template Create<T, Function>(inObj);
			MAD_ASSERT_DESC(!IsDuplicate(d), "Cannot bind the same function to a multicast delegate more than once.");
			auto h = HandleGen();
			mUserFuncs.emplace(h.mHandle, d);
			return h;
		}

		template <void(*Function)(Params...)>
		inline SDelegateHandle BindStatic()
		{
			auto d = DelegateType::template Create<Function>();
			MAD_ASSERT_DESC(!IsDuplicate(d), "Cannot bind the same function to a multicast delegate more than once.");
			auto h = HandleGen();
			mUserFuncs.emplace(h.mHandle, d);
			return h;
		}

		inline SDelegateHandle BindDelegate(const DelegateType& inDelegate)
		{
			MAD_ASSERT_DESC(!IsDuplicate(inDelegate), "Cannot bind the same function to a multicast delegate more than once.");
			auto h = HandleGen();
			mUserFuncs[h.mHandle] = inDelegate;
			return h;
		}

		inline bool UnBind(SDelegateHandle inHandle)
		{
			auto iter = mUserFuncs.find(inHandle.mHandle);
			if (iter != mUserFuncs.end())
			{
				mUserFuncs.erase(iter);
				return true;
			}

			return false;
		}

	protected:
		eastl::hash_map<SDelegateHandle::HandleType, DelegateType> mUserFuncs;

	protected:
		bool IsDuplicate(const DelegateType& inDelegate)
		{
			for (const auto& pair : mUserFuncs)
			{
				if (pair.second == inDelegate) return true;
			}
			return false;
		}

		inline static SDelegateHandle HandleGen()
		{
			static SDelegateHandle::HandleType sHandle = 0;
			return SDelegateHandle(sHandle++);
		}
	};

	template <typename ...Params>
	struct SMulticastDelegate : public SMulticastDelegateBase<Params...>
	{
	public:
		inline void Execute(Params... p) const
		{
			auto userFuncsCpy = SMulticastDelegateBase<Params...>::mUserFuncs;
			for (const auto& pair : userFuncsCpy)
			{
				pair.second.Execute(p...);
			}
		}

		inline void Clear()
		{
			SMulticastDelegateBase<Params...>::mUserFuncs.clear();
		}
	};

	template <typename ...Params>
	struct SMulticastEvent : public SMulticastDelegateBase<Params...>
	{
	protected:
		inline void Execute(Params... p) const
		{
			auto userFuncsCpy = SMulticastDelegateBase<Params...>::mUserFuncs;
			for (const auto& pair : userFuncsCpy)
			{
				pair.second.Execute(p...);
			}
		}

		inline void Clear()
		{
			SMulticastDelegateBase<Params...>::mUserFuncs.clear();
		}
	};

	// Use like (for functions of type bool(int):
	//		DECLARE_DELEGATE(MyDelegate, bool, int);
	//		MyDelegate foo;
	// Wraps a single static or member function pointer. Can have
	// any return type or argument list.
	#define DECLARE_DELEGATE(Name, Ret, ...) \
		using Name = SDelegate<Ret, __VA_ARGS__>

	// Use like (for functions of type void(int):
	//		DECLARE_MULTICAST_DELEGATE(MyMulticastDelegate, int);
	//		MyMulticastDelegate foo;
	// Wraps a list of static or member function pointers. Can have
	// void return type and any argument list.
	#define DECLARE_MULTICAST_DELEGATE(Name, ...) \
		using Name = SMulticastDelegate<__VA_ARGS__>

	// Use like (for functions of type void(int):
	//		DECLARE_EVENT(MyOwningClass, MyEvent, int);
	//		MyEvent foo;
	// Wraps a list of static or member function pointers. Can have
	// void return type and any argument list. Can only be executed
	// by the Owner class/struct.
	#define DECLARE_EVENT(Owner, Name, ...) \
		struct Name : public SMulticastEvent<__VA_ARGS__> \
		{ \
			friend Owner; \
		}
}
