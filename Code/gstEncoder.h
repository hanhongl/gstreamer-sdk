#ifndef CAMERASDK_GSTENCODER_H
#define CAMERASDK_GSTENCODER_H

#include "CameraSDKDefine.h"
#include "CodeOption.h"

namespace camerasdk{

class gstEncoder{
 public:
  gstEncoder(const CCodeOption &option);
  ~gstEncoder();

 public:
  static gstEncoder *Create(const CCodeOption &option);
  
  /**
  * @brief 视频采集码流回调函数
  * @param gst_buffer 码流数据
  * @param user_data  用户数据
  */
  static void CameraBufferCBFun(GstBuffer *gst_buffer, void *user_data);

 public:
  /**
   * @brief 开始编码
   * @return true 成功
   */
  bool Open();

  /**
   * @brief 停止编码
   */
  void Close();

  /**
   * @brief 编码视频帧数据
   * @param gst_buffer 码流数据
   * @return true 成功 
   */
  bool Render(GstBuffer *gst_buffer);

 private:
  bool Init();
  void Uninit();
  void checkMsgBus();

 private:
  static void onNeedData(GstElement *pipeline, guint size, void *user_data);
  static void onEnoughData(GstElement *pipeline, void *user_data);

 private:
  CCodeOption options_;
  GstElement *pipeline_;
  GstBus *bus_;
  GstElement *app_src_;
  bool streaming_;
  bool need_data_;
};

}

#endif