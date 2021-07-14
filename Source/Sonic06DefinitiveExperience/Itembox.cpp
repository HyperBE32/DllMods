#include "Itembox.h"

float const c_10ringRadius = 0.57f;
float const c_1upRadius = 0.70f;

bool m_parsingSetdata = false;
HOOK(bool, __stdcall, ParseSetdata, 0xEB5050, void* a1, char** pFileName, void* a3, void* a4, uint8_t a5, uint8_t a6)
{
	m_parsingSetdata = true;
	bool result = originalParseSetdata(a1, pFileName, a3, a4, a5, a6);
	m_parsingSetdata = false;

	return result;
}

HOOK(uint32_t*, __fastcall, ReadXmlData, 0xCE5FC0, uint32_t size, char* pData, void* a3, uint32_t* a4)
{
    // Get all 1up and 10ring objects, add cmn_itembox_lock at the same position
    std::string injectStr;
	if (!m_parsingSetdata || Itembox::getInjectStr(pData, size, injectStr) != tinyxml2::XML_SUCCESS || injectStr.empty())
	{
		return originalReadXmlData(size, pData, a3, a4);
	}

    const size_t newSize = size + injectStr.size();
    const std::unique_ptr<char[]> pBuffer = std::make_unique<char[]>(newSize);
    memcpy(pBuffer.get(), pData, size);

    char* pInsertionPos = strstr(pBuffer.get(), "</SetObject>");

    memmove(pInsertionPos + injectStr.size(), pInsertionPos, size - (size_t)(pInsertionPos - pBuffer.get()));
    memcpy(pInsertionPos, injectStr.c_str(), injectStr.size());

    return originalReadXmlData(newSize, pBuffer.get(), a3, a4);
}

const char* volatile const ObjectProductionItemboxLock = "ObjectProductionItemboxLock.phy.xml";
uint32_t LoadItemboxLockAsmHookReturnAddress = 0xD45FAA;
uint32_t sub_EA0450 = 0xEA0450;
void __declspec(naked) LoadItemboxLockAsmHook()
{
	__asm
	{
		call	[CStringDestructor]
		push    ObjectProductionItemboxLock
		lea     ecx, [esp + 0x10]
		call	[CStringConstructor]
		lea     ecx, [esp + 0xC]
		push    ecx
		push    esi
		call	[sub_EA0450]
		lea     ecx, [esp + 0xC]
		call	[CStringDestructor]
		jmp		[LoadItemboxLockAsmHookReturnAddress]
	}
}

// Have to use ASM to prevent double playing sfx
uint32_t sub_D5FD10 = 0xD5FD10;
uint32_t objItemPlaySfxReturnAddress = 0xFFF9AF;
void __declspec(naked) objItemPlaySfx()
{
	__asm
	{
		call	[sub_D5FD10]
		call	Itembox::playItemboxSfx
		jmp		[objItemPlaySfxReturnAddress]
	}
}

void Itembox::applyPatches()
{
	// Play itembox sfx for 1up and 10ring
	WRITE_MEMORY(0x011F2FE0, uint32_t, 4002032);
	WRITE_JUMP(0xFFF9AA, objItemPlaySfx);

	// Set itembox radius
	WRITE_MEMORY(0x11F3353, float*, &c_10ringRadius);
	WRITE_MEMORY(0xFFF9E8, float*, &c_1upRadius);

	// Load itembox lock-on object physics
	WRITE_JUMP(0xD45FA5, LoadItemboxLockAsmHook);

	// Inject lock-on object
	INSTALL_HOOK(ParseSetdata);
	INSTALL_HOOK(ReadXmlData);
}

void Itembox::playItemboxSfx()
{
	static SharedPtrTypeless soundHandle;
	Common::SonicContextPlaySound(soundHandle, 4002032, 0);
}

tinyxml2::XMLError Itembox::getInjectStr(char const* pData, uint32_t size, std::string& injectStr)
{
	injectStr.clear();

	tinyxml2::XMLDocument pathXml;
	tinyxml2::XMLError result = pathXml.Parse(pData, size);
	XMLCheckResult(result);

	tinyxml2::XMLNode* pRoot = pathXml.FirstChildElement("SetObject");
	if (pRoot == nullptr) return tinyxml2::XML_ERROR_FILE_READ_ERROR;

	std::set<uint32_t> setObjectIDs;
	ItemboxData itemboxData;
	for (tinyxml2::XMLElement* pObjectElement = pRoot->FirstChildElement(); pObjectElement != nullptr; pObjectElement = pObjectElement->NextSiblingElement())
	{
		// Grab all the object ID to check if we have already injected this code
		tinyxml2::XMLElement* pSetObjectIDElement = pObjectElement->FirstChildElement("SetObjectID");
		if (pSetObjectIDElement)
		{
			uint32_t setObjectID = std::stoul(pSetObjectIDElement->GetText());
			setObjectIDs.insert(setObjectID);

			std::string objName = pObjectElement->Name();
			if (objName != "Item" && objName != "SuperRing") continue;

			PositionStr positionStr;
			tinyxml2::XMLElement* pPositionElement = pObjectElement->FirstChildElement("Position");
			positionStr.x = pPositionElement->FirstChildElement("x")->GetText();
			positionStr.y = pPositionElement->FirstChildElement("y")->GetText();
			positionStr.z = pPositionElement->FirstChildElement("z")->GetText();

			itemboxData[setObjectID] = positionStr;
		}
	}

	for (auto const& iter : itemboxData)
	{
		uint32_t const newSetObjectID = iter.first * 100;
		PositionStr const& positionStr = iter.second;

		// Only inject if ID not exist already
		if (setObjectIDs.find(newSetObjectID) != setObjectIDs.end())
		{
			printf("[Itembox] cmn_itembox_lock already injected!\n");
			continue;
		}

		printf("[Itembox] Injecting cmn_itembox_lock at pos: %s, %s, %s\n", positionStr.x.c_str(), positionStr.y.c_str(), positionStr.z.c_str());
		injectStr += "<ObjectPhysics>\n";
		injectStr += "    <AddRange>0</AddRange>\n";
		injectStr += "    <CullingRange>15</CullingRange>\n";
		injectStr += "    <DebrisTarget>\n";
		injectStr += "        <x>0</x>\n";
		injectStr += "        <y>0</y>\n";
		injectStr += "        <z>0</z>\n";
		injectStr += "    </DebrisTarget>\n";
		injectStr += "    <GroundOffset>0</GroundOffset>\n";
		injectStr += "    <IsCastShadow>true</IsCastShadow>\n";
		injectStr += "    <IsDynamic>false</IsDynamic>\n";
		injectStr += "    <IsReset>false</IsReset>\n";
		injectStr += "    <Range>100</Range>\n";
		injectStr += "    <SetObjectID>" + std::to_string(newSetObjectID) + "</SetObjectID>\n";
		injectStr += "    <Type>cmn_itembox_lock</Type>\n";
		injectStr += "    <WrappedObjectID>\n";
		injectStr += "        <SetObjectID>0</SetObjectID>\n";
		injectStr += "    </WrappedObjectID>\n";
		injectStr += "    <Position>\n";
		injectStr += "        <x>" + positionStr.x + "</x>\n";
		injectStr += "        <y>" + positionStr.y + "</y>\n";
		injectStr += "        <z>" + positionStr.z + "</z>\n";
		injectStr += "    </Position>\n";
		injectStr += "    <Rotation>\n";
		injectStr += "        <x>0</x>";
		injectStr += "        <y>0</y>";
		injectStr += "        <z>0</z>";
		injectStr += "        <w>1</w>";
		injectStr += "    </Rotation>\n";
		injectStr += "</ObjectPhysics>\n";
	}

	return tinyxml2::XML_SUCCESS;
}
