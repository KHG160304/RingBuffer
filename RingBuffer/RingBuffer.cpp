
#include "RingBuffer.h"
#include <memory.h>

static int defaultCapacity = sizeof(size_t);//1000;//1048576; // 1mb : 1024byte(1kb) * 1024
static int minCapacity = 2; // 100byte is minimum size

RingBuffer::RingBuffer(void)
	: __capacity(defaultCapacity + 1)
	//, __useSize(0)
	//, __freeSize(defaultCapacity)
	, __internalBuffer(new char[defaultCapacity])
	, __queueFrontIndex(0)
	, __queueRearIndex(0)
{
}

RingBuffer::RingBuffer(int capacity)
	//: __useSize(0)
	//, 
	: __queueFrontIndex(0)
	, __queueRearIndex(0)
{
	if (capacity < minCapacity)
	{
		this->__capacity = minCapacity + 1;
	}
	else
	{
		this->__capacity = capacity + 1;
	}
	//this->__freeSize = this->__capacity;
	this->__internalBuffer = new char[this->__capacity];
}

RingBuffer::~RingBuffer()
{
	delete[] __internalBuffer;
}

bool RingBuffer::Resize(int size)
{
	return false;
}

int RingBuffer::GetBufferSize()
{
	return this->__capacity - 1;
}

int RingBuffer::GetUseSize(void)
{
	/*
		_load* 변수의 용도
		멀티스레딩시 한쪽은 Enqueue, 한쪽은 Dequeue 할때
		GetUseSize 로직이 작동하는 중에, 다른 스레드에 의해서
		변수의 값이 변경될 것을 우려하여, 함수를 호출한 스레드의
		지역변수에 값을 로드하여 사용한다.
		공유 메모리 사용을 하지 않기위해서 이다.
	*/
	int _loadRearIndex = this->__queueRearIndex;
	int _loadFrontIndex = this->__queueFrontIndex;
	if (_loadRearIndex >= _loadFrontIndex)
	{
		return (_loadRearIndex - _loadFrontIndex);
	}
	else
	{
		return this->__capacity - (_loadFrontIndex - _loadRearIndex);
	}
	//return this->__useSize;
}

int RingBuffer::GetFreeSize()
{
	/*
		_load* 변수의 용도
		멀티스레딩시 한쪽은 Enqueue, 한쪽은 Dequeue 할때
		GetFreeSize 로직이 작동하는 중에, 다른 스레드에 의해서
		변수의 값이 변경될 것을 우려하여, 함수를 호출한 스레드의
		지역변수에 값을 로드하여 사용한다. 
		공유 메모리 사용을 하지 않기위해서 이다.
	*/
	int _loadRearIndex = this->__queueRearIndex;
	int _loadFrontIndex = this->__queueFrontIndex;
	if (_loadRearIndex >= _loadFrontIndex)
	{
		return this->__capacity - (_loadRearIndex - _loadFrontIndex) - 1;
	}
	else
	{
		return (_loadFrontIndex - _loadRearIndex) - 1;
	}
	//return this->__capacity - this->__useSize;//this->__freeSize;
}

int RingBuffer::GetDirectEnqueueSize(void) const
{
	int _loadRearIndex = this->__queueRearIndex;
	int _loadFrontIndex = this->__queueFrontIndex;
	if (_loadRearIndex >= _loadFrontIndex)
	{
		return this->__capacity - _loadRearIndex - 1;
	}
	else
	{
		return _loadFrontIndex - _loadRearIndex - 1;
	}
}

int RingBuffer::GetDirectDequeueSize(void) const
{
	int _loadRearIndex = this->__queueRearIndex;
	int _loadFrontIndex = this->__queueFrontIndex;
	if (_loadRearIndex >= _loadFrontIndex)
	{
		return _loadRearIndex - _loadFrontIndex;
	}
	else
	{
		return this->__capacity - _loadFrontIndex;
	}
}

int RingBuffer::Enqueue(const char* data, int size)
{ 
	if (this->GetFreeSize() < size || size < 1)
	{
		return 0;
	}
	/*if (this->__useSize + size > this->__capacity || size < 1)
	{
		return 0;
	}*/
	/*if (this->__freeSize < size || size < 1)
	{
		return 0;
	}*/
	//freeSize;
	

	int nextRearIndex = (this->__queueRearIndex + size) % this->__capacity;
	char* ptrCopyStart = this->__internalBuffer + this->__queueRearIndex;
	if (nextRearIndex <= this->__queueRearIndex && nextRearIndex != 0)
	{
		memcpy(ptrCopyStart, data, size - nextRearIndex);
		memcpy(this->__internalBuffer, data + (size - nextRearIndex), nextRearIndex);
	}
	else
	{
		memcpy(ptrCopyStart, data, size);
	}

	this->__queueRearIndex = nextRearIndex;
	//this->__freeSize -= size;
	//this->__useSize += size;
	return size;
}

#include <stdio.h>
int RingBuffer::Dequeue(char* const buffer, int size, int* rear, int* front, int peekSize)
{
	if (rear != nullptr && front != nullptr)
	{
		*rear = this->__queueRearIndex;
		*front = this->__queueFrontIndex;
	}
	int a = this->GetUseSize();
	if (a < size || size < 1)
	{
		if (peekSize > 0)
		{
			printf("test\n");
		}
		return 0;
	}
	/*if (this->__useSize < size || size < 1)
	{
		return 0;
	}*/

	//int copySize;
	//if (this->__useSize < size)
	//{
	//	copySize = this->__useSize;
	//}
	//else
	//{
	//	copySize = size;
	//}

	int nextFrontIndex = (this->__queueFrontIndex + size) % this->__capacity;
	char* ptrCopyStart = this->__internalBuffer + this->__queueFrontIndex;
	if (nextFrontIndex <= this->__queueFrontIndex && nextFrontIndex != 0)
	{
		memcpy(buffer, ptrCopyStart, size - nextFrontIndex);
		memcpy(buffer + (size - nextFrontIndex), this->__internalBuffer, nextFrontIndex);
	}
	else
	{
		memcpy(buffer, ptrCopyStart, size);
	}

	this->__queueFrontIndex = nextFrontIndex;
	//this->__freeSize += size;
	//this->__useSize -= size;
	return size;
}

int RingBuffer::Peek(char* const buffer, int size, int* rear, int* front)
{
	if (rear != nullptr && front != nullptr)
	{
		*rear = this->__queueRearIndex;
		*front = this->__queueFrontIndex;
	}
	if (this->GetUseSize() < size || size < 1)
	{
		return 0;
	}
	/*if (this->__useSize < size || size < 1)
	{
		return 0;
	}*/

	/*int copySize;
	if (this->__useSize < size)
	{
		copySize = this->__useSize;
	}
	else
	{
		copySize = size;
	}*/

	int nextFrontIndex = (this->__queueFrontIndex + size) % this->__capacity;
	char* ptrCopyStart = this->__internalBuffer + this->__queueFrontIndex;
	if (nextFrontIndex <= this->__queueFrontIndex && nextFrontIndex != 0)
	{
		memcpy(buffer, ptrCopyStart, size - nextFrontIndex);
		memcpy(buffer + (size - nextFrontIndex), this->__internalBuffer, nextFrontIndex);
	}
	else
	{
		memcpy(buffer, ptrCopyStart, size);
	}

	return size;
}

int RingBuffer::MoveRear(int size)
{
	if (this->GetFreeSize() < size || size < 1)
	{
		return 0;
	}
	/*if (this->__useSize + size > this->__capacity || size < 1)
	{
		return 0;
	}*/
	this->__queueRearIndex = (this->__queueRearIndex + size) % this->__capacity;
	//this->__freeSize -= size;
	//this->__useSize += size;
	return size;
}

int RingBuffer::MoveFront(int size)
{
	if (this->GetUseSize() < size || size < 1)
	{
		return 0;
	}
	/*if (this->__useSize < size || size < 1)
	{
		return 0;
	}*/

	/*int moveSize;
	if (this->__useSize < size)
	{
		moveSize = this->__useSize;
	}
	else
	{
		moveSize = size;
	}*/

	this->__queueFrontIndex = (this->__queueFrontIndex + size) % this->__capacity;
	//this->__freeSize += size;
	//this->__useSize -= size;
	return size;
}

void RingBuffer::ClearBuffer(void)
{
	//this->__useSize = 0;
	//this->__freeSize = this->__capacity;
	this->__queueRearIndex = 0;
	this->__queueFrontIndex = 0;
}

char* RingBuffer::GetRearBufferPtr(void) const
{
	return this->__internalBuffer + this->__queueRearIndex;
}

char* RingBuffer::GetFrontBufferPtr(void) const
{
	return this->__internalBuffer + this->__queueFrontIndex;
}