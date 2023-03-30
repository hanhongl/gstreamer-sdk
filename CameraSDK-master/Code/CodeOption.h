#ifndef CAMERASDK_CODEOPTION_H
#define CAMERASDK_CODEOPTION_H

#include "Camera/CameraOption.h"

namespace camerasdk{

#define CODE_PROTOCOL_DEFAULT "rtp"             // 默认输出协议
#define CODE_LOCATION_DEFAULT "127.0.0.1"       // 默认输出地址
static const int CODE_PORT_DEFAULT = 5022;      // 默认输出端口

class CCodeOption{
 public:
  CCodeOption();
  ~CCodeOption();

 public:
  /**
   * @brief 视频编码启动参数
   * @param strLaunch 输出启动字符串
   */
  bool BuildEncodeLaunchStr(std::string &strLaunch);

 public:
  //编码类型，默认值为CODEC_RAW
  Codec m_codec;

  /**
   * The encoding bitrate for compressed streams (only applies to video codecs like H264/H265)..
   * @note the default bitrate for encoding output streams is 4Mbps (target VBR).
   */
  uint32_t m_bitrate;

  /**
   * 编码后输出协议 (e.g. `file`, `rtp`, `rtmp`, ect)
   */
  std::string m_protocol;

  /**
   * 输出地址, IP地址或文件路径
   */
  std::string m_location;

  /**
   * 输出端口
   */
  int m_port;

  /**
   * @brief 摄像头配置参数
   */
  CCameraOption m_cam_options;

  /**
   * @brief true:采集视频，false：通过回调函数输入视频
   */
  bool m_with_source;

  //本地调试显示
  bool m_debug_show;
};

}

#endif