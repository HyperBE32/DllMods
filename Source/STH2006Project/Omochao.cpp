﻿#include "Omochao.h"
#include "Application.h"

HOOK(int, __fastcall, Omochao_MsgNotifyObjectEvent, 0x114FB60, void* This, void* Edx, uint32_t a2)
{
	uint32_t* pEvent = (uint32_t*)(a2 + 16);
	switch (*pEvent)
	{
	case 51:
	{
		// Elise specific dialogs
		if (!Common::IsPlayerSuper() && Common::CheckPlayerNodeExist("ch_princess01_elise"))
		{
			*pEvent = 6;
		}
		break;
	}
	case 52:
	{
		// Non-Elise specific dialogs
		if (Common::IsPlayerSuper() || !Common::CheckPlayerNodeExist("ch_princess01_elise"))
		{
			*pEvent = 6;
		}
		break;
	}
	default: break;
	}

	return originalOmochao_MsgNotifyObjectEvent(This, Edx, a2);
}

void Omochao::applyPatches()
{
	INSTALL_HOOK(Omochao_MsgNotifyObjectEvent);
}
