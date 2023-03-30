#ifndef CAMERASDK_GSTUTILS_H
#define CAMERASDK_GSTUTILS_H

#include "CameraSDKDefine.h"
#include "Camera/CameraOption.h"
#include <vector>

#if defined  __aarch64__
// 使用硬件加速编编解码
#define GST_CODECS_OMX
#endif

// 对720P分辨率使用H265编解码
//#define CODE_720P_H265 1

// 定义编码和解码插件
#ifdef GST_CODECS_OMX
#if GST_CHECK_VERSION(1,0,0)
// Decoders for JetPack <= 4 and GStreamer >= 1.0
 #define GST_DECODER_H264  "omxh264dec"
 #define GST_DECODER_H265  "omxh265dec"
// Encoders for JetPack <= 4 and GStreamer >= 1.0
#define GST_ENCODER_H264  "omxh264enc"
#define GST_ENCODER_H265  "omxh265enc"
#else   
// Decoders for JetPack <= 4 and GStreamer < 1.0
#define GST_DECODER_H264  "nv_omx_h264dec"
 #define GST_DECODER_H265  "nv_omx_h265dec"
// Encoders for JetPack <= 4 and GStreamer < 1.0
#define GST_ENCODER_H264  "nv_omx_h264enc"
#define GST_ENCODER_H265  "nv_omx_h265enc"
#endif  //GST_CHECK_VERSION
#elif defined GST_CODECS_NV
#define GST_DECODER_H264  "nvh264dec"
#define GST_DECODER_H265  "nvh265dec"
#define GST_ENCODER_H264  "nvh264enc"
#define GST_ENCODER_H265  "nvh265enc"
#else
#define GST_DECODER_H264  "avdec_h264"
#define GST_DECODER_H265  "avdec_h265"
#define GST_ENCODER_H264  "x264enc"
#define GST_ENCODER_H265  "x265enc"
#endif  //GST_CODECS_OMX

namespace camerasdk{
  
// 摄像头能力信息
typedef struct{
  Codec codec;            // 编码类型
  ImageFormat format;     // 图像格式
  int width;              // 分辨率宽
  int height;             // 分辨率高
  float framerate;        // 帧率
}CameraCapInfo;

// 摄像头能力信息列表
typedef std::vector<CameraCapInfo> CameraCapInfoList;

// 摄像头信息
typedef struct
{
  std::string path;         // 摄像头路径,例如"/dev/video0"
  CameraCapInfoList caps;
}CameraInfo;

// 摄像头信息列表
typedef std::vector<CameraInfo> CameraInfoList;

  /** 
   * @brief 解析摄像头编码类型
   * @param caps 摄像头参数集
   * @return Codec 编码类型
   */
  Codec GstParseCodec(GstStructure* caps);
 
  /**
   * @brief 解析摄像头图片格式
   * @param caps 摄像头参数集
   * @return ImageFormat 图片格式
   */
  ImageFormat GstParseFormat(GstStructure* caps);

  /**
   * @brief 获取摄像头能力信息
   * @param device 摄像头设备
   * @param cam_info 摄像头信息
   * @return true 成功
   */
  bool GetCameraCaps(GstDevice *device, CameraInfo& cam_info);

  /**
   * @brief 获取所有摄像头信息
   * @param camera_list 摄像头信息列表
   * @return true 成功
   */
  bool GetAllCamera(CameraInfoList& camera_list);

}

#endif  //CAMERASDK_GSTUTILS_H