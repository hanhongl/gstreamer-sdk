#include "gstEncoder.h"
#include <gst/app/gstappsrc.h>
#include <unistd.h>

namespace camerasdk{

gstEncoder *gstEncoder::Create(const CCodeOption &option){
  gstEncoder *enc = new gstEncoder(option);

  if (!enc)
    return NULL;

  if (!enc->Init()){
    g_printerr("gstEncoder -- failed to create encoder engine\n");
    return NULL;
  }

  return enc;
}

void gstEncoder::CameraBufferCBFun(GstBuffer *gst_buffer, void *user_data){
  gstEncoder *dec = (gstEncoder *)user_data;
  dec->Render(gst_buffer);
}

void gstEncoder::onNeedData(GstElement *pipeline, guint size, void *user_data){
  if (!user_data)
    return;

  gstEncoder *enc = (gstEncoder *)user_data;
  enc->need_data_ = true;
}

void gstEncoder::onEnoughData(GstElement *pipeline, void *user_data){
  if (!user_data)
    return;

  gstEncoder *enc = (gstEncoder *)user_data;
  enc->need_data_ = false;
}

gstEncoder::gstEncoder(const CCodeOption &option)
  : options_(option){
  app_src_ = NULL;
  bus_ = NULL;
  pipeline_ = NULL;
  need_data_ = false;
  streaming_ = false;
}

gstEncoder::~gstEncoder(){
  Close();
  Uninit();
}

bool gstEncoder::Open(){
  if (streaming_)
    return true;

  // transition pipline to STATE_PLAYING
  g_print("gstEncoder-- starting pipeline, transitioning to GST_STATE_PLAYING\n");

  const GstStateChangeReturn result = gst_element_set_state(pipeline_, GST_STATE_PLAYING);

  if (result == GST_STATE_CHANGE_ASYNC){
  }
  else if (result != GST_STATE_CHANGE_SUCCESS){
    g_printerr("gstEncoder -- failed to set pipeline state to PLAYING (error %u)\n", result);
    return false;
  }

  streaming_ = true;
  return true;
}

void gstEncoder::Close(){
  if (!streaming_)
    return;

  // send EOS
  need_data_ = false;

  g_print("gstEncoder -- shutting down pipeline, sending EOS\n");
  if (app_src_ != NULL){
    GstFlowReturn eos_result = gst_app_src_end_of_stream(GST_APP_SRC(app_src_));

    if (eos_result != 0)
      g_printerr("gstEncoder -- failed sending appsrc EOS (result %u)\n", eos_result);

    sleep(1);
  }

  // stop pipeline
  g_print("gstEncoder -- transitioning pipeline to GST_STATE_NULL\n");

  const GstStateChangeReturn result = gst_element_set_state(pipeline_, GST_STATE_NULL);

  if (result != GST_STATE_CHANGE_SUCCESS)
    g_printerr("gstEncoder -- failed to set pipeline state to NULL (error %u)\n", result);

  sleep(1);
  streaming_ = false;
  g_print("gstEncoder -- pipeline stopped\n");
}

bool gstEncoder::Render(GstBuffer *gst_buffer)
{
  if (app_src_ == NULL){
    return false;
  }
  // confirm the stream is open
  if (!streaming_){
    if (!Open())
      return false;
  }

  // check to see if data can be accepted
  if (!need_data_){
    g_print("gstEncoder -- pipeline full, skipping frame\n");
    return true;
  }
  GstFlowReturn ret;
  g_signal_emit_by_name(app_src_, "push-buffer", gst_buffer, &ret);
  if (ret != 0)
    g_printerr("gstEncoder -- failed render (result %u)\n", ret);

  return true;
}

bool gstEncoder::Init(){
  // check for default codec
  if (options_.m_codec == CODEC_UNKNOWN){
    g_print("gstEncoder -- codec not specified, defaulting to H.264\n");
    options_.m_codec = CODEC_H264;
  }

  // check if default framerate is needed
  if (options_.m_cam_options.m_frame_rate <= 0){
    g_print("gstEncoder -- framerate not specified, defaulting to 30\n");
    options_.m_cam_options.m_frame_rate = 30;
  }

  std::string strLaunch;
  if (!options_.BuildEncodeLaunchStr(strLaunch)){
    g_printerr("gstEncoder failed to build pipeline string\n");
    return false;
  }

  // create the pipeline
  GError *err = NULL;
  pipeline_ = gst_parse_launch(strLaunch.c_str(), &err);

  if (err != NULL){
    g_printerr("gstEncoder -- failed to create pipeline\n");
    g_printerr("   (%s)\n", err->message);
    g_error_free(err);
    return false;
  }

  GstPipeline *pipeline = GST_PIPELINE(pipeline_);

  if (!pipeline){
    g_printerr("gstEncoder -- failed to cast GstElement into GstPipeline\n");
    return false;
  }

  // retrieve pipeline bus
  bus_ = gst_pipeline_get_bus(pipeline);

  if (!bus_){
    g_printerr("gstEncoder -- failed to retrieve GstBus from pipeline\n");
    return false;
  }

  // get the appsrc element
  GstElement *appsrc_element = gst_bin_get_by_name(GST_BIN(pipeline), "mysource");
  if (appsrc_element != NULL){
    GstAppSrc *appsrc = GST_APP_SRC(appsrc_element);

    g_object_set(G_OBJECT(appsrc), "caps",
      gst_caps_new_simple(CCameraOption::CodecToString(options_.m_cam_options.m_codec),
        "format", G_TYPE_STRING, CCameraOption::ImageFormatToString(options_.m_cam_options.m_image_format),
        "width", G_TYPE_INT, options_.m_cam_options.m_width,
        "height", G_TYPE_INT, options_.m_cam_options.m_height,
        "framerate", GST_TYPE_FRACTION, options_.m_cam_options.m_frame_rate, 1,
        NULL),
      NULL);

    if (!appsrc_element || !appsrc){
      g_printerr("gstEncoder -- failed to retrieve appsrc element from pipeline\n");
      return false;
    }

    app_src_ = appsrc_element;

    g_signal_connect(appsrc_element, "need-data", G_CALLBACK(onNeedData), this);
    g_signal_connect(appsrc_element, "enough-data", G_CALLBACK(onEnoughData), this);
  }

  return true;
}

void gstEncoder::Uninit(){
  if (app_src_ != NULL){
    gst_object_unref(app_src_);
    app_src_ = NULL;
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

void gstEncoder::checkMsgBus(){
}

}