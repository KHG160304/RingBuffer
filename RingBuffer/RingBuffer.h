#pragma once
#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__
class RingBuffer
{
public:
	RingBuffer(void);
	RingBuffer(int capacity);
	~RingBuffer();

	bool Resize(int capacity);
	int GetBufferSize();
	int GetUseSize(void);
	int GetFreeSize(void);
	int GetDirectEnqueueSize(void) const;
	int GetDirectDequeueSize(void) const;
	int Enqueue(const char* buffer, int size);
	int Dequeue(char* buffer, int size, int* rear = nullptr, int* front = nullptr, int peekSize = -1);
	int Peek(char* const buffer, int size, int* rear = nullptr, int* front = nullptr);
	int MoveRear(int size);
	int MoveFront(int size);
	void ClearBuffer(void);
	char* GetRearBufferPtr(void) const;
	char* GetFrontBufferPtr(void) const;
private:
	char* __internalBuffer;
	int __capacity;
	//int __useSize;
	//int __freeSize;
	int __queueFrontIndex;
	int __queueRearIndex;
};

#endif // !__RING_BUFFER_H__
