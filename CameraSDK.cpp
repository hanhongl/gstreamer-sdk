#include "CameraSDK.h"
#include <gst/gst.h>

FUN_API bool CameraSDK_Init(){
  // 初始化标志，防止重复初始化
  static bool gstreamer_initialized = false;

  if (gstreamer_initialized)
    return true;

  int argc = 0;

  // 初始化GStreamer
  if (!gst_init_check(&argc, NULL, NULL)){
    g_printerr("failed to initialize gstreamer library with gst_init()\n");
    return false;
  }

  gstreamer_initialized = true;

  // 获取GStreamer版本号并打印
  guint ver[] = {0, 0, 0, 0};
  gst_version(&ver[0], &ver[1], &ver[2], &ver[3]);
  g_print("initialized gstreamer, version %u.%u.%u.%u\n", ver[0], ver[1], ver[2], ver[3]);

  return true;
}

FUN_API void CameraSDK_Uninit(){
  // 反初始化GStreamer
  gst_deinit();
}