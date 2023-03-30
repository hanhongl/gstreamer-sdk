#ifndef CAMERASDK_GSTDISPLAY_H
#define CAMERASDK_GSTDISPLAY_H

#include "CameraSDKDefine.h"

namespace camerasdk{

class gstDisplay{
 public:
  gstDisplay();
  ~gstDisplay();

  static void CameraBufferCBFun(GstBuffer *gst_buffer, void *user_data);
  static void onNeedData(GstElement *pipeline, guint size, void *user_data);
  static void onEnoughData(GstElement *pipeline, void *user_data);

 public:
  void Open();
  void Close();
  void onBuffer(GstBuffer *gst_buffer);
  void Run();

 private:
  int Init();
  void Uninit();

 private:
  GstElement *pipeline_;
  GstElement *app_src_;
  GstElement *avdec_h264_;
  GstElement *video_convert_;
  GstElement *tee_;
  GstElement *disp_queue_;
  GstElement *xvimage_sink_;
  GstPad *tee_disp_pad_;
  GstPad *queue_disp_pad_;
  bool need_data_;
};

}

#endif