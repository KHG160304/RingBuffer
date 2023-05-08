#include "RingBuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <windows.h>

#define SOURCE_PATTERN_STRING_CNT 81

const char strPattern[82] = "1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 12345";
char compstrPattern[82]   = "1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 12345";
char strPeek[82] = { 0, };
char strDequeue[82] = { 0, };

void logToFile(time_t seedTime, __int64 loopCnt, int lineNo);
int testCase1();
int testCase2();

int main(void)
{
	testCase1();
	//testCase2();
	/*printf("enqueue size: %d\n", rbufTest.Enqueue(nullptr, 100));
	printf("dequeue size: %d\n", rbufTest.Dequeue(nullptr, 20));
	printf("링버퍼 전체 크기%d\n", rbufTest.GetBufferSize());
	printf("링버퍼 사용 크기%d\n", rbufTest.GetUseSize());
	printf("링버퍼 가용 크기%d\n", rbufTest.GetFreeSize());*/


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