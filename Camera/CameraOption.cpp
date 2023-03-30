#include "CameraOption.h"
#include "Base/gstUtils.h"
#include <sstream> 

namespace camerasdk{

CCameraOption::CCameraOption()
  : m_resource(VIDEO_DEFAULT)
  , m_width(VIDEO_WIDTH_DEFAULT)
  , m_height(VIDEO_HEIGHT_DEFAULT)
  , m_frame_rate(VIDEO_FRAMERATE_DEFAULT)
  , m_codec(CODEC_RAW)
  , m_image_format(IMAGE_YUY2)
  , m_time_overlay(true)
  , m_video_scale(false)
  , m_scale_width(VIDEO_WIDTH_DEFAULT)
  , m_scale_height(VIDEO_HEIGHT_DEFAULT){
}
    
CCameraOption::~CCameraOption(){
}

bool CCameraOption::BuildCaptureLaunchStr(std::string& strLaunch){
  std::ostringstream ss;

  //ss << "v4l2src device=" << m_resource << " do-timestamp=true ! ";
  ss << "v4l2src device=" << m_resource << " ! ";

  if (m_codec != CODEC_UNKNOWN){
    ss << CodecToString(m_codec) << ", ";
    if (m_codec == CODEC_RAW){
      ss << "format=(string)" << ImageFormatToString(m_image_format) << ", ";
    }
    ss << "width=(int)" << m_width << ", height=(int)" << m_height << ", framerate=" << (int)m_frame_rate << "/1 ! ";
    if(m_video_scale){
      ss << "videoconvert ! ";
      ss << "videoscale ! " << CodecToString(m_codec) << ", ";
      if (m_codec == CODEC_RAW){
        ss << "format=(string)" << ImageFormatToString(m_image_format) << ", ";
      }
      ss << "width=(int)" << m_scale_width << ", height=(int)" << m_scale_height;
      ss << ", framerate=" << (int)m_frame_rate << "/1, pixel-aspect-ratio=1/1 ! ";
    }
    ss << "videoconvert ! ";
  }

  if (m_time_overlay){
    // ss << "clockoverlay time-format=\"%Y/%m/%d %H:%M:%S\" ! ";
    ss << "timeoverlay ! ";
  }
  strLaunch = ss.str();
}

bool CCameraOption::BuildCameraLaunchStr(std::string& strLaunch){
  std::ostringstream ss;
  std::string capture;
  BuildCaptureLaunchStr(capture);

  ss << "appsink name=mysink";
  strLaunch = capture + ss.str();
  g_print("%s\n", strLaunch.c_str());

  return true;
}

const char* CCameraOption::CodecToString(Codec codec){
  switch (codec){
    case CODEC_RAW:
      return "video/x-raw";
    case CODEC_H264:
      return "video/x-h264";
    case CODEC_H265:
      return "video/x-h265";
  }
  return " ";
}

const char* CCameraOption::ImageFormatToString(ImageFormat format){
  switch(format){
    case IMAGE_RGB8:	return "RGB";
    case IMAGE_YUY2:	return "YUY2";
    case IMAGE_I420:	return "I420";
    case IMAGE_NV12:	return "NV12";
    case IMAGE_YV12:	return "YV12";
    case IMAGE_YVYU:	return "YVYU";
    case IMAGE_UYVY:	return "UYVY";
    case IMAGE_BAYER_BGGR:	return "bggr";
    case IMAGE_BAYER_GBRG:	return "gbrg";
    case IMAGE_BAYER_GRBG:	return "grbg";
    case IMAGE_BAYER_RGGB:	return "rggb";
  }
	
  return " ";
}

}