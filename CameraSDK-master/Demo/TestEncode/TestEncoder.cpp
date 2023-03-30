#include "Camera/gstCamera.h"
#include "CameraSDK.h"
#include "Code/gstEncoder.h"
#include "Base/gstUtils.h"
#include <iostream>
using namespace camerasdk;

// 定义宏: gstEncoder内部进行视频采集，否则:通过gstCamera采集视频回调给gstEncoder
#define TEST_ENCODE_WITH_SOURCE 1

void Run(){
  while (true){
    char c;
    scanf("%c", &c);
    if (c == 'q'){
      break;
    }
  }
}

void SelectVideoSize(int &iSizeIndex)
{
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

int main(int argc, char *argv[]){
  CameraSDK_Init();

  CCameraOption camOption;
  CCodeOption codeOption;

  int iDeviceIndex = 0;
  SelectDevice(iDeviceIndex);
  std::string strDevice = "/dev/video0";
  int port = 5022 + iDeviceIndex;
  if(iDeviceIndex == 1){
    strDevice = "/dev/video1";
  }
  else if(iDeviceIndex == 2){
    strDevice = "/dev/video2";
  }
  else if(iDeviceIndex == 3){
    strDevice = "/dev/video3";
  }
  camOption.m_resource = strDevice;

  int iSizeIndex = 1;
  SelectVideoSize(iSizeIndex);
  if (iSizeIndex == 2){
    camOption.m_video_scale = true;
    camOption.m_scale_width = 848;
    camOption.m_scale_height = 480;
    //camOption.m_width = 848;
    //camOption.m_height = 480;
    codeOption.m_codec = CODEC_H264;
  }
  else{
#ifdef CODE_720P_H265
    codeOption.m_codec = CODEC_H265;
#else
    codeOption.m_codec = CODEC_H264;
#endif
  }

#ifdef TEST_ENCODE_WITH_SOURCE
  codeOption.m_with_source = true;
#endif

  gstCamera *cam = NULL;

  if (!codeOption.m_with_source){
    cam = gstCamera::Create(camOption);
  }
  codeOption.m_cam_options = camOption;

  codeOption.m_location = "172.20.1.42";
  codeOption.m_port = port;
  gstEncoder *enc = gstEncoder::Create(codeOption);
  enc->Open();
  if (cam != NULL){
    cam->SetBufferCBFun(&gstEncoder::CameraBufferCBFun, enc);
    cam->Open();
  }

  Run();

  if (cam != NULL){
    cam->Close();
    delete cam;
  }
  enc->Close();
  delete enc;
  CameraSDK_Uninit();
  return 0;
}
