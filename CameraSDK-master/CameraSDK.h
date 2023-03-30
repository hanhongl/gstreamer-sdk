#ifndef CAMERASDK_H
#define CAMERASDK_H
#include "CameraSDKDefine.h"

/**
 * @brief 初始化模块
 * @return true 成功 
 */
FUN_API bool CameraSDK_Init();

/**
 * @brief 反初始化模块 
 */
FUN_API void CameraSDK_Uninit();


#endif