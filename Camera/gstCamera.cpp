#include "gstCamera.h"
#include "Base/gstUtils.h"
#include "Base/TimeUtil.h"
#include <gst/gst.h>
#include <gst/app/gstappsink.h>

namespace camerasdk{

gstCamera::gstCamera(const CCameraOption& option)
  : options_(option){
  pipeline_ = NULL;
  bus_ = NULL;
  app_sink_ = NULL;
  callback_fun_ = NULL;
  user_data_ = NULL;
}

gstCamera::~gstCamera(){
  Close();
  Uninit();
}

bool gstCamera::Open(){
  if (pipeline_ != NULL){
    gst_element_set_state(pipeline_, GST_STATE_PLAYING);
  }
  return true;
}

void gstCamera::Close(){
  if (pipeline_ != NULL){
    gst_element_set_state(pipeline_, GST_STATE_NULL);
  }
}

void gstCamera::SetBufferCBFun(CameraBufferCBFun* fun, void* user_data){
  callback_fun_ = fun;
  user_data_ = user_data;
}

void gstCamera::AddBufferCBFun(CameraBufferCBFun* fun, void* user_data){
  std::lock_guard<std::mutex> lock(cb_info_mtx_);
  for(std::vector<BufferCBFunInfo>::iterator it = cb_info_list.begin(); it != cb_info_list.end(); it++){
    if(it->fun == fun && it->user_data == user_data){
      return;
    }
  }
  BufferCBFunInfo cb_info;
  cb_info.fun = fun;
  cb_info.user_data = user_data;
  cb_info_list.push_back(cb_info);
}

void gstCamera::DelBufferCBFun(CameraBufferCBFun* fun, void* user_data){
  std::lock_guard<std::mutex> lock(cb_info_mtx_);
  for(std::vector<BufferCBFunInfo>::iterator it = cb_info_list.begin(); it != cb_info_list.end(); it++){
    if(it->fun == fun && it->user_data == user_data){
      cb_info_list.erase(it);
      break;
    }
  }
}

GstFlowReturn gstCamera::OnBuffer(_GstAppSink* sink, void* user_data){
  //g_print("gstCamera OnBuffer %s\n", GetCurTimeStr().c_str());
  if( !user_data )
    return GST_FLOW_OK;
		
  gstCamera* cam = (gstCamera*)user_data;
	
  cam->CheckBuffer();
  cam->CheckMsgBus();
	
  return GST_FLOW_OK;
}

// OnEOS
void gstCamera::OnEOS(_GstAppSink* sink, void* user_data){
  g_print("gstCamera -- end of stream (EOS)\n");
}

// OnPreroll
GstFlowReturn gstCamera::OnPreroll(_GstAppSink* sink, void* user_data){
  g_print("gstCamera -- OnPreroll\n");
  return GST_FLOW_OK;
}

gstCamera* gstCamera::Create(const CCameraOption& option){
  gstCamera* cam = new gstCamera(option);
  if (!cam)
    return NULL;
  // initialize camera (with fallback)
  if (!cam->Init()){
    g_printerr("gstCamera -- failed to create device %s\n", option.m_resource.c_str());
    return NULL;
  }
  g_print("gstCamera successfully created device %s\n", option.m_resource.c_str()); 
  return cam;
}

bool gstCamera::Init(){
  GError *err = NULL;
  g_print("gstCamera -- attempting to create device %s\n", options_.m_resource.c_str());

  // discover device stats
  if (!Discover(options_)){
    g_printerr("gstCamera -- device discovery failed\n");
  }

  // build pipeline string
  std::string strLaunch;
  if (!options_.BuildCameraLaunchStr(strLaunch)){
    g_printerr("gstCamera failed to build pipeline string\n");
    return false;
  }

  pipeline_ = gst_parse_launch(strLaunch.c_str(), &err);
  if (err != NULL){
    g_printerr("gstCamera failed to create pipeline\n");
    g_printerr("   (%s)\n", err->message);
    g_error_free(err);
    return false;
  }

  GstPipeline *pipeline = GST_PIPELINE(pipeline_);

  if (!pipeline){
    g_printerr("gstCamera failed to cast GstElement into GstPipeline\n");
    return false;
  }

  bus_ = gst_pipeline_get_bus(pipeline);

  if (!bus_){
    g_printerr("gstCamera failed to retrieve GstBus from pipeline\n");
    return false;
  }

  GstElement *appsinkElement = gst_bin_get_by_name(GST_BIN(pipeline), "mysink");
  GstAppSink *appsink = GST_APP_SINK(appsinkElement);

  if (!appsinkElement || !appsink){
    g_printerr("gstCamera failed to retrieve AppSink element from pipeline\n");
    return false;
  }

  app_sink_ = appsink;

  // setup callbacks
  GstAppSinkCallbacks cb;
  memset(&cb, 0, sizeof(GstAppSinkCallbacks));

  cb.eos = OnEOS;
  cb.new_preroll = OnPreroll;
  cb.new_sample = OnBuffer;
  gst_app_sink_set_callbacks(app_sink_, &cb, this, NULL);

  return true;
}
    
void gstCamera::Uninit()
{
  if (app_sink_ != NULL){
    gst_object_unref(app_sink_);
    app_sink_ = NULL;
  }

  if (bus_ != NULL){
    gst_object_unref(bus_);
    bus_ = NULL;
  }

  if (pipeline_ != NULL){
    gst_object_unref(pipeline_);
    pipeline_ = NULL;
  }
}

#define release_return { gst_sample_unref(gst_sample); return; }

void gstCamera::CheckBuffer(){
  if (!app_sink_)
    return;

  // block waiting for the buffer
  GstSample *gst_sample = gst_app_sink_pull_sample(app_sink_);
  if (!gst_sample){
    g_printerr("gstCamera -- app_sink_pull_sample() returned NULL...\n");
    return;
  }

  // retrieve sample caps
  GstCaps *gst_caps = gst_sample_get_caps(gst_sample);
  if (!gst_caps){
    g_printerr("gstCamera -- gst_sample had NULL caps...\n");
    release_return;
  }

  // retrieve the buffer from the sample
  GstBuffer *gst_buffer = gst_sample_get_buffer(gst_sample);
  if (!gst_buffer){
    g_printerr("gstCamera -- app_sink_pull_sample() returned NULL...\n");
    release_return;
  }

  //g_print("OnBuffer = %" GST_TIME_FORMAT "\n", GST_TIME_ARGS(gst_buffer->pts));

  // if( !mBufferManager->Enqueue(gst_buffer, gst_caps) )
  //	LogError(LOG_GSTREAMER "gstCamera -- failed to handle incoming buffer\n");
  if (callback_fun_ != NULL){
    (*callback_fun_)(gst_buffer, user_data_);
  }

  {
    std::lock_guard<std::mutex> lock(cb_info_mtx_);
    for(std::vector<BufferCBFunInfo>::iterator it = cb_info_list.begin(); it != cb_info_list.end(); it++){
      if(it->fun != NULL){
        (*it->fun)(gst_buffer, it->user_data);
      }
  }
  }

  release_return;
}

void gstCamera::CheckMsgBus(){
  while (true){
    GstMessage *msg = gst_bus_pop(bus_);
    if (!msg)
      break;

    // gst_message_print(bus_, msg, this);
    gst_message_unref(msg);
  }
}

bool gstCamera::MatchBestOption(CCameraOption& option){
  return Discover(option);
}

bool gstCamera::Discover(CCameraOption& option){
  // check desired frame sizes
  if (option.m_width == 0)
    option.m_width = VIDEO_WIDTH_DEFAULT;
  if (option.m_height == 0)
    option.m_height = VIDEO_HEIGHT_DEFAULT;
  if (option.m_frame_rate <= 0)
    option.m_frame_rate = VIDEO_FRAMERATE_DEFAULT;

  // create v4l2 device service
  GstDeviceProvider *device_provider = gst_device_provider_factory_get_by_name("v4l2deviceprovider");

  if (!device_provider){
    g_printerr("gstCamera -- failed to create v4l2 device provider during discovery\n");
    return false;
  }

  // get list of v4l2 devices
  GList *device_list = gst_device_provider_get_devices(device_provider);

  if (!device_list){
    g_printerr("gstCamera -- didn't discover any v4l2 devices\n");
    return false;
  }

  // find the requested /dev/video* device
  GstDevice *device = NULL;

  for (GList *n = device_list; n; n = n->next){
    GstDevice *d = GST_DEVICE(n->data);
    g_print("gstCamera -- found v4l2 device: %s\n", gst_device_get_display_name(d));
    GstStructure *properties = gst_device_get_properties(d);

    if (properties != NULL){
      g_print("%s\n", gst_structure_to_string(properties));
      const char *devicePath = gst_structure_get_string(properties, "device.path");

      if (devicePath != NULL && strcasecmp(devicePath, option.m_resource.c_str()) == 0){
        device = d;
        break;
      }
    }
  }

  if (!device){
    g_printerr("gstCamera -- could not find v4l2 device %s\n", option.m_resource.c_str());
    return false;
  }

  // get the caps of the device
  GstCaps *device_caps = gst_device_get_caps(device);

  if (!device_caps){
    g_printerr("gstCamera -- failed to retrieve caps for v4l2 device %s\n", option.m_resource.c_str());
    return false;
  }

  PrintCaps(device_caps, option);

  // pick the best caps
  if (!MatchCaps(device_caps, option))
    return false;

  g_print("gstCamera -- selected device profile:  codec=%s format=%s width=%u height=%u framerate=%f\n", \
    CCameraOption::CodecToString(option.m_codec), CCameraOption::ImageFormatToString(option.m_image_format), \
    option.m_width, option.m_height, option.m_frame_rate);

  return true;
}

bool gstCamera::MatchCaps( GstCaps* device_caps, CCameraOption& option ){
  const uint32_t num_caps = gst_caps_get_size(device_caps);
  GstStructure *best_caps = NULL;
  int best_resolution = 1000000;
  float best_framerate = 0.0f;
  Codec best_codec = CODEC_UNKNOWN;

  for (uint32_t n = 0; n < num_caps; n++){
    GstStructure *caps = gst_caps_get_structure(device_caps, n);

    if (!caps)
      continue;

    Codec codec;
    ImageFormat format;
    uint32_t width, height;
    float framerate;

    if (!ParseCaps(caps, &codec, &format, &width, &height, &framerate, option))
      continue;

    const int resolution_diff = abs(int(option.m_width) - int(width)) + abs(int(option.m_height) - int(height));

    // pick this one if the resolution is closer, or if the resolution is the same but the framerate is better
    // (or if the framerate is the same and previous codec was MJPEG, pick the new one because MJPEG isn't preferred)
    if (resolution_diff < best_resolution || (resolution_diff == best_resolution && (framerate > best_framerate || best_codec == CODEC_MJPEG))){
      best_resolution = resolution_diff;
      best_framerate = framerate;
      best_codec = codec;
      best_caps = caps;
    }

    // if( resolution_diff == 0 )
    //	break;
  }

  if (!best_caps){
    g_print("gstCamera -- couldn't find a compatible codec/format for v4l2 device %s\n", option.m_resource.c_str());
    return false;
  }

  if (!ParseCaps(best_caps, &option.m_codec, &option.m_image_format, &option.m_width, &option.m_height, &option.m_frame_rate, option))
    return false;

  return true;
}

bool gstCamera::PrintCaps(GstCaps* device_caps, CCameraOption& option){
  const uint32_t num_caps = gst_caps_get_size(device_caps);
  g_print("gstCamera -- found %u caps for v4l2 device %s\n", num_caps, option.m_resource.c_str());

  if (num_caps == 0)
    return false;

  for (uint32_t n = 0; n < num_caps; n++){
    GstStructure *caps = gst_caps_get_structure(device_caps, n);

    if (!caps)
      continue;

    g_print("[%u] %s\n", n, gst_structure_to_string(caps));
  }

  return true;
}

bool gstCamera::ParseCaps(GstStructure *caps, Codec *_codec, ImageFormat *_format, uint32_t *_width, uint32_t *_height, float *_frameRate, CCameraOption &option){
  // parse codec/format
  const Codec codec = GstParseCodec(caps);
  const ImageFormat format = GstParseFormat(caps);

  if (codec == CODEC_UNKNOWN)
    return false;

  if (codec == CODEC_RAW && format == IMAGE_UNKNOWN)
    return false;

  // if the user is requesting a codec, check that it matches
  if (option.m_codec != CODEC_UNKNOWN && option.m_codec != codec)
    return false;

  // get width/height
  int width = 0;
  int height = 0;

  if (!gst_structure_get_int(caps, "width", &width) ||
      !gst_structure_get_int(caps, "height", &height)){
    return false;
  }

  // get highest framerate
  float framerate = 0;
  int framerate_num = 0;
  int framerate_denom = 0;

  if (gst_structure_get_fraction(caps, "framerate", &framerate_num, &framerate_denom)){
    framerate = float(framerate_num) / float(framerate_denom);
  }
  else{
    // it's a list of framerates, pick the max
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
    g_print("gstCamera -- missing framerate in caps, ignoring\n");

  *_codec = codec;
  *_format = format;
  *_width = width;
  *_height = height;
  *_frameRate = framerate;

  return true;
}

}