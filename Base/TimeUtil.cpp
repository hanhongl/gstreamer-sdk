#include "TimeUtil.h"
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

namespace camerasdk{

std::string GetCurTimeStr(){
  char buf[32] = {0};
  struct timeval tv;
  struct tm tm;
  size_t len = 28;
	
  memset(&tv, 0, sizeof(tv));
  memset(&tm, 0, sizeof(tm));
  gettimeofday(&tv, NULL);
  localtime_r(&tv.tv_sec, &tm);
  strftime(buf, len, "%Y-%m-%d %H:%M:%S", &tm);
  len = strlen(buf);
  sprintf(buf + len, ".%-6.3d", (int)(tv.tv_usec / 1000));
  return buf;
}

}