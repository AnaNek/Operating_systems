#include <stdio.h>
#include <dirent.h>	//opendir()/readdir()/closedir()
#include <sys/stat.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <unistd.h>

static int dopath(const char *filename, int depth)
{
	struct stat statbuf;
	struct dirent *dirp;
	DIR *dp;
	int fl = 1;

	if (lstat(filename, &statbuf) < 0)
	{
            switch (errno)
            {
                case EBADF:
                    printf("Неверный файловый дескриптор\n");
                    break;
                case ENOENT:
                    printf("%s: Отсутствует компонент полного имени файла или полное имя - пустая строка\n",
                       filename);
                    break;
                case ENOTDIR:
                    printf("%s: Компонент пути не является каталогом\n", filename);
                    break;
                case ELOOP:
                    printf("Символьная ссылка\n");
                    break;
                case EFAULT:
                    printf("Некорректный адрес\n");
                    break;
                case EACCES:
                    printf("Доступ запрещен\n");
                    break;
                case ENOMEM:
                    printf("Недостаточно памяти\n");
                    break;
                case ENAMETOOLONG:
                    printf("Слишком длинное название файла\n");
                    break;
            }
            return -1;
	}

	// не каталог
	if (S_ISDIR(statbuf.st_mode) == 0)
	{
		for (int i = 0; i < depth - 1; ++i)
			printf("     ");
		printf("|____%s\n", filename);
		return 0;
	}

	// каталог
	for (int i = 0; i < depth; ++i)
		printf("     ");
	printf("%s\n", filename);
	if ((dp = opendir(filename)) == NULL)
	{
		printf("каталог %s недоступен\n", filename);
		return -1;
	}

	chdir(filename);

	// для каждого элемента каталога
	while ((dirp = readdir(dp)) != NULL && fl)
	{
		if (strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0)
                {
			int rc = dopath(dirp->d_name, depth + 1); //рекурсия
			if (rc)
				fl = 0;
		}
	}
	chdir("..");

	if (closedir(dp) < 0)
	{
	   perror("Невозможно закрыть каталог");
   	   return -1;
        }
	return 0;
}

static int my_ftw(char * pathname)
{
    return(dopath(pathname, 0));
}

int main(int argc, char *argv[])
{
	int ret;

	if (argc != 2)
		ret = my_ftw(".");
	else
		ret = my_ftw(argv[1]);

	return ret;
}
