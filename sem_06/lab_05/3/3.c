#include <fcntl.h>
#include <stdio.h>

int main() 
{
  FILE *fd1 = fopen("q.txt", "w");
  FILE *fd2 = fopen("q.txt", "w");

  int curr = 0;

  for(char c = 'a'; c <= 'z'; c++)
  {
	if (c%2)
        {
	    fprintf(fd1, &c, 1);
	}
  	else
        {
            printf("%d %c", (int)c, c);
	    fprintf(fd2, &c, 1);
	}
  }
  fclose(fd1);
  fclose(fd2);
  return 0;
}
