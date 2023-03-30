#include "CameraSDK.h"
#include "Base/gstUtils.h"
#include <gst/app/gstappsink.h>
#include <string>
#include <sstream>
#include <sys/time.h>


static void GetCurTime(time_t&s , int& ms){
  struct timeval tv;
  memset(&tv, 0, sizeof(tv));
  gettimeofday(&tv, NULL);
  s = tv.tv_sec;
  ms = (int)(tv.tv_usec / 1000);
}

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data){
  GMainLoop *loop = (GMainLoop *)data;
  switch (GST_MESSAGE_TYPE(msg))
  {
  case GST_MESSAGE_EOS:
    g_print("End of stream\n");
    g_main_loop_quit(loop);
    break;
  case GST_MESSAGE_ERROR:{
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

void Run(GstPipeline *pipeline){
  if (!pipeline){
    g_printerr("testPlayView -- failed to cast GstElement into GstPipeline\n");
    return;
  }
  GstBus *bus = NULL;
  // retrieve pipeline bus
  bus = gst_pipeline_get_bus(pipeline);
  if (!bus){
    g_printerr("testPlayView -- failed to retrieve GstBus from pipeline\n");
    return;
  }
  GMainLoop *loop = NULL;
  guint bus_watch_id;
  loop = g_main_loop_new(NULL, FALSE);
  bus_watch_id = gst_bus_add_watch(bus, bus_call, loop);
  gst_object_unref(bus);
  g_main_loop_run(loop);
}

void SelectVideoSize(int &iSizeIndex){
  g_printerr("Select Video Size:\n");
  g_printerr("1. 1280 *7 20\n");
  g_printerr("2. 848 * 480 \n");
  scanf("%d", &iSizeIndex);
}

void SelectDevice(int &iDeviceIndex){
  g_printerr("Select Device:\n");
  g_printerr("0. /dev/video0\n");
  g_printerr("1. /dev/video1\n");
  g_printerr("2. /dev/video2\n");
  g_printerr("3. /dev/video3\n");
  scanf("%d", &iDeviceIndex);
}

void BuildPlayLaunch(std::string &strLaunch){
  int iDeviceIndex = 0;
  SelectDevice(iDeviceIndex);
  int port = 5022 + iDeviceIndex;
  int iSizeIndex = 1;
  SelectVideoSize(iSizeIndex);
  std::ostringstream ss;
  bool bH264 = true;
  if (iSizeIndex == 1){
#ifdef CODE_720P_H265
    bH264 = false;
#endif
  }
  // ss << "rtpsrc address=127.0.0.1,port=" << port << " ! application/x-rtp,encoding-name=H264 ! rtpjitterbuffer latency=0 ! rtph264depay ! ";
  if (bH264){
    ss << "udpsrc port=" << port << " ! application/x-rtp,encoding-name=H264 ! rtpjitterbuffer latency=0";
  }
  else{
    ss << "udpsrc port=" << port << " ! application/x-rtp,encoding-name=H265 ! rtpjitterbuffer latency=0";
  }

  //ss << " ! tee name=t    t. ! queue";

  if (bH264){
    ss << " ! rtph264depay ! ";

#ifdef GST_CODECS_OMX
    ss << "video/x-h264, framerate=30/1 ! h264parse ! ";
#endif

    ss << "tee name=t    t. ! queue ! ";
    ss << GST_DECODER_H264;
  }
  else{
    ss << " ! rtph265depay ! ";

#ifdef GST_CODECS_OMX
    ss << "video/x-h265, framerate=30/1 ! h265parse ! ";
#endif

    //ss << "tee name=t    t. ! queue ! ";
    ss << GST_DECODER_H265;
  }

  ss << " ! videoconvert ! ";

  //ss << "tee name=t    t. ! queue ! ";
  ss << "tee name=t2   t2. ! queue ! appsink name=sink_ana    t2. ! queue ! ";

#ifdef GST_CODECS_OMX
  ss << "xvimagesink";
#else
  ss << "ximagesink";
#endif
  
  ss << "   t. ! queue ! appsink name=sink_bit";
  strLaunch = ss.str();
  g_print("%s\n", strLaunch.c_str());

  //  ! tee name=t    t. ! queue ! jpegenc ! shmsink wait-for-connection=false socket-path=/tmp/blah sync=false    t. ! queue ! appsink sync=false
}

static bool is_first_frame = true;
static int s_width = 0;
static int s_height = 0;
static float s_framerate = 0;
static int s_real_framerate = 0;
uint64_t s_last_time = 0;

static int s_real_bitrate = 0;
uint64_t s_last_time_bitrate = 0;

static uint64_t GetTimeMs(){
  time_t s = 0;
  int ms = 0;
  GetCurTime(s, ms);
  uint64_t time_ms = (uint64_t)s * 1000 + ms;
  return time_ms;
}

static GstFlowReturn new_sample1 (GstElement *sink, gpointer user_data){
  GstSample *sample;
  GstBuffer *buffer;
  g_signal_emit_by_name (sink, "pull-sample", &sample);
  if (sample){
    buffer = gst_sample_get_buffer (sample);		
    if(!buffer){
        g_print ("gst_sample_get_buffer fail\n");
        gst_sample_unref (sample);
        return GST_FLOW_ERROR;
    }
    GstMapInfo map;     
    //把buffer映射到map，这样我们就可以通过map.data取到buffer的数据
    if (gst_buffer_map (buffer, &map, GST_MAP_READ)){	
      
      //g_print("buffer size = %ld \n", map.size);

      s_real_bitrate += map.size;
      if(s_last_time_bitrate == 0){
        s_last_time_bitrate = GetTimeMs();
      }
      
      uint64_t cur_ms = GetTimeMs();
      if((cur_ms - s_last_time_bitrate) > 1000){
        g_print("real bitrate = %f MBit/s\n", s_real_bitrate * 8.0 / 1000 / 1000 * 1000 / (cur_ms - s_last_time_bitrate));
        s_last_time_bitrate = cur_ms;
        s_real_bitrate = 0;
      }

      gst_buffer_unmap (buffer, &map);	//解除映射
    }
    gst_sample_unref (sample);
  }

  return GST_FLOW_OK ;
}


static GstFlowReturn new_sample (GstElement *sink, gpointer user_data){
  GstSample *sample;
  GstCaps *caps;
  GstStructure *s;

  /* Retrieve the buffer */
  g_signal_emit_by_name (sink, "pull-sample", &sample);
  if (sample){
    //g_print ("*");
    caps = gst_sample_get_caps (sample);
    if (!caps) {
        g_print ("gst_sample_get_caps fail\n");
        gst_sample_unref (sample);
        return GST_FLOW_ERROR;
    }
    const uint32_t num_caps = gst_caps_get_size(caps);
    //g_print("num_caps = %u\n", num_caps);

    s = gst_caps_get_structure (caps, 0);
    //g_print("%s\n", gst_structure_to_string(s));

    s_real_framerate++;

    if(is_first_frame){
      is_first_frame = false;
      gboolean res;
      res = gst_structure_get_int (s, "width", &s_width);		//获取图片的宽
      res |= gst_structure_get_int (s, "height", &s_height);	//获取图片的高
      if (!res) {
          g_print ("gst_structure_get_int fail\n");
      }
      g_print("width = %d, height = %d\n", s_width, s_height);

      
      int framerate_num = 0;
      int framerate_denom = 0;

      if (gst_structure_get_fraction(s, "framerate", &framerate_num, &framerate_denom)){
        s_framerate = float(framerate_num) / float(framerate_denom);
      }
      g_print("framerate = %f\n", s_framerate);
      s_last_time = GetTimeMs();
    }

    uint64_t cur_ms = GetTimeMs();
    if((cur_ms - s_last_time) > 1000){
      g_print("real framerate = %d\n", s_real_framerate * 1000 / (cur_ms - s_last_time));
      s_last_time = cur_ms;
      s_real_framerate = 0;
    }
    
    gst_sample_unref (sample);
  }

  return GST_FLOW_OK ;
 
}

int main(int argc, char *argv[]){
  GstElement *mPipeline;
  GstBus *mBus;

  CameraSDK_Init();
  std::string strLaunch;
  BuildPlayLaunch(strLaunch);

  GError *err = NULL;
  mPipeline = gst_parse_launch(strLaunch.c_str(), &err);

  if (err != NULL){
    g_printerr("testPlayView -- failed to create pipeline\n");
    g_printerr("   (%s)\n", err->message);
    g_error_free(err);
    return -1;
  }

  GstPipeline *pipeline = GST_PIPELINE(mPipeline);

  GstElement *appsinkElement = gst_bin_get_by_name(GST_BIN(pipeline), "sink_ana");
  GstAppSink *appsink = GST_APP_SINK(appsinkElement);

  if (!appsinkElement || !appsink){
    g_printerr("gstCamera failed to retrieve AppSink element from pipeline\n");
    return false;
  }

  g_object_set(appsinkElement, "emit-signals", TRUE, NULL);
  void* user_data = NULL;
  g_signal_connect(appsinkElement, "new-sample", G_CALLBACK (new_sample), user_data);

  GstElement *appsinkBit = gst_bin_get_by_name(GST_BIN(pipeline), "sink_bit");
  if(appsinkBit){
    g_object_set(appsinkBit, "emit-signals", TRUE, NULL);
     void* user_data1 = NULL;
    g_signal_connect(appsinkBit, "new-sample", G_CALLBACK (new_sample1), user_data1);
  }
  

  // Start playing the pipeline
  gst_element_set_state(mPipeline, GST_STATE_PLAYING);
  Run(pipeline);

  gst_element_set_state(mPipeline, GST_STATE_NULL);
  gst_object_unref(mPipeline);

  CameraSDK_Uninit();
  return 0;
}
