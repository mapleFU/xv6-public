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


typedef unsigned long HashResult;
typedef struct {
  HashResult* hashset;
  int current_size;
  int max_size;
} HashSet;

HashResult
hash(char *str)
{
    HashResult hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

HashSet* buildHashSet(int size) {
  HashResult* result = malloc(sizeof(HashResult[size]));
  memset(result, 0, sizeof(HashResult[size]));
  HashSet* hashset = malloc(sizeof(HashSet));
  
  hashset->current_size = 0;
  hashset->max_size = size;
  hashset->hashset = result;

  return hashset;
}

static HashResult* check_hashset_valid(HashSet* hashset) {
  if (!hashset) {
    printf(2, "Err, hashset is nil");
    return 0;
  }
  HashResult* arr = hashset->hashset;
  if (!arr) {
    printf(2, "Err, hashset->hashset is nil");
    return 0;
  }

  return arr;
}

int exist(HashSet* hashset, HashResult result_value) {
  HashResult* arr = 0;
  if ((arr = check_hashset_valid(hashset))) {
    for(int i = 0;  i < hashset->max_size; i++) {
      if (arr[i] == result_value) {
        return 1;
      }
    }
    return 0;
  }
  return 0;
}

int add(HashSet* hashset, HashResult result_value) {
  HashResult* arr = 0;
  if ((arr = check_hashset_valid(hashset))) {
    if (hashset->current_size >= hashset->max_size) {
      printf(2, "Err, hashset is full");
      return 0;
    }
    
    hashset->hashset[hashset->current_size] = result_value;
    hashset->current_size++;
    return 1;
  }
  return 0;
}

void DestroyHashSet(HashSet* hashset) {
  free(hashset->hashset);
  free(hashset);
}

void uniq(int fd) {
  // bubble sort for strings...
  int n;
  // int line_nums = 0;
  char line_datas[LINE_CHAR_MAX];
  
  HashSet *set = buildHashSet(MAXLINE);

  int line_pos = 0;

  while((n = read(fd, buf, sizeof(buf))) > 0) {
    if (n < 512) {
      buf[n] = '\0';
    }
    // printf(1, "read a line~\n");
    int sum_pos = 0;
    while(1) {
      if (sum_pos >= strlen(buf)) {
        // printf(1, "Line nums is %d, break out\n", line_nums);
        break;
      }
      
      if (strchr(SPLITER, buf[sum_pos])) {
        
        ++sum_pos;
        line_datas[line_pos] = '\0';

        HashResult cur_hash = hash(line_datas);
        if (!exist(set, cur_hash)) {
          add(set, cur_hash);
          printf(1, "%s\n", line_datas);
        }
        // set zero
        memset(line_datas, 0, strlen(line_datas));

        line_pos = 0;
      
        continue;
      }
      line_datas[line_pos++] = buf[sum_pos++];
    }

  }
  DestroyHashSet(set);
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
    uniq(fd);
    exit();
  } else {
    printf(1, "debug: nmsl\n");
  }
  exit();
}
