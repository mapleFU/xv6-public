#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

#define DEBUG_SORT1 
#undef DEBUG_SORT1

static char buf[512];
static const int MAXLINE = 30;
static const int LINE_CHAR_MAX = 80;
static const char* SPLITER = "\n\t";

void sort(int fd) {
  // bubble sort for strings...
  int n;
  int line_nums = 0;
  char line_datas[MAXLINE][LINE_CHAR_MAX];
  
  int line_pos = 0;

  while((n = read(fd, buf, sizeof(buf))) > 0) {
    if (n < 512) {
      buf[n] = '\0';
    }
    // printf(1, "read a line~\n");
    int sum_pos = 0;
    while(1) {
      if (sum_pos >= strlen(buf)) {
        line_datas[line_nums][line_pos] = '\0';
        // printf(1, "Line nums is %d, break out\n", line_nums);
        break;
      }
      
      if (strchr(SPLITER, buf[sum_pos])) {
        
        ++sum_pos;
        line_datas[line_nums][line_pos] = '\0';

        // printf(1, "consume a line: %s. linepos: %d, sumpos:%d, strlen: %d\n", line_datas[line_nums], line_pos, sum_pos, strlen(buf));

        ++line_nums;
        line_pos = 0;
      
        continue;
      }
      line_datas[line_nums][line_pos++] = buf[sum_pos++];
    }

  }
  
  // fill value for it.
  int index[line_nums];
  for (int i = 0; i < line_nums; i++) {
    index[i] = i;
  }

  // sort
  for (int i = 1; i < line_nums; i++) {
    // insertion sort
    for (int j = i; j > 0; j--) {
      if (strcmp(line_datas[index[j]], line_datas[index[j - 1]]) < 0) {
        int tmp = index[j];
        index[j] = index[j - 1];
        index[j - 1] = tmp;
      } else {
        break;
      }
    }
  }
  
  // for (int i = 0; i < line_nums; i++) {
  //   printf(1, "%d ", index[i]);
  // }
  // printf(1, "\n");

  for (int j = 0; j < line_nums; j++) {
    if (strlen(line_datas[index[j]]) > 0) {
      printf(1, "%s\n", line_datas[index[j]]);
    } else {
      break;
    }
  }

}

int
main(int argc, char *argv[])
{
  int fd;
  // 0-stdin 1-stdout 2-screen?
  // cat for stdin
  if(argc <= 1){
    // sort sdtin
    fd = 0;
    sort(fd);
    exit();
  } else {
    printf(1, "debug: nmsl\n");
  }
  exit();
}
