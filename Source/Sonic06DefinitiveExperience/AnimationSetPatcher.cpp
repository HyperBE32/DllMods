#include "AnimationSetPatcher.h"
#include "Configuration.h"

HOOK(bool, __fastcall, CAnimationControlSingle_Debug, 0x6D84F0, uint32_t** This, void* Edx, float a2, int a3)
{
    std::string name((char*)(This[58][2]));
    if (name.find("sn_") != string::npos)
    {
        printf("%s\n", name.c_str());
    }
    return originalCAnimationControlSingle_Debug(This, Edx, a2, a3);
}

HOOK(int*, __fastcall, CSonic_AnimationBlending, 0xE14A90, void* This, void* Edx, int a2, float a3)
{
    return nullptr;
}

std::vector<NewAnimationData> AnimationSetPatcher::m_newAnimationData;
HOOK(void*, __cdecl, InitializeSonicAnimationList, 0x1272490)
{
    void* pResult = originalInitializeSonicAnimationList();
    {
        CAnimationStateSet* pList = (CAnimationStateSet*)0x15E8D40;
        CAnimationStateInfo* pEntries = new CAnimationStateInfo[pList->m_Count + AnimationSetPatcher::m_newAnimationData.size()];
        std::copy(pList->m_pEntries, pList->m_pEntries + pList->m_Count, pEntries);

        for (size_t i = 0; i < AnimationSetPatcher::m_newAnimationData.size(); i++)
        {
            NewAnimationData const& data = AnimationSetPatcher::m_newAnimationData[i];

            pEntries[pList->m_Count + i].m_Name = data.m_stateName;
            pEntries[pList->m_Count + i].m_FileName = data.m_fileName;
            pEntries[pList->m_Count + i].m_Speed = data.m_speed;
            pEntries[pList->m_Count + i].m_PlaybackType = !data.m_isLoop;
            pEntries[pList->m_Count + i].field10 = 0;
            pEntries[pList->m_Count + i].field14 = -1.0f;
            pEntries[pList->m_Count + i].field18 = -1.0f;
            pEntries[pList->m_Count + i].field1C = 0;
            pEntries[pList->m_Count + i].field20 = -1;
            pEntries[pList->m_Count + i].field24 = -1;
            pEntries[pList->m_Count + i].field28 = -1;
            pEntries[pList->m_Count + i].field2C = -1;
        }

        WRITE_MEMORY(&pList->m_pEntries, void*, pEntries);
        WRITE_MEMORY(&pList->m_Count, size_t, pList->m_Count + AnimationSetPatcher::m_newAnimationData.size());
    }

    return pResult;
}

FUNCTION_PTR(void*, __stdcall, fpCreateAnimationState, 0xCDFA20, void* This, boost::shared_ptr<void>& spAnimationState, const Hedgehog::Base::CSharedString& name, const Hedgehog::Base::CSharedString& name2);
FUNCTION_PTR(uint32_t*, __stdcall, fpGetAnimationTransitionData, 0xCDFB40, void* A2, const Hedgehog::Base::CSharedString& name);
HOOK(void, __fastcall, CSonicCreateAnimationStates, 0xE1B6C0, void* This, void* Edx, void* A2, void* A3)
{
    boost::shared_ptr<void> spAnimationState;
    for (NewAnimationData const& data : AnimationSetPatcher::m_newAnimationData)
    {
        fpCreateAnimationState(A2, spAnimationState, data.m_stateName, data.m_stateName);
    }

    // Set transition data
    for (NewAnimationData const& data : AnimationSetPatcher::m_newAnimationData)
    {
        if (!data.m_destinationState) continue;

        // Initialise data on destination state
        bool found = false;
        for (NewAnimationData const& destData : AnimationSetPatcher::m_newAnimationData)
        {
            if (data.m_destinationState == destData.m_stateName)
            {
                uint32_t* pTransitionDestData = fpGetAnimationTransitionData(A2, destData.m_stateName);
                *(uint64_t*)(*pTransitionDestData + 96) = 1;
                *(uint64_t*)(*pTransitionDestData + 104) = 0;
                *(uint32_t*)(*pTransitionDestData + 112) = 1;
                found = true;
                break;
            }
        }

        if (found)
        {
            uint32_t* pTransitionData = fpGetAnimationTransitionData(A2, data.m_stateName);
            *(uint64_t*)(*pTransitionData + 96) = 1;
            *(uint64_t*)(*pTransitionData + 104) = 0;
            *(uint32_t*)(*pTransitionData + 112) = 1;
            *(float*)(*pTransitionData + 140) = -1.0f;
            *(bool*)(*pTransitionData + 144) = true;
            *(Hedgehog::Base::CSharedString*)(*pTransitionData + 136) = data.m_destinationState;
        }
        else
        {
            MessageBox(NULL, L"Animation transition destination does not exist, please check your code!", NULL, MB_ICONERROR);
        }
    }

    originalCSonicCreateAnimationStates(This, Edx, A2, A3);
}

const char* volatile const AnimationSetPatcher::RunResult = "RunResult";
const char* volatile const AnimationSetPatcher::RunResultLoop = "RunResultLoop";
const char* volatile const AnimationSetPatcher::BrakeFlip = "BrakeFlip";
const char* volatile const AnimationSetPatcher::SpinFall = "SpinFall";
const char* volatile const AnimationSetPatcher::SpinFallSpring = "SpinFallSpring";
const char* volatile const AnimationSetPatcher::SpinFallLoop = "SpinFallLoop";
const char* volatile const AnimationSetPatcher::HomingAttackLoop = "HomingAttackLoop";

void AnimationSetPatcher::applyPatches()
{
    // DEBUG!!!
    //INSTALL_HOOK(CAnimationControlSingle_Debug);

    // Disable using blending animations since they cause crash
    INSTALL_HOOK(CSonic_AnimationBlending);
    WRITE_MEMORY(0x1274A6D, uint32_t, 0x15E7670); // sn_plate_v_l
    WRITE_MEMORY(0x1274AD4, uint32_t, 0x15E7670); // sn_plate_v_l
    WRITE_MEMORY(0x1274B3B, uint32_t, 0x15E7670); // sn_plate_v_r
    WRITE_MEMORY(0x1274BA2, uint32_t, 0x15E7670); // sn_plate_v_r
    WRITE_MEMORY(0x1274C09, uint32_t, 0x15E7670); // sn_plate_h
    WRITE_MEMORY(0x1274C70, uint32_t, 0x15E7670); // sn_plate_h
    WRITE_MEMORY(0x1278EFA, uint32_t, 0x15E7670); // sn_needle_blow_loop
    WRITE_MEMORY(0x1278F66, uint32_t, 0x15E7670); // sn_direct_l
    WRITE_MEMORY(0x1278FCD, uint32_t, 0x15E7670); // sn_direct_r

    // Trick animation for Super Form
    WRITE_STRING(0x15D58F4, "ssn_trick_jump");

    // Change Super Form dashring to use spring jump animation
    WRITE_MEMORY(0x1293D60, uint32_t, 0x15D5D8C); // DashRingL
    WRITE_MEMORY(0x1293DA7, uint32_t, 0x15D5D8C); // DashRingR

    // Set animations to loop
    WRITE_MEMORY(0x1276D20, uint8_t, 0x1D); // DashRingL
    WRITE_MEMORY(0x1276D87, uint8_t, 0x1D); // DashRingR

    if (Configuration::m_model == Configuration::ModelType::Sonic)
    {
        // Running goal
        if (Configuration::m_run != Configuration::RunResultType::Disable)
        {
            m_newAnimationData.emplace_back(RunResult, "sn_result_run", 1.0f, false, RunResultLoop);
            m_newAnimationData.emplace_back(RunResultLoop, "sn_result_run_loop", 1.0f, true, nullptr);
        }

        // Brake flip (for 06 physics)
        m_newAnimationData.emplace_back(BrakeFlip, "sn_brake_flip", 1.0f, false, nullptr);

        // Remove JumpBoard to Fall transition and add spin fall
        WRITE_JUMP(0xE1F503, (void*)0xE1F56E);
        m_newAnimationData.emplace_back(SpinFall, "sn_spin_fall", 1.0f, false, SpinFallLoop);
        m_newAnimationData.emplace_back(SpinFallSpring, "sn_spin_fall_spring", 1.0f, false, SpinFallLoop);
        m_newAnimationData.emplace_back(SpinFallLoop, "sn_jump_d_loop", 1.0f, true, nullptr);

        // Set animations to loop
        WRITE_MEMORY(0x127779C, uint8_t, 0x1D); // UpReelEnd
        WRITE_MEMORY(0x1276B84, uint8_t, 0x1D); // JumpBoard
        WRITE_MEMORY(0x1276BEB, uint8_t, 0x1D); // JumpBoardRev
        WRITE_MEMORY(0x1276C4D, uint8_t, 0x1D); // JumpBoardSpecialL
        WRITE_MEMORY(0x1276CB9, uint8_t, 0x1D); // JumpBoardSpecialR
    }
    
    if (Configuration::m_model == Configuration::ModelType::SonicElise)
    {
        // Use unique animation for homing attack
        m_newAnimationData.emplace_back(HomingAttackLoop, "sn_homing_loop", 1.0f, true, nullptr);
    }

    if (!m_newAnimationData.empty())
    {
        INSTALL_HOOK(InitializeSonicAnimationList);
        INSTALL_HOOK(CSonicCreateAnimationStates);
    }
}
