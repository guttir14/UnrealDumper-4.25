#include "wrappers.h"

UE_UClass TUObjectArray::FindClass(std::string name)
{
	for (auto Id = 0; Id < NumElements; Id++)
	{
		UE_UClass object = GetObjectPtr(Id);
		if (object.GetFullName() == name)
		{
			return object;
		}
	}
	return nullptr;
}