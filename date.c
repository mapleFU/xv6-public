#include "types.h"
#include "user.h"
#include "date.h"

struct rtcdate;

int
main(int argc, char *argv[])
{
  struct rtcdate r;
  
  if (date(&r)) {
    printf(2, "date failed\n");
    exit();
  }
	printf(2, "UTC: %d-%d-%d %d:%d:%d\n", r.year, r.month, r.day, r.hour, r.minute, r.second);
  // your code to print the time in any format you like...

  exit();
}

