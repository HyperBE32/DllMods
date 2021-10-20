/*----------------------------------------------------------*/
//	Author: brianuuuSonic https://github.com/brianuuu
//	Year: 2021
//	Description: Apply stage specific codes
/*----------------------------------------------------------*/

#pragma once

class Stage
{
public:
	static void applyPatches();

	static bool m_wallJumpStart;
	static float m_wallJumpTime;
	static void __fastcall getIsWallJumpImpl(float* outOfControl);

	static bool m_waterRunning;

	static std::string m_lapTimeStr;
	static float m_checkpointTimer;
	static void draw();
};

