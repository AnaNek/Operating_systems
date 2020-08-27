#include <stdio.h>
#include <windows.h>

#define countW 5
#define countR 5

HANDLE thr[countR];
HANDLE thw[countW];

bool wrt = false;
volatile LONG nr = 0;
volatile LONG wnr = 0;
volatile LONG wnw = 0;

HANDLE can_read;
HANDLE can_write;
HANDLE mutex;

int val = 0;

void start_read()
{
	InterlockedIncrement(&wnr);
	if (wrt || WaitForSingleObject(can_write, 0) == WAIT_OBJECT_0)
	{
		WaitForSingleObject(can_read, INFINITE);
	}
	InterlockedDecrement(&wnr);
	InterlockedIncrement(&nr);
	SetEvent(can_read);
}

void start_write()
{
	InterlockedIncrement(&wnw);
	if (nr > 0 || wrt)
	{
		WaitForSingleObject(can_write, INFINITE);
	}
	WaitForSingleObject(mutex, INFINITE);
	InterlockedDecrement(&wnw);
	wrt = true;
	ResetEvent(can_write);
	ReleaseMutex(mutex);
}

void stop_read()
{
	InterlockedDecrement(&nr);
	if (nr == 0)
	{
		SetEvent(can_write);
	}
}

void stop_write()
{
	wrt = false;
	if (WaitForSingleObject(can_read, 0) == WAIT_OBJECT_0)
	{
		SetEvent(can_read);
	}
	else
	{
		SetEvent(can_write);
	}
}

DWORD WINAPI reader(LPVOID)
{
	srand(GetCurrentThreadId());
	while (1)
	{
		Sleep(rand() % 5 * 1000);
		start_read();
		printf("Reader[%d] value: %d\n", GetCurrentThreadId(), val);
		stop_read();
	}
	return 0;
}

DWORD WINAPI writer(LPVOID)
{
	srand(GetCurrentThreadId());
	while (1)
	{
		Sleep(rand() % 5 * 1000);
		start_write();
		printf("Writer[%d] value: %d\n", GetCurrentThreadId(), ++val);
		stop_write();
	}
	return 0;
}

int main()
{
	mutex = CreateMutex(NULL, FALSE, NULL);
	if (mutex == NULL)
	{
		perror("mutex");
		return 1;
	}

	can_read = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (can_read == NULL) {
		perror("can_read");
		return 1;
	}

	can_write = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (can_write == NULL) {
		perror("can_write");
		return 1;
	}

	for (int i = 0; i < countW; i++)
	{
		thw[i] = CreateThread(NULL, 0, writer, NULL, 0, NULL);

		if (thw[i] == NULL)
		{
			perror("thread creation\n");
			return 1;
		}
	}

	for (int i = 0; i < countR; i++)
	{
		thr[i] = CreateThread(NULL, 0, reader, NULL, 0, NULL);

		if (thr[i] == NULL)
		{
			perror("thread creation\n");
			return 1;
		}
	}

	WaitForMultipleObjects(countW, thw, TRUE, INFINITE);
	WaitForMultipleObjects(countR, thr, TRUE, INFINITE);

	CloseHandle(mutex);
	CloseHandle(can_read);
	CloseHandle(can_write);

	system("pause");
	return 0;
}
