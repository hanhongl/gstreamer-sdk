#include "Camera/gstCamera.h"
#include "../gstDisplay.h"
#include "CameraSDK.h"
#include "Base/gstUtils.h"
#include <iostream>
using namespace camerasdk;

int main(int argc, char *argv[]){
  CameraSDK_Init();

#if 0
  {
    CameraInfoList cam_list;
    GetAllCamera(cam_list);
    for (int i = 0; i < cam_list.size(); i++){
      CameraInfo& cam_info = cam_list.at(i);
      g_print("camera[%d] path: %s\n", i, cam_info.path.c_str());

      for (int j = 0; j < cam_info.caps.size(); j++){
        CameraCapInfo& cap_info = cam_info.caps.at(j);
        g_print("camera caps[%d]:  codec=%s format=%s width=%u height=%u framerate=%f\n", j, \
                CCameraOption::CodecToString(cap_info.codec), CCameraOption::ImageFormatToString(cap_info.format), \
                cap_info.width, cap_info.height, cap_info.framerate);
      }
    }
    return 0;
  }
#endif

  CCameraOption option;
  gstCamera *cam = gstCamera::Create(option);
  gstDisplay *dis = new gstDisplay();
  cam->SetBufferCBFun(&gstDisplay::CameraBufferCBFun, dis);
  dis->Open();
  cam->Open();
  dis->Run();

  /*while(true)
  {
      char c;
      scanf("%c", &c);
      if(c == 'q')
      {
          break;
      }
  }*/

  cam->Close();
  dis->Close();
  delete cam;
  delete dis;
  CameraSDK_Uninit();
  return 0;
}
