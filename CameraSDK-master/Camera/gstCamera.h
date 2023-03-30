#ifndef CAMERASDK_GSTCAMERA_H
#define CAMERASDK_GSTCAMERA_H

#include "CameraSDKDefine.h"
#include "CameraOption.h"
#include <vector>
#include <mutex>

struct _GstAppSink;

namespace camerasdk{

struct BufferCBFunInfo
{
  BufferCBFunInfo(){
    fun = NULL;
    user_data = NULL;
  }
  CameraBufferCBFun* fun;
  void* user_data;
};


class gstCamera{
 public:
   gstCamera(const CCameraOption& option);
   ~gstCamera();

 public:
   static gstCamera* Create(const CCameraOption& option);

   /**
    * @brief 匹配最佳的摄像头采集参数
    * @param option 采集参数
    * @return true 成功
    */
   static bool MatchBestOption(CCameraOption& option);

 public:
   /**
    * @brief 开始采集
    * @return true 成功
    */
   bool Open();

  /**
   * @brief 停止采集
   */
  void Close();

  /**
   * @brief 设置码流回调函数
   * @param fun 码流回调函数
   * @param user_data 用户数据
   */
  void SetBufferCBFun(CameraBufferCBFun* fun, void* user_data);
  void AddBufferCBFun(CameraBufferCBFun* fun, void* user_data);
  void DelBufferCBFun(CameraBufferCBFun* fun, void* user_data);

 private:
   bool Init();
   void Uninit();
   void CheckMsgBus();
   void CheckBuffer();

 private:
   static bool Discover(CCameraOption& option);
   static bool MatchCaps(GstCaps* caps, CCameraOption& option);
   static bool PrintCaps(GstCaps* caps, CCameraOption& option);
   static bool ParseCaps(GstStructure* caps, Codec* codec, ImageFormat* format, uint32_t* width, uint32_t* height, float* frameRate, CCameraOption& option);

 private:
   static void OnEOS(_GstAppSink* sink, void* user_data);
   static GstFlowReturn OnPreroll(_GstAppSink* sink, void* user_data);
   static GstFlowReturn OnBuffer(_GstAppSink* sink, void* user_data);

 private:
   CCameraOption       options_;
   GstElement*         pipeline_;
   GstBus*             bus_;
   _GstAppSink*        app_sink_;
   CameraBufferCBFun*  callback_fun_;
   void*               user_data_;

   std::mutex cb_info_mtx_;
   std::vector<BufferCBFunInfo> cb_info_list;


};

}

#endif