#pragma once

namespace MAD
{
	template <typename SingletonType>
	class Singleton
	{
	public:
		static SingletonType& Get()
		{
			static SingletonType s_singleInstance;

			return s_singleInstance;
		}
	protected:
		Singleton() {} // Protected so that child class instances can't be instantiated from other methods
	};

#define MAD_SINGLETON(SingletonType)		\
	friend class Singleton<SingletonType>;	\
	private:								\
		SingletonType() {}

}