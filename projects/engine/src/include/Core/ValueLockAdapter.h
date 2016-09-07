#pragma once

namespace MAD
{
	// Utility class template that allows you lock a value from being changed after a certain point
	template <typename ValueType>
	class ValueLockAdapater
	{
	public:
		ValueLockAdapater() : m_isLocked(false), m_internalValue(ValueType()) {}

		explicit ValueLockAdapater(const ValueType& inInitialValue) : m_isLocked(false), m_internalValue(inInitialValue) {}

		void LockValue() { m_isLocked = true; }
		
		void SetValue(const ValueType& inValue)
		{
			if (!m_isLocked)
			{
				m_internalValue = inValue;
			}
		}

		const ValueType& GetValue() const { return m_internalValue; }
	private:
		bool m_isLocked;
		ValueType m_internalValue;
	};
}