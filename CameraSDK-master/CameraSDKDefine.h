#ifndef CAMERASDKDEFINE_H
#define CAMERASDKDEFINE_H
#include <gst/gst.h>

// 导出函数定义
#define FUN_API extern "C"

// 码流回调函数
typedef void (CameraBufferCBFun)(GstBuffer* gstBuffer, void* user_data);

namespace camerasdk{
  // 公共对外的结构体定义放在这里
}

#endif