#pragma once
#include "basic_types.h"

template<class T>
class cManager
{
public:
	T* m_Array_4;
	uint32_t m_nArray_8;
	uint32_t m_blockSize_C;
	uint8_t m_DieTimelag_10;
	uint8_t pad_11[3];
	T* m_pAlive_14;
	T* m_pAliveBack_18;
	uint32_t m_nMaxActiveWork_1C; // TODO: unsure if this is part of cManager or maybe the cxxxMgr (cEmMgr etc) classes that inherit it

	T* get(uint32_t i)
	{
		return (T*)(((uint8_t*)m_Array_4) + (i * m_blockSize_C));
	}

	T* operator[](uint32_t i)
	{
		return get(i);
	}

	int indexOf(T* entry)
	{
		// m_Array_4 is a big block of all the entries, check that it's part of block & figure out index of it
		if (m_Array_4 > entry)
			return -1;
		if (get(m_nArray_8) <= entry)
			return -1;
		uintptr_t offset = (uintptr_t)entry - (uintptr_t)m_Array_4;
		return (int)(offset / m_blockSize_C);
	}

	// iterator is kinda unnecessary.. was going to use it to only iterate through valid items, but found a better way for that instead
	// the codes already written up now though, not much point in removing it?
	struct Iterator
	{
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = T*;
		using reference = T&;

		Iterator(pointer ptr, int size) : m_ptr(ptr), m_ptr_size(size) {}

		reference operator*() const { return *m_ptr; }
		pointer operator->() { return m_ptr; }
		Iterator& operator++() { increment(); return *this; }
		Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
		friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
		friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };

	private:
		void increment() { m_ptr = (pointer)(((uint8_t*)m_ptr) + m_ptr_size); }
		pointer m_ptr;
		pointer m_end_ptr;
		int m_ptr_size = sizeof(value_type);
	};

	Iterator begin() { return Iterator(get(0), m_blockSize_C); }
	Iterator end() { return Iterator(get(m_nArray_8), m_blockSize_C); }
	int count() { return m_nArray_8; }

	virtual ~cManager() = 0;
	virtual void* memAlloc() = 0;
	virtual void memFree(void* ptr) = 0;
	virtual void memClear(void* ptr, uint32_t size) = 0;
	virtual void log(char* pStr, ...) = 0;
	virtual void destroy(T* pT) = 0;
	virtual int construct(T* result, uint32_t id) = 0;
};
assert_size(cManager<int>, 0x20);
