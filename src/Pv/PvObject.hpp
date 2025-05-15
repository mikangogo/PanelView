#pragma once

template <class ObjectType, typename HandleType>
struct PvObject
{
private:
    PvObject() = default;

public:
    static ObjectType& HandleToObject(HandleType handle)
    {
        if (!handle)
        {
            pvPfThrowException("Failed: PvObject::HandleToObject");
        }

        return *(static_cast<ObjectType*>(handle));
    }

    static ObjectType* HandleToPointer(HandleType handle)
    {
        return static_cast<ObjectType*>(handle);
    }

    static bool Sanity(HandleType handle)
    {
        return handle != nullptr;
    }

    friend ObjectType;
};
