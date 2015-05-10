#include <stdio.h>
#include <time.h>

int main ()
{
  time_t rawtime;
  struct tm * timeinfo;

  time ( &rawtime );
  printf ( "Current local time and date: %s", ctime (&rawtime) );

  return 0;
}
