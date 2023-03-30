#include "gstUtils.h"

namespace camerasdk{

Codec GstParseCodec(GstStructure *caps){
  const char *codec = gst_structure_get_name(caps);

  if (!codec)
    return CODEC_UNKNOWN;

  if (strcasecmp(codec, "video/x-raw") == 0 || strcasecmp(codec, "video/x-bayer") == 0)
    return CODEC_RAW;
  else if (strcasecmp(codec, "video/x-h264") == 0)
    return CODEC_H264;
  else if (strcasecmp(codec, "video/x-h265") == 0)
    return CODEC_H265;
  else if (strcasecmp(codec, "image/jpeg") == 0)
    return CODEC_MJPEG;
  else if (strcasecmp(codec, "video/mpeg") == 0){
    int mpeg_version = 0;

    if (!gst_structure_get_int(caps, "mpegversion", &mpeg_version)){
      g_printerr("MPEG codec, but failed to get MPEG version from caps\n");
      return CODEC_UNKNOWN;
    }

    if (mpeg_version == 2)
      return CODEC_MPEG2;
    else if (mpeg_version == 4)
      return CODEC_MPEG4;
    else{
      g_printerr("invalid MPEG codec version:  %i (MPEG-2 and MPEG-4 are supported)\n", mpeg_version);
      return CODEC_UNKNOWN;
    }
  }

  g_printerr("unrecognized codec - %s\n", codec);
  return CODEC_UNKNOWN;
}

ImageFormat GstParseFormat(GstStructure *caps){
  const char *format = gst_structure_get_string(caps, "format");

  if (!format)
    return IMAGE_UNKNOWN;

  if (strcasecmp(format, "rgb") == 0)
    return IMAGE_RGB8;
  else if (strcasecmp(format, "yuy2") == 0)
    return IMAGE_YUY2;
  else if (strcasecmp(format, "i420") == 0)
    return IMAGE_I420;
  else if (strcasecmp(format, "nv12") == 0)
    return IMAGE_NV12;
  else if (strcasecmp(format, "yv12") == 0)
    return IMAGE_YV12;
  else if (strcasecmp(format, "yuyv") == 0)
    return IMAGE_YUYV;
  else if (strcasecmp(format, "yvyu") == 0)
    return IMAGE_YVYU;
  else if (strcasecmp(format, "uyvy") == 0)
    return IMAGE_UYVY;
  else if (strcasecmp(format, "bggr") == 0)
    return IMAGE_BAYER_BGGR;
  else if (strcasecmp(format, "gbrg") == 0)
    return IMAGE_BAYER_GBRG;
  else if (strcasecmp(format, "grgb") == 0)
    return IMAGE_BAYER_GRBG;
  else if (strcasecmp(format, "rggb") == 0)
    return IMAGE_BAYER_RGGB;

  return IMAGE_UNKNOWN;
}

bool GetCameraCaps(GstDevice *device, CameraInfo& cam_info){
  // 获取摄像头能力列表
  GstCaps *device_caps = gst_device_get_caps(device);

  if (!device_caps){
    g_printerr("failed to retrieve caps for v4l2 device %s\n", cam_info.path.c_str());
    return false;
  }

  const uint32_t num_caps = gst_caps_get_size(device_caps);

  for (uint32_t n = 0; n < num_caps; n++){
    GstStructure *caps = gst_caps_get_structure(device_caps, n);

    if (!caps)
      continue;

    CameraCapInfo cap_info;

    cap_info.codec = GstParseCodec(caps);
    cap_info.format = GstParseFormat(caps);
    gst_structure_get_int(caps, "width", &cap_info.width);
    gst_structure_get_int(caps, "height", &cap_info.height);

    // 获取最大帧率
    float framerate = 0;
    int framerate_num = 0;
    int framerate_denom = 0;

    if (gst_structure_get_fraction(caps, "framerate", &framerate_num, &framerate_denom)){
      framerate = float(framerate_num) / float(framerate_denom);
    }
    else{
      // 支持多个帧率时，选择最大的
      GValueArray *framerate_list = NULL;

      if (gst_structure_get_list(caps, "framerate", &framerate_list) && framerate_list->n_values > 0){
        for (uint32_t n = 0; n < framerate_list->n_values; n++){
          GValue *value = framerate_list->values + n;

          if (GST_VALUE_HOLDS_FRACTION(value)){
            framerate_num = gst_value_get_fraction_numerator(value);
            framerate_denom = gst_value_get_fraction_denominator(value);

            if (framerate_num > 0 && framerate_denom > 0){
              const float rate = float(framerate_num) / float(framerate_denom);

              if (rate > framerate)
                framerate = rate;
            }
          }
        }
      }
    }

    if (framerate <= 0.0f)
      g_print("missing framerate in caps, ignoring\n");

    cap_info.framerate = framerate;
    cam_info.caps.push_back(cap_info);
  }
  return true;
}

bool GetAllCamera(CameraInfoList& camera_list){
  // 创建v4l2设备服务
  GstDeviceProvider *device_provider = gst_device_provider_factory_get_by_name("v4l2deviceprovider");

  if (!device_provider){
    g_printerr("failed to create v4l2 device provider during discovery\n");
    return false;
  }

  // 获取v4l2设备列表
  GList *device_list = gst_device_provider_get_devices(device_provider);

  if (!device_list){
    g_printerr("didn't discover any v4l2 devices\n");
    return false;
  }

  // 遍历 /dev/video* 摄像头
  for (GList *n = device_list; n; n = n->next){
    GstDevice *device = GST_DEVICE(n->data);
    //g_print("found v4l2 device: %s\n", gst_device_get_display_name(device));
    GstStructure *properties = gst_device_get_properties(device);

    if (properties != NULL){
      //g_print("%s\n", gst_structure_to_string(properties));
      const char *devicePath = gst_structure_get_string(properties, "device.path");

      if (devicePath != NULL){
        CameraInfo cam_info;
        cam_info.path = devicePath;
        GetCameraCaps(device, cam_info);
        camera_list.push_back(cam_info);
      }
    }
  }
  return true;
}

}