#include "gstDisplay.h"
#include <gst/app/gstappsrc.h>

namespace camerasdk{

gstDisplay::gstDisplay(){
  need_data_ = false;
  Init();
}

gstDisplay::~gstDisplay(){
  Uninit();
}

void gstDisplay::CameraBufferCBFun(GstBuffer *gst_buffer, void *user_data){
  gstDisplay *dec = (gstDisplay *)user_data;

  dec->onBuffer(gst_buffer);
}

void gstDisplay::onNeedData(GstElement *pipeline, guint size, void *user_data){
  if (!user_data)
    return;

  gstDisplay *enc = (gstDisplay *)user_data;
  enc->need_data_ = true;
}

void gstDisplay::onEnoughData(GstElement *pipeline, void *user_data){
  if (!user_data)
    return;

  gstDisplay *enc = (gstDisplay *)user_data;
  enc->need_data_ = false;
}

void gstDisplay::onBuffer(GstBuffer *gst_buffer){
  if (app_src_ == NULL)
  {
    return;
  }
  // return;
  if (!need_data_)
  {
    g_print("gstDisplay -- pipeline full\n");
    return;
  }
  g_print("gstDisplay onBuffer\n");
  GstFlowReturn ret;
  g_signal_emit_by_name(app_src_, "push-buffer", gst_buffer, &ret);
  /*static gboolean white = FALSE;
  static GstClockTime timestamp = 0;
  GstBuffer *buffer;
  guint size;
  GstFlowReturn ret;

  size = 1280 * 720 * 2;
  buffer = gst_buffer_new_allocate (NULL, size, NULL);

  // this makes the image black/white
  gst_buffer_memset (buffer, 0, white ? 0xFF : 0x00, size);

  white = !white;

  GST_BUFFER_PTS (buffer) = timestamp;
  GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale_int (1, GST_SECOND, 2);

  timestamp += GST_BUFFER_DURATION (buffer);

  g_signal_emit_by_name (app_src_, "push-buffer", buffer, &ret);
  gst_buffer_unref (buffer);
  */
}

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data){
  GMainLoop *loop = (GMainLoop *)data;
  switch (GST_MESSAGE_TYPE(msg))
  {
  case GST_MESSAGE_EOS:
    g_print("End of stream\n");
    g_main_loop_quit(loop);
    break;
  case GST_MESSAGE_ERROR:
  {
    gchar *debug;
    GError *error;
    gst_message_parse_error(msg, &error, &debug);
    g_printerr("ERROR from element %s: %s\n",
               GST_OBJECT_NAME(msg->src), error->message);
    if (debug)
      g_printerr("Error details: %s\n", debug);
    g_free(debug);
    g_error_free(error);
    g_main_loop_quit(loop);
    break;
  }
  default:
    break;
  }
  return TRUE;
}

void gstDisplay::Open(){
  gst_element_set_state(pipeline_, GST_STATE_PLAYING);
}

void gstDisplay::Run(){
  GstBus *bus = NULL;
  GMainLoop *loop = NULL;
  guint bus_watch_id;
  loop = g_main_loop_new(NULL, FALSE);
  bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline_));
  bus_watch_id = gst_bus_add_watch(bus, bus_call, loop);
  gst_object_unref(bus);
  g_main_loop_run(loop);
}

void gstDisplay::Close(){
  gst_element_set_state(pipeline_, GST_STATE_NULL);
  need_data_ = false;
}

int gstDisplay::Init(){
  app_src_ = gst_element_factory_make("appsrc", "app_src"); // v4l2src appsrc
  // avdec_h264_ = gst_element_factory_make("avdec_h264", "avdec_h264");
  video_convert_ = gst_element_factory_make("videoconvert", "video_convert");
  // tee_ = gst_element_factory_make("tee", "tee");
  // disp_queue_ = gst_element_factory_make("queue", "disp_queue");
  xvimage_sink_ = gst_element_factory_make("xvimagesink", "xvimage_sink");

  // Create the empty pipeline
  pipeline_ = gst_pipeline_new("test-pipeline");

  if (!pipeline_ || !app_src_ || !video_convert_ || !xvimage_sink_){
    g_printerr("Not all elements could be created.\n");
    return -1;
  }
  // RGB16
  g_object_set(G_OBJECT(app_src_), "caps",
    gst_caps_new_simple("video/x-raw",
      "format", G_TYPE_STRING, "YUY2",
      "width", G_TYPE_INT, 1280,
      "height", G_TYPE_INT, 720,
      "framerate", GST_TYPE_FRACTION, 0, 1,
      NULL),
    NULL);

  // gst_bin_add_many(GST_BIN(pipeline_), app_src_, video_convert_, tee_, disp_queue_, xvimage_sink_, NULL);
  // if (gst_element_link_many(app_src_, video_convert_, tee_, NULL) != TRUE ||
  //     gst_element_link_many(disp_queue_, xvimage_sink_, NULL) != TRUE)
  gst_bin_add_many(GST_BIN(pipeline_), app_src_, video_convert_, xvimage_sink_, NULL);
  if (gst_element_link_many(app_src_, video_convert_, xvimage_sink_, NULL) != TRUE){
    g_printerr("Elements could not be linked.\n");
    gst_object_unref(pipeline_);
    return -1;
  }

  /* setup appsrc */
  g_signal_connect(app_src_, "need-data", G_CALLBACK(onNeedData), this);
  g_signal_connect(app_src_, "enough-data", G_CALLBACK(onEnoughData), this);

  GstAppSrc *appsrc = GST_APP_SRC(app_src_);
  gst_app_src_set_stream_type(appsrc, GST_APP_STREAM_TYPE_STREAM);
  g_object_set(app_src_, "is-live", TRUE, NULL);
  g_object_set(app_src_, "stream-type", 0, NULL);
  g_object_set(app_src_, "format", GST_FORMAT_TIME, NULL);
  // 给实时视频流打上时间戳
  g_object_set(app_src_, "do-timestamp", TRUE, NULL);
  g_object_set(app_src_, "min-percent", 0, NULL); // then the min-latency must be set to 0 because it timestamps based on the running-time when the buffer entered appsrc

  // Manually link the Tee, which has "Request" pads
  // tee_disp_pad_ = gst_element_get_request_pad(tee_, "src_%u");
  // g_print("Obtained request pad %s for disp branch.\n", gst_pad_get_name(tee_disp_pad));
  // queue_disp_pad_ = gst_element_get_static_pad(disp_queue_, "sink");

  // if (gst_pad_link(tee_disp_pad_, queue_disp_pad_) != GST_PAD_LINK_OK)
  //{
  //     g_printerr("Tee could not be linked.\n");
  //     gst_object_unref(pipeline_);
  //     return -1;
  // }

  // g_object_set(G_OBJECT(disp_queue_), "max-size-buffers", 0, NULL);
  // g_object_set(G_OBJECT(disp_queue_), "max-size-time", 0, NULL);
  // g_object_set(G_OBJECT(disp_queue_), "max-size-bytes", 512000000, NULL);

  // gst_object_unref(queue_disp_pad_);

  return 0;
}

void gstDisplay::Uninit(){
  gst_element_release_request_pad(tee_, tee_disp_pad_);
  gst_object_unref(tee_disp_pad_);
  gst_object_unref(pipeline_);
}

}