#include "Base/TimeUtil.h"
#include <unistd.h>

using namespace camerasdk;

int main(int argc, char *argv[]){
  while(true){
    g_printerr("cur time = %s\n", GetCurTimeStr().c_str());
    usleep(1000);
  }
  return 0;
}