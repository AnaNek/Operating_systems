/*//testKernelIO.c
#include <fcntl.h>
#include <unistd.h>

int main()
{
   char c;    
  // have kernel open two connection to file alphabet.txt
  int fd1 = open("alphabet.txt",O_RDONLY);
  int fd2 = open("alphabet.txt",O_RDONLY);
  // read a char & write it alternatingly from connections fs1 & fd2
  int fl = 1;

  while(fl)
  {
    fl = 0;
    if (read(fd1,&c,1))
    {
          write(1,&c,1);
          fl = 1;
    }
    if (read(fd2,&c,1))
    {
          write(1,&c,1);
          fl = 1;
    }
  }
  return 0;
}
*/

 //testKernelIO.c
  #include <fcntl.h>
  #include <unistd.h>

  int main()
  {
      char c;    
      // have kernel open two connection to file alphabet.txt
      int fd1 = open("alphabet.txt",O_RDONLY);
      int fd2 = open("alphabet.txt",O_RDONLY);
      // read a char & write it alternatingly from connections fs1 & fd2
      int fl = 1;

      while(fl)
      {
          fl = 0;
          if (read(fd1,&c,1))
          {
              write(1,&c,1);
              if (read(fd2,&c,1))
              {
                  write(1,&c,1);
                  fl = 1;
              }
          }
      }
      return 0;
  }