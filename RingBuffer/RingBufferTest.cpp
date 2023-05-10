#include "RingBuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <windows.h>

#include <map>
#include <list>
#include <synchapi.h>
#include <process.h>
#include <conio.h>

#define SOURCE_PATTERN_STRING_CNT 81

#define VK_Q 0x51

char strPeek[82] = { 0, };
char strDequeue[82] = { 0, };

void logToFile(time_t seedTime, __int64 loopCnt, int lineNo);
int testCase1();
int testCase2();
void testCase3_RingBuffer_MultiThread_Enqueue_Dequeue();

int main(void)
{
	//testCase1();
	//testCase2();
	/*printf("enqueue size: %d\n", rbufTest.Enqueue(nullptr, 100));
	printf("dequeue size: %d\n", rbufTest.Dequeue(nullptr, 20));
	printf("링버퍼 전체 크기%d\n", rbufTest.GetBufferSize());
	printf("링버퍼 사용 크기%d\n", rbufTest.GetUseSize());
	printf("링버퍼 가용 크기%d\n", rbufTest.GetFreeSize());*/
	testCase3_RingBuffer_MultiThread_Enqueue_Dequeue();
	return 0;
}

int testCase1() 
{
	time_t startAndSeedTime = 0x0000000063ee8ff0;
	//srand(startAndSeedTime);
	srand((unsigned int)time(&startAndSeedTime));

	RingBuffer rbufTest(SOURCE_PATTERN_STRING_CNT);
	rbufTest.MoveRear(70);
	rbufTest.MoveFront(70);
	int copySize;
	int startIndex = 0;
	int nextStartIndex = 0;
	int restStrCnt = SOURCE_PATTERN_STRING_CNT;
	__int64 loopCnt = 0;
	int ll = 100;

	const char strPattern[82] = "1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 12345";
	char   compstrPattern[82] = "1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 12345";

	while (ll)
	{
		copySize = (restStrCnt == 1) ? 1 : (rand() % restStrCnt) + 1;

		rbufTest.Enqueue(strPattern + nextStartIndex, copySize);
		rbufTest.Peek(strPeek, copySize);
		strPeek[copySize] = '\0';
		rbufTest.Dequeue(strDequeue, copySize);
		strDequeue[copySize] = '\0';
		printf("%s", strDequeue);

		if (memcmp(strPeek, strDequeue, copySize) != 0)
		{
			logToFile(startAndSeedTime, loopCnt, __LINE__);
			return -1;
		}

		memcpy(compstrPattern + nextStartIndex, strDequeue, copySize);

		if (memcmp(compstrPattern, strPattern, sizeof(strPattern)) != 0)
		{
			logToFile(startAndSeedTime, loopCnt, __LINE__);
			return -1;
		}
		nextStartIndex += copySize;
		restStrCnt -= copySize;
		if (restStrCnt <= 0)
		{
			restStrCnt = SOURCE_PATTERN_STRING_CNT;
			nextStartIndex = 0;
		}
		loopCnt += 1;
	}

	return 0;
}

int testCase2() 
{
	RingBuffer rbufTest(5);
	rbufTest.MoveRear(2);
	rbufTest.MoveFront(2);

	rbufTest.Enqueue("12345", 5);
	char buf[6] = { 0, };
	rbufTest.Dequeue(buf, 5);
	if (memcmp(buf, "12345", sizeof(buf)) == 0)
	{
		printf("Dequeue 성공\n");
	}
	else
	{
		printf("Dequeue 실패\n");
	}
	return 0;
}

struct FreeAndUseSize
{
	int requestSize;
	int freeSize;
	int useSize;
};
std::map<int, FreeAndUseSize> testEnqueueMap;

RingBuffer shareRingBuffer(SOURCE_PATTERN_STRING_CNT);
RingBuffer shareCopySizeRingBuffer(SOURCE_PATTERN_STRING_CNT);
std::list<int> copySizeList;
bool isShutdown = false;
SRWLOCK srwlock = RTL_SRWLOCK_INIT;
unsigned int seedValue = 0x6459516a;//(unsigned int)time(nullptr);
unsigned _stdcall EnqueueThread(void* args)
{
	srand(seedValue);

	const char strPattern[82] = "1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 12345";
	shareRingBuffer.MoveRear(70);
	shareRingBuffer.MoveFront(70);
	int copySize;
	int nextStartIndex = 0;
	int restStrCnt = SOURCE_PATTERN_STRING_CNT;
	int loopCnt = 0;
	for (;;)
	{
		if (isShutdown)
		{
			break;
		}

		copySize = (restStrCnt == 1) ? 1 : (rand() % restStrCnt) + 1;
		//shareRingBuffer.Enqueue(strPattern + nextStartIndex, copySize);
		if (shareRingBuffer.Enqueue(strPattern + nextStartIndex, copySize) == 0)
		{
			continue;
		}

		testEnqueueMap.insert({ loopCnt, { copySize, shareRingBuffer.GetFreeSize(), shareRingBuffer.GetUseSize() } });
		//AcquireSRWLockExclusive(&srwlock);
		//copySizeList.push_back(copySize);
		if (copySize == 256)
		{
			while (shareCopySizeRingBuffer.Enqueue((char*)&copySize, sizeof(copySize)) == 0);
		}
		else
		{
			while (shareCopySizeRingBuffer.Enqueue((char*)&copySize, sizeof(copySize)) == 0);
		}
		
		//ReleaseSRWLockExclusive(&srwlock);

		nextStartIndex += copySize;
		restStrCnt -= copySize;
		if (restStrCnt <= 0)
		{
			restStrCnt = SOURCE_PATTERN_STRING_CNT;
			nextStartIndex = 0;
		}

		loopCnt += 1;
	}
	printf("Enqueue Thread 종료\n");
	return 0;
}


std::map<int, FreeAndUseSize> testDequeueMap;
unsigned _stdcall DequeueThread(void* args)
{
	const char strPattern[82] = "1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 12345";
	char   compstrPattern[82] = "1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 12345";
	char strPeek[82] = { 0, };
	char strDequeue[82] = { 0, };
	int copySize;
	int nextStartIndex = 0;
	int restStrCnt = SOURCE_PATTERN_STRING_CNT;
	int loopCnt = 0;
	for (;;)
	{
		//AcquireSRWLockExclusive(&srwlock);
		/*if (copySizeList.size() > 0)
		{
			copySize = copySizeList.front();
			copySizeList.pop_front();
			//ReleaseSRWLockExclusive(&srwlock);
		}
		else
		{
			//ReleaseSRWLockExclusive(&srwlock);
			continue;
		}*/
		
		if (shareCopySizeRingBuffer.Dequeue((char*)&copySize, sizeof(copySize)) == 0)
		{
			continue;
		}
		int peekFront = 0;
		int peekRear = 0;
		int resultsize1 = shareRingBuffer.Peek(strPeek, copySize, &peekRear, &peekFront);
		testDequeueMap.insert({ loopCnt, {copySize, shareRingBuffer.GetFreeSize(), shareRingBuffer.GetUseSize()} });
		strPeek[copySize] = '\0';
		
		int dequeueFront = 0;
		int dequeueRear = 0;
		int resultsize2 = shareRingBuffer.Dequeue(strDequeue, copySize, &dequeueRear, &dequeueFront, resultsize1);
		testDequeueMap.insert({ loopCnt+1, {copySize, shareRingBuffer.GetFreeSize(), shareRingBuffer.GetUseSize()} });
		//shareRingBuffer.GetFreeSize();
		//shareRingBuffer.GetUseSize();
		strDequeue[copySize] = '\0';
		
		printf("%s", strDequeue);

		if (memcmp(strPeek, strDequeue, copySize) != 0)
		{
			
			//testDequeueMap[loopCnt].useSize;
			printf("Peek Dequeue 에러 발생 freesize: %d, useSize: %d\n", testDequeueMap[loopCnt].freeSize, testDequeueMap[loopCnt].useSize);
			printf("Dequeue Dequeue 에러 발생 freesize: %d, useSize: %d\n", testDequeueMap[loopCnt + 1].freeSize, testDequeueMap[loopCnt + 1].useSize);
			logToFile(seedValue, loopCnt, __LINE__);
			isShutdown = true;
		}

		memcpy(compstrPattern + nextStartIndex, strDequeue, copySize);
		if (memcmp(compstrPattern, strPattern, sizeof(strPattern)) != 0)
		{
			printf("Enqueue Dequeue 에러 발생\n");
			logToFile(seedValue, loopCnt, __LINE__);
			shareRingBuffer.GetFrontBufferPtr();
			isShutdown = true;
		}

		if (loopCnt == 178829)
		{
			shareRingBuffer.GetFrontBufferPtr();
			shareRingBuffer.GetRearBufferPtr();
		}

		nextStartIndex += copySize;
		restStrCnt -= copySize;
		if (restStrCnt <= 0)
		{
			restStrCnt = SOURCE_PATTERN_STRING_CNT;
			nextStartIndex = 0;
		}

		loopCnt += 1;

		if (isShutdown)
		{
			break;
		}
	}

	printf("Dequeue Thread 종료\n");
	return 0;
}

void testCase3_RingBuffer_MultiThread_Enqueue_Dequeue()
{
	HANDLE enqueueThread = (HANDLE)_beginthreadex(nullptr, 0, EnqueueThread, nullptr, 0, nullptr);
	HANDLE dequeueThread = (HANDLE)_beginthreadex(nullptr, 0, DequeueThread, nullptr, 0, nullptr);

	for (;;)
	{
		if (_kbhit())
		{
			_getch();
			if (GetAsyncKeyState(VK_Q) & 0x8001)
			{
				isShutdown = true;
			}
		}

		if (isShutdown)
		{
			break;
		}
	}

	HANDLE hThreadArr[2] = { enqueueThread, dequeueThread };
	DWORD result = WaitForMultipleObjects(2, hThreadArr, true, INFINITE);
	if (result == WAIT_FAILED)
	{
		printf("\n\n\nWaitForMultipleObjects error code: %d\n", GetLastError());
	}

	printf("모든 Thread 종료\n");
	CloseHandle(enqueueThread);
	CloseHandle(dequeueThread);
}

void logToFile(time_t seedTime, __int64 loopCnt, int lineNo)
{
	tm breakTm;
	time_t breakTime;
	time(&breakTime);
	localtime_s(&breakTm, &breakTime);

	tm startTm;
	localtime_s(&startTm, &seedTime);
	FILE* file;
	char fileName[150] = { 0, };
	char yyyyMMddhhiiss[20] = { 0, };
	char yyyyMMddhhiiss2[20] = { 0, };
	sprintf_s(yyyyMMddhhiiss, sizeof(yyyyMMddhhiiss), "%04d-%02d-%02d %02d_%02d_%02d"
		, startTm.tm_year + 1900, startTm.tm_mon + 1, startTm.tm_mday
		, startTm.tm_hour, startTm.tm_min, startTm.tm_sec);
	sprintf_s(yyyyMMddhhiiss2, sizeof(yyyyMMddhhiiss), "%04d-%02d-%02d %02d_%02d_%02d"
		, breakTm.tm_year + 1900, breakTm.tm_mon + 1, breakTm.tm_mday
		, breakTm.tm_hour, breakTm.tm_min, breakTm.tm_sec);
	sprintf_s(fileName, sizeof(fileName), "RingBuffer Error Log_%s.txt", yyyyMMddhhiiss2);
	printf("%s\n", fileName);
	errno_t errCode = fopen_s(&file, fileName, "w");
	if (errCode != 0)
	{
		printf("file error %d\n", errCode);
		return;
	}

	fprintf_s(file, "[%s ~ %s][lineNo: %d] random seed: 0x%08llx, loop count: %lld", yyyyMMddhhiiss, yyyyMMddhhiiss2, lineNo, seedTime, loopCnt);
	fclose(file);
}