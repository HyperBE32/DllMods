#include "Stage.h"
#include "Application.h"
#include "Configuration.h"
#include "UIContext.h"
#include "ParamManager.h"
#include "LoadingUI.h"

//---------------------------------------------------
// Kingdom Valley sfx
//---------------------------------------------------
HOOK(int32_t*, __fastcall, Stage_MsgHitGrindPath, 0xE25680, void* This, void* Edx, uint32_t a2)
{
    // If at Kingdom Valley, change sfx to wind
    // There are normal rails too, when x > 210
    Eigen::Vector3f playerPosition;
    Eigen::Quaternionf playerRotation;
    if (Common::CheckCurrentStage("euc200") 
     && Common::GetPlayerTransform(playerPosition, playerRotation)
     && playerPosition.x() < 210.0f)
    {
        WRITE_MEMORY(0xE4FC73, uint8_t, 0);
        WRITE_MEMORY(0xE4FC82, uint32_t, 4042004);
        WRITE_MEMORY(0xE4FCD4, uint8_t, 0);
        WRITE_MEMORY(0xE4FCD6, uint32_t, 4042005);
    }
    else
    {
        WRITE_MEMORY(0xE4FC73, uint8_t, 1);
        WRITE_MEMORY(0xE4FC82, uint32_t, 2002038);
        WRITE_MEMORY(0xE4FCD4, uint8_t, 1);
        WRITE_MEMORY(0xE4FCD6, uint32_t, 2002037);
    }

    return originalStage_MsgHitGrindPath(This, Edx, a2);
}

HOOK(int, __fastcall, Stage_CObjSpringSFX, 0x1038DA0, void* This)
{
    // If at Kingdom Valley, change sfx to rope
    if (Common::CheckCurrentStage("euc200"))
    {
        WRITE_MEMORY(0x1A6B7D8, uint32_t, 8000);
        WRITE_MEMORY(0x1A6B7EC, uint32_t, 8000);
    }
    else
    {
        WRITE_MEMORY(0x1A6B7D8, uint32_t, 4001015);
        WRITE_MEMORY(0x1A6B7EC, uint32_t, 4001015);
    }

    return originalStage_CObjSpringSFX(This);
}

//---------------------------------------------------
// Wall Jump
//---------------------------------------------------
bool Stage::m_wallJumpStart = false;
float Stage::m_wallJumpTime = 0.0f;
float const c_wallJumpSpinTime = 0.6f;
HOOK(bool, __fastcall, Stage_CSonicStateFallAdvance, 0x1118C50, void* This)
{
    bool result = originalStage_CSonicStateFallAdvance(This);

    if (Stage::m_wallJumpStart)
    {
        Common::SonicContextChangeAnimation("SpinAttack");
        Stage::m_wallJumpStart = false;
        Stage::m_wallJumpTime = c_wallJumpSpinTime;
    }
    else if (Stage::m_wallJumpTime > 0.0f)
    {
        Stage::m_wallJumpTime -= Application::getDeltaTime();
        if (Stage::m_wallJumpTime <= 0.0f)
        {
            Common::SonicContextChangeAnimation("FallFast");
        }
    }

    return result;
}

HOOK(int, __fastcall, Stage_CSonicStateFallEnd, 0x1118F20, void* This)
{
    Stage::m_wallJumpStart = false;
    Stage::m_wallJumpTime = 0.0f;
    return originalStage_CSonicStateFallEnd(This);
}

void __declspec(naked) getIsWallJump()
{
    static uint32_t returnAddress = 0xE6D5AF;
    __asm
    {
        push    esi
        push    ebx
        lea     ecx, [esi + 30h]
        call    Stage::getIsWallJumpImpl
        pop     ebx
        pop     esi

        push    [0x15F4FE8] // Walk
        jmp     [returnAddress]
    }
}

void __fastcall Stage::getIsWallJumpImpl(float* outOfControl)
{
    if (*outOfControl < 0.0f)
    {
        m_wallJumpStart = true;
        *outOfControl = -*outOfControl;
    }
    else
    {
        m_wallJumpStart = false;
    }
}

//---------------------------------------------------
// Water Running
//---------------------------------------------------
bool Stage::m_waterRunning = false; 
static SharedPtrTypeless waterSoundHandle;
HOOK(char, __stdcall, Stage_CSonicStateGrounded, 0xDFF660, int* a1, bool a2)
{
    if (Common::CheckCurrentStage("ghz200"))
    {
        alignas(16) MsgGetAnimationInfo message {};
        Common::SonicContextGetAnimationInfo(message);

        CSonicStateFlags* flags = Common::GetSonicStateFlags();
        if (flags->KeepRunning && flags->OnWater)
        {
            // Initial start, play sfx
            if (!Stage::m_waterRunning)
            {
                Common::SonicContextPlaySound(waterSoundHandle, 2002059, 1);
            }

            // Change animation
            Stage::m_waterRunning = true;
            if (!message.IsAnimation("Sliding"))
            {
                Common::SonicContextChangeAnimation("Sliding");
            }
        }
        else
        {
            // Auto-run finished
            Stage::m_waterRunning = false;
            waterSoundHandle.reset();
            if (message.IsAnimation("Sliding"))
            {
                Common::SonicContextChangeAnimation("Walk");
            }
        }
    }

    return originalStage_CSonicStateGrounded(a1, a2);
}

void __declspec(naked) playWaterPfx()
{
    static uint32_t successAddress = 0x11DD1B9;
    static uint32_t returnAddress = 0x11DD240;
    __asm
    {
        mov     edx, [ecx + 4]

        // check boost
        cmp     byte ptr[edx + 10h], 0
        jnz     jump

        // check auto-run
        cmp     byte ptr[edx + 2Dh], 0
        jnz     jump

        jmp     [returnAddress]

        jump:
        jmp     [successAddress]
    }
}

HOOK(void, __stdcall, Stage_SonicChangeAnimation, 0xCDFC80, void* a1, int a2, const hh::base::CSharedString& name)
{
    if (Stage::m_waterRunning)
    {
        // if still water running, do not use walk animation (boost)
        if (strcmp(name.c_str(), "Walk") == 0)
        {
            originalStage_SonicChangeAnimation(a1, a2, "Sliding");
            return;
        }

        alignas(16) MsgGetAnimationInfo message {};
        Common::SonicContextGetAnimationInfo(message);

        if (message.IsAnimation("Sliding"))
        {
            Stage::m_waterRunning = false;
            waterSoundHandle.reset();
        }
    }

    originalStage_SonicChangeAnimation(a1, a2, name);
}

//---------------------------------------------------
// Object Physics dummy event
//---------------------------------------------------
ParamValue* m_LightSpeedDashStartCollisionFovy = nullptr;
ParamValue* m_LightSpeedDashStartCollisionFar = nullptr;
ParamValue* m_LightSpeedDashCollisionFovy = nullptr;
ParamValue* m_LightSpeedDashCollisionFar = nullptr;
ParamValue* m_LightSpeedDashMinVelocity = nullptr;
ParamValue* m_LightSpeedDashMinVelocity3D = nullptr;
ParamValue* m_LightSpeedDashMaxVelocity = nullptr;
ParamValue* m_LightSpeedDashMaxVelocity3D = nullptr;
HOOK(void, __fastcall, Stage_MsgNotifyObjectEvent, 0xEA4F50, void* This, void* Edx, uint32_t a2)
{
    uint32_t* pEvent = (uint32_t*)(a2 + 16);
    uint32_t* pObject = (uint32_t*)This;

    switch (*pEvent)
    {
    // Change light speed dash param
    case 1001:
    {
        *m_LightSpeedDashStartCollisionFovy->m_funcData->m_pValue = 80.0f;
        m_LightSpeedDashStartCollisionFovy->m_funcData->update();

        *m_LightSpeedDashStartCollisionFar->m_funcData->m_pValue = 10.0f;
        m_LightSpeedDashStartCollisionFar->m_funcData->update();

        *m_LightSpeedDashCollisionFovy->m_funcData->m_pValue = 80.0f;
        m_LightSpeedDashCollisionFovy->m_funcData->update();

        *m_LightSpeedDashCollisionFar->m_funcData->m_pValue = 10.0f;
        m_LightSpeedDashCollisionFar->m_funcData->update();

        float speed = 80.0f;
        if (Common::CheckCurrentStage("ghz200")) speed = 100.0f;
        else if (Common::CheckCurrentStage("euc200")) speed = 65.0f;
        *m_LightSpeedDashMinVelocity->m_funcData->m_pValue = speed;
        m_LightSpeedDashMinVelocity->m_funcData->update();
        *m_LightSpeedDashMinVelocity3D->m_funcData->m_pValue = speed;
        m_LightSpeedDashMinVelocity3D->m_funcData->update();

        *m_LightSpeedDashMaxVelocity->m_funcData->m_pValue = 100.0f;
        m_LightSpeedDashMaxVelocity->m_funcData->update();
        *m_LightSpeedDashMaxVelocity3D->m_funcData->m_pValue = 100.0f;
        m_LightSpeedDashMaxVelocity3D->m_funcData->update();

        break;
    }
    case 300:
    {
        Common::PlayStageMusic("City_Escape_Generic2", 1.5f);
        break;
    }
    case 301:
    {
        Common::PlayStageMusic("Speed_Highway_Generic1", 0.0f);
        break;
    }
    case 302:
    {
        Common::PlayStageMusic("Speed_Highway_Generic2", 0.0f);
        break;
    }
    case 303:
    {
        Common::PlayStageMusic("Speed_Highway_Generic3", 0.0f);
        break;
    }
    }

    originalStage_MsgNotifyObjectEvent(This, Edx, a2);
}

//---------------------------------------------------
// Result music
//---------------------------------------------------
const char* SNG19_JNG_STH = "SNG19_JNG_STH";
HOOK(int, __fastcall, Stage_SNG19_JNG_1, 0xCFF440, void* This, void* Edx, int a2)
{
    WRITE_MEMORY(0xCFF44E, char*, SNG19_JNG_STH);
    return originalStage_SNG19_JNG_1(This, Edx, a2);
}

HOOK(void, __fastcall, Stage_SNG19_JNG_2, 0xD00F70, void* This, void* Edx, int a2)
{
    WRITE_MEMORY(0xD01A06, char*, SNG19_JNG_STH);
    originalStage_SNG19_JNG_2(This, Edx, a2);
}

HOOK(void, __fastcall, Stage_CStateGoalFadeIn, 0xCFD2D0, void* This)
{
    static const char* Result_Town = "Result_Town";
    static const char* Result = (char*)0x15B38F0;

    if (Common::IsCurrentStageMission())
    {
        WRITE_MEMORY(0xCFD3C9, char*, Result_Town);
    }
    else
    {
        WRITE_MEMORY(0xCFD3C9, char*, Result);
    }

    // Music length
    static double length = 7.831;
    WRITE_MEMORY(0xCFD562, double*, &length);

    // Always use Result1
    WRITE_MEMORY(0xCFD4E5, uint8_t, 0xEB);

    originalStage_CStateGoalFadeIn(This);
}

//---------------------------------------------------
// Perfect Chaos
//---------------------------------------------------
void Stage_CBossPerfectChaosFinalHitSfxImpl()
{
    static SharedPtrTypeless soundHandle;
    Common::PlaySoundStatic(soundHandle, 5552007);
}

void __declspec(naked) Stage_CBossPerfectChaosFinalHitSfx()
{
    static uint32_t returnAddress = 0xC0FFC5;
    static uint32_t sub_C0F580 = 0xC0F580;
    __asm
    {
        push	eax
        call	Stage_CBossPerfectChaosFinalHitSfxImpl
        pop     eax

        // original function
        call    [sub_C0F580]
        jmp     [returnAddress]
    }
}

HOOK(void, __fastcall, Stage_CBossPerfectChaosCStateDefeated, 0x5D20A0, int This)
{
    bool wasMovieStarted = *(bool*)(This + 40);
    bool wasMovieEnded = *(bool*)(This + 41);

    originalStage_CBossPerfectChaosCStateDefeated(This);

    if (!wasMovieStarted && *(bool*)(This + 40))
    {
        Common::PlayStageMusic("Dummy", 0.0f);
        LoadingUI::startNowLoading(6.8f);
    }

    if (!wasMovieEnded && *(bool*)(This + 41))
    {
        // Movie ended by player skipping
        LoadingUI::startNowLoading();
    }
}

//---------------------------------------------------
// HUD Music
//---------------------------------------------------
HOOK(void, __fastcall, Stage_CTutorialImpl, 0xD24440, int This, void* Edx, int a2)
{
    if (*(uint32_t*)(This + 176) == 3)
    {
        Common::PlayStageMusic("City_Escape_Generic2", 1.5f);
    }
    originalStage_CTutorialImpl(This, Edx, a2);
}

void Stage::applyPatches()
{
    //---------------------------------------------------
    // General
    //---------------------------------------------------
    // Disable enter CpzPipe sfx
    WRITE_MEMORY(0x1234856, int, -1);

    // Disable result first sfx
    WRITE_MEMORY(0x11D24DA, int, -1);
    
    //---------------------------------------------------
    // Kingdom Valley sfx
    //---------------------------------------------------
    // Play robe sfx in Kingdom Valley
    INSTALL_HOOK(Stage_CObjSpringSFX);

    // Play wind rail sfx for Kingdom Valley
    INSTALL_HOOK(Stage_MsgHitGrindPath);

    //---------------------------------------------------
    // Wall Jump
    //---------------------------------------------------
    // Do SpinAttack animation for walljumps (required negative out of control time)
    WRITE_JUMP(0xE6D5AA, getIsWallJump);
    INSTALL_HOOK(Stage_CSonicStateFallAdvance);
    INSTALL_HOOK(Stage_CSonicStateFallEnd);

    //---------------------------------------------------
    // Water Running
    //---------------------------------------------------
    // Do slide animation on water running in Wave Ocean
    INSTALL_HOOK(Stage_CSonicStateGrounded);
    INSTALL_HOOK(Stage_SonicChangeAnimation);
    WRITE_JUMP(0x11DD1AC, playWaterPfx);

    //---------------------------------------------------
    // Object Physics dummy event
    //---------------------------------------------------
    ParamManager::addParam(&m_LightSpeedDashStartCollisionFovy, "LightSpeedDashStartCollisionFovy");
    ParamManager::addParam(&m_LightSpeedDashStartCollisionFar, "LightSpeedDashStartCollisionFar");
    ParamManager::addParam(&m_LightSpeedDashCollisionFovy, "LightSpeedDashCollisionFovy");
    ParamManager::addParam(&m_LightSpeedDashCollisionFar, "LightSpeedDashCollisionFar");
    ParamManager::addParam(&m_LightSpeedDashMinVelocity, "LightSpeedDashMinVelocity");
    ParamManager::addParam(&m_LightSpeedDashMinVelocity3D, "LightSpeedDashMinVelocity3D");
    ParamManager::addParam(&m_LightSpeedDashMaxVelocity, "LightSpeedDashMaxVelocity");
    ParamManager::addParam(&m_LightSpeedDashMaxVelocity3D, "LightSpeedDashMaxVelocity3D");
    INSTALL_HOOK(Stage_MsgNotifyObjectEvent);

    //---------------------------------------------------
    // Result music
    //---------------------------------------------------
    // Use custom SNG19_JNG, adjust round clear length
    INSTALL_HOOK(Stage_SNG19_JNG_1);
    INSTALL_HOOK(Stage_SNG19_JNG_2);
    INSTALL_HOOK(Stage_CStateGoalFadeIn);

    //---------------------------------------------------
    // Perfect Chaos
    //---------------------------------------------------
    // Iblis final hit sfx & event movie
    WRITE_JUMP(0xC0FFC0, Stage_CBossPerfectChaosFinalHitSfx);
    WRITE_STRING(0x1587DD8, "ev704");
    INSTALL_HOOK(Stage_CBossPerfectChaosCStateDefeated);

    //---------------------------------------------------
    // HUD Music
    //---------------------------------------------------
    // Tutorial stop music
    INSTALL_HOOK(Stage_CTutorialImpl);

    // Shop don't change music
    WRITE_JUMP(0xD34984, (void*)0xD349E2);
    WRITE_JUMP(0xD32D4C, (void*)0xD32D8E);
}
