#include "CodeOption.h"
#include "Base/gstUtils.h"
#include "Camera/gstCamera.h"
#include <sstream> 

namespace camerasdk{

CCodeOption::CCodeOption()
  : m_codec(CODEC_H264)
  , m_bitrate(0){
  m_protocol = CODE_PROTOCOL_DEFAULT;
  m_location = CODE_LOCATION_DEFAULT;
  m_port = CODE_PORT_DEFAULT;
  m_with_source = false;
  m_debug_show = false;
}

CCodeOption::~CCodeOption(){
}

bool CCodeOption::BuildEncodeLaunchStr(std::string &strLaunch){
  std::ostringstream ss;
  // setup appsrc input element
  if (!m_with_source){
    // ss << "appsrc name=mysource is-live=true do-timestamp=true format=3 ! videoconvert !";
    ss << "appsrc name=mysource is-live=true format=3 ! videoconvert ! ";
  }
  else{
    gstCamera::MatchBestOption(m_cam_options);
    std::string capture;
    m_cam_options.BuildCaptureLaunchStr(capture);
    ss << capture;
  }

  // set default bitrate (if needed)
  if (m_bitrate == 0){
    m_bitrate = 2048;
    if(m_cam_options.m_video_scale && m_cam_options.m_scale_width < VIDEO_WIDTH_DEFAULT){
      m_bitrate = 600;
    }
    else if(!m_cam_options.m_video_scale && m_cam_options.m_width < VIDEO_WIDTH_DEFAULT){
      m_bitrate = 600;
    }
  }

  int iframe_interval = (int)m_cam_options.m_frame_rate;
  if(iframe_interval == 0){
    iframe_interval = 30;
  }

  //if(m_debug_show){
  //  ss << "tee name=t    t. ! queue ! videoconvert ! xvimagesink    t. ! queue ! ";
  //}

  // select hardware codec to use
  if (m_codec == CODEC_H264)
    ss << GST_ENCODER_H264; // TODO:  investigate quality-level setting
  else if (m_codec == CODEC_H265)
    ss << GST_ENCODER_H265;
  else{
    g_printerr("gstEncoder -- unsupported codec requested (%s)\n", CCameraOption::CodecToString(m_codec));
    g_printerr("              supported encoder codecs are:\n");
    g_printerr("                 * h264\n");
    g_printerr("                 * h265\n");
  }

#ifdef GST_CODECS_OMX
  // 硬编需要码流分析后再rtp转发，否则rtp转发会失败
  if (m_codec == CODEC_H264)
    ss << " bitrate=" << m_bitrate << " iframeinterval=" << iframe_interval  << " ! video/x-h264, stream-format=byte-stream ! h264parse ! ";
  else if (m_codec == CODEC_H265)
    ss << " bitrate=" << m_bitrate << " iframeinterval=" << iframe_interval  << " ! video/x-h265, stream-format=byte-stream ! h265parse ! ";
  else
    ss << " bitrate=" << m_bitrate << " ! ";
#else
  ss << " bitrate=" << m_bitrate << " key-int-max=" << iframe_interval << " tune=zerolatency ! ";
#endif

  if(m_debug_show){
    ss << "tee name=t    t. ! queue ! avdec_h264 ! videoconvert ! xvimagesink    t. ! queue ! ";
  }

  if (m_protocol == "rtp"){
    // rtp转发编码后的码流
    if (m_codec == CODEC_H264)
      ss << "rtph264pay";
    else if (m_codec == CODEC_H265)
      ss << "rtph265pay";
    else if (m_codec == CODEC_MJPEG)
      ss << "rtpjpegpay";

    if (m_codec == CODEC_H264 || m_codec == CODEC_H265){
      ss << " ! udpsink host=";
    }
    else{
      ss << " ! rtpsink address=";
    }
    ss << m_location << " ";

    if (m_port != 0)
      ss << "port=" << m_port;

    // ss << " auto-multicast=true";
  }

  strLaunch = ss.str();
  g_print("%s\n", strLaunch.c_str());
  return true;
}

}