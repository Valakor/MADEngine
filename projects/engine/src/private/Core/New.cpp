#include <new>

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	(void)pName;
	(void)flags;
	(void)debugFlags;
	(void)file;
	(void)line;

	return operator new[](size);
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	(void)alignment;
	(void)alignmentOffset;
	(void)pName;
	(void)flags;
	(void)debugFlags;
	(void)file;
	(void)line;

	return operator new[](size);
}