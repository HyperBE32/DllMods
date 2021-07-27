#pragma once

#include <BlueBlur.h>

namespace Hedgehog::Base
{
    class CSharedString;

    static FUNCTION_PTR(CSharedString*, __thiscall, fpCSharedStringCtor, 0x6621A0, CSharedString* This, const char* pValue);
    static FUNCTION_PTR(void, __thiscall, fpCSharedStringDtor, 0x661550, CSharedString* This);
    static FUNCTION_PTR(CSharedString*, __thiscall, fpCSharedStringAssign, 0x662010, CSharedString* This, CSharedString const* Other);

    class CSharedString
    {
    public:
        const char* m_pStr;

        CSharedString(const char* pValue)
        {
            fpCSharedStringCtor(this, pValue);
        }

        ~CSharedString()
        {
            fpCSharedStringDtor(this);
        }

        void operator= (const CSharedString& other)
        {
            fpCSharedStringAssign(this, &other);
        }
    };

    ASSERT_OFFSETOF(CSharedString, m_pStr, 0x0);
    ASSERT_SIZEOF(CSharedString, 0x4);
}