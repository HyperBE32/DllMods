﻿#pragma once

#include <Sonic/Camera/Camera.h>
#include <Sonic/FxPipeline/FxSceneRenderer.h>
#include <Sonic/FxPipeline/FxScheduler.h>
#include <Sonic/FxPipeline/Jobs/FxJob.h>
#include <Sonic/FxPipeline/Jobs/FxJobBase.h>
#include <Sonic/FxPipeline/Jobs/AlternativeDepthOfField/FxAlternativeDepthOfField.h>
#include <Sonic/FxPipeline/Jobs/BloomGlare/FxBloomGlare.h>
#include <Sonic/FxPipeline/Jobs/ColorCorrection/FxColorCorrection.h>
#include <Sonic/FxPipeline/Jobs/CrossFade/FxCrossFade.h>
#include <Sonic/FxPipeline/Jobs/RenderGameScene/FxRenderGameScene.h>
#include <Sonic/FxPipeline/Jobs/RenderScene/FxRenderScene.h>
#include <Sonic/FxPipeline/Jobs/ShadowMap/FxShadowMap.h>
#include <Sonic/FxPipeline/Jobs/ToneMapping/FxToneMapping.h>
#include <Sonic/System/GameDocument.h>
#include <Sonic/System/GameObject.h>
#include <Sonic/System/InputState.h>
#include <Sonic/System/PadState.h>
#include <Sonic/System/LightManager/LightManager.h>
#include <Sonic/System/LightManager/LocalLight.h>
#include <Sonic/System/RenderDirector/RenderDirector.h>
#include <Sonic/System/RenderDirector/RenderDirectorFxPipeline.h>
#include <Sonic/Tool/EditParam/EditParam.h>
#include <Sonic/Tool/EditParam/ParamBase.h>
#include <Sonic/Tool/EditParam/ParamBool.h>
#include <Sonic/Tool/EditParam/ParamTypeList.h>
// #include <Sonic/Tool/EditParam/ParamValue.h>
#include <Sonic/Tool/ParameterEditor/AbstractParameter.h>
#include <Sonic/Tool/ParameterEditor/AbstractParameterNode.h>
#include <Sonic/Tool/ParameterEditor/ParameterCategory.h>
#include <Sonic/Tool/ParameterEditor/ParameterFile.h>
#include <Sonic/Tool/ParameterEditor/ParameterGroup.h>

// Lost World/Forces style namespace alias
namespace app = Sonic;