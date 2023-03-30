#ifndef CAMERASDK_CAMERAOPTION_H
#define CAMERASDK_CAMERAOPTION_H

#include "CameraSDKDefine.h"
#include <string>

namespace camerasdk{

#define VIDEO_DEFAULT "/dev/video0"             // 默认摄像头路径
static const int VIDEO_WIDTH_DEFAULT = 1280;    // 默认分辨率宽度
static const int VIDEO_HEIGHT_DEFAULT = 720;    // 默认分辨率高度
static const int VIDEO_FRAMERATE_DEFAULT = 30;  // 默认帧率

/**
 * @brief 视频编码类型
 */
enum Codec{
  CODEC_UNKNOWN = 0,  //未知
  CODEC_RAW,          //未压缩 (e.g. RGB) 
  CODEC_H264,         //H.264
  CODEC_H265,         //H.265
  CODEC_MPEG2,        //MPEG2 (decode only)
  CODEC_MPEG4,        //MPEG4 (decode only)
  CODEC_MJPEG         //MJPEG
};

/**
 * @brief 图片格式
 */
enum ImageFormat{
  // RGB
  IMAGE_RGB8=0,               /**< uchar3 RGB8    (`'rgb8'`) */
  IMAGE_RGBA8,                /**< uchar4 RGBA8   (`'rgba8'`) */
  IMAGE_RGB32F,               /**< float3 RGB32F  (`'rgb32f'`) */
  IMAGE_RGBA32F,              /**< float4 RGBA32F (`'rgba32f'`) */

  // BGR
  IMAGE_BGR8,                 /**< uchar3 BGR8    (`'bgr8'`) */
  IMAGE_BGRA8,                /**< uchar4 BGRA8   (`'bgra8'`) */
  IMAGE_BGR32F,               /**< float3 BGR32F  (`'bgr32f'`) */
  IMAGE_BGRA32F,              /**< float4 BGRA32F (`'bgra32f'`) */
	
  // YUV
  IMAGE_YUYV,                 /**< YUV YUYV 4:2:2 packed (`'yuyv'`) */
  IMAGE_YUY2=IMAGE_YUYV,		  /**< Duplicate of YUYV     (`'yuy2'`) */
  IMAGE_YVYU,                 /**< YUV YVYU 4:2:2 packed (`'yvyu'`) */
  IMAGE_UYVY,                 /**< YUV UYVY 4:2:2 packed (`'uyvy'`) */
  IMAGE_I420,                 /**< YUV I420 4:2:0 planar (`'i420'`) */
  IMAGE_YV12,                 /**< YUV YV12 4:2:0 planar (`'yv12'`) */
  IMAGE_NV12,                 /**< YUV NV12 4:2:0 planar (`'nv12'`) */
	
  // Bayer
  IMAGE_BAYER_BGGR,           /**< 8-bit Bayer BGGR (`'bayer-bggr'`) */
  IMAGE_BAYER_GBRG,           /**< 8-bit Bayer GBRG (`'bayer-gbrg'`) */
  IMAGE_BAYER_GRBG,           /**< 8-bit Bayer GRBG (`'bayer-grbg'`) */
  IMAGE_BAYER_RGGB,           /**< 8-bit Bayer RGGB (`'bayer-rggb'`) */
	
  // grayscale
  IMAGE_GRAY8,                /**< uint8 grayscale  (`'gray8'`)   */
  IMAGE_GRAY32F,              /**< float grayscale  (`'gray32f'`) */

  // extras
  IMAGE_COUNT,                /**< The number of image formats */
  IMAGE_UNKNOWN=999,          /**< Unknown/undefined format */
  IMAGE_DEFAULT=IMAGE_RGBA32F /**< Default format (IMAGE_RGBA32F) */
};

class CCameraOption{
 public:
  CCameraOption();
  ~CCameraOption();

 public:
  /**
   * @brief 根据配置参数生成摄像头采集命令行字符串
   * @param strLaunch 输出命令行字符串
   * @return true 成功
   */
  bool BuildCameraLaunchStr(std::string& strLaunch);
  // 根据配置参数生成摄像头采集命令行字符串,不包含码流回调
  bool BuildCaptureLaunchStr(std::string& strLaunch);

 public:
  /**
   * @brief 编码类型枚举值转化成字符串
   * @param codec 编码类型枚举值
   * @return const char* 编码类型字符串
   */
  static const char* CodecToString(Codec codec);

  /**
   * @brief 图像格式枚举值转化成字符串
   * @param format 图像格式枚举值
   * @return const char* 图像格式字符串
   */
  static const char* ImageFormatToString(ImageFormat format);

 public:
  //摄像头URI，默认值为VIDEO_DEFAULT
  std::string m_resource;

  //码流宽度，默认值为VIDEO_WIDTH_DEFAULT
  uint32_t m_width;
	
  //码流高度，默认值为VIDEO_HEIGHT_DEFAULT
  uint32_t m_height;	

  //帧率，默认值为VIDEO_FRAMERATE_DEFAULT
  float m_frame_rate;

  //编码类型，默认值为CODEC_RAW
  Codec m_codec;

  //图像格式，默认值为IMAGE_YUY2
  ImageFormat  m_image_format;

  //是否叠加时间
  bool  m_time_overlay;

  //是否缩放视频
  bool m_video_scale;

  //缩放视频宽度，默认值为VIDEO_WIDTH_DEFAULT
  uint32_t m_scale_width;

  //缩放视频高度，默认值为VIDEO_HEIGHT_DEFAULT
  uint32_t m_scale_height;

};

}

#endif