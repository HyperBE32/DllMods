﻿#pragma once

#include <BlueBlur.h>

#include <Hedgehog/Base/Container/hhList.h>
#include <Hedgehog/Base/Container/hhMap.h>
#include <Hedgehog/Base/Type/hhSharedString.h>
#include <Hedgehog/Base/Thread/hhSynchronizedObject.h>
#include <Hedgehog/Base/hhObject.h>

namespace Hedgehog::Database
{
    class CDatabase;
}

namespace Hedgehog::Universe
{
    class CUpdateManager;
}

namespace Sonic
{
    class CGameObject;
    class CLightManager;
    class CRenderDirector;
    class CWorld;

    class CGameDocument : public Hedgehog::Base::CSynchronizedObject
    {
    public:
        class CMember
        {
        public:
            boost::shared_ptr<Hedgehog::Universe::CUpdateManager> m_spUpdateManager;
            boost::shared_ptr<CRenderDirector> m_spRenderDirector;
            Hedgehog::map<Hedgehog::Base::CSharedString, boost::shared_ptr<CWorld>> m_Worlds;
            boost::shared_ptr<Hedgehog::Database::CDatabase> m_spDatabase;
            Hedgehog::list<boost::shared_ptr<CGameObject>> m_GameObjects;
            INSERT_PADDING(0x50);
            boost::shared_ptr<CLightManager> m_spLightManager;
            INSERT_PADDING(0x128);
        };

        static constexpr CGameDocument** ms_pInstance = (CGameDocument**)0x1E0BE5C;

        static CGameDocument* GetInstance()
        {
            return *ms_pInstance;
        }

        CMember* m_pMember;
        INSERT_PADDING(0x14);

        virtual ~CGameDocument() = default;
        virtual void _4() = 0;
        virtual void _8() = 0;
        virtual void _C() = 0;
        virtual void _10() = 0;
        virtual void _14() = 0;

        boost::shared_ptr<CWorld> GetWorld(const char* name = "main") const
        {
            for (auto it = m_pMember->m_Worlds.begin(); it != m_pMember->m_Worlds.end(); it = it->next())
            {
                if (it->m_Value.m_Key == name)
                    return it->m_Value.m_Value;
            }

            return nullptr;
        }
    };

    ASSERT_OFFSETOF(CGameDocument::CMember, m_spUpdateManager, 0x0);
    ASSERT_OFFSETOF(CGameDocument::CMember, m_spRenderDirector, 0x8);
    ASSERT_OFFSETOF(CGameDocument::CMember, m_Worlds, 0x10);
    ASSERT_OFFSETOF(CGameDocument::CMember, m_spDatabase, 0x1C);
    ASSERT_OFFSETOF(CGameDocument::CMember, m_GameObjects, 0x24);
    ASSERT_OFFSETOF(CGameDocument::CMember, m_spLightManager, 0x80);
    ASSERT_SIZEOF(CGameDocument::CMember, 0x1B0);

    ASSERT_OFFSETOF(CGameDocument, m_pCriticalSection, 0x4);
    ASSERT_OFFSETOF(CGameDocument, m_pMember, 0x8);
    ASSERT_SIZEOF(CGameDocument, 0x20);
}