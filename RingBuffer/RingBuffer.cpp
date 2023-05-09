
#include "RingBuffer.h"
#include <memory.h>

static int defaultCapacity = sizeof(size_t);//1000;//1048576; // 1mb : 1024byte(1kb) * 1024
static int minCapacity = 2; // 100byte is minimum size

RingBuffer::RingBuffer(void)
	: __capacity(defaultCapacity)
	, __useSize(0)
	, __freeSize(defaultCapacity)
	, __internalBuffer(new char[defaultCapacity])
	, __queueFrontIndex(0)
	, __queueRearIndex(0)
{
}

RingBuffer::RingBuffer(int capacity)
	: __useSize(0)
	, __queueFrontIndex(0)
	, __queueRearIndex(0)
{
	if (capacity < minCapacity)
	{
		this->__capacity = minCapacity;
	}
	else
	{
		this->__capacity = capacity;
	}
	this->__freeSize = this->__capacity;
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
	return this->__capacity;
}

int RingBuffer::GetUseSize(void)
{
	return this->__useSize;
}

int RingBuffer::GetFreeSize()
{
	return this->__freeSize;
}

int RingBuffer::GetDirectEnqueueSize(void) const
{
	if (!this->__freeSize)
	{
		return 0;
	}

	if (this->__queueRearIndex >= this->__queueFrontIndex)
	{
		return this->__capacity - this->__queueRearIndex;
	}
	else
	{
		return this->__queueFrontIndex - this->__queueRearIndex;
	}
}

int RingBuffer::GetDirectDequeueSize(void) const
{
	if (!this->__useSize)
	{
		return 0;
	}

	if (this->__queueRearIndex > this->__queueFrontIndex)
	{
		return this->__queueRearIndex - this->__queueFrontIndex;
	}
	else
	{
		return this->__capacity - this->__queueFrontIndex;
	}
}

int RingBuffer::Enqueue(const char* data, int size)
{ 
	if (this->__freeSize < size || size < 1)
	{
		return 0;
	}

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
	this->__freeSize -= size;
	this->__useSize += size;
	return size;
}

int RingBuffer::Dequeue(char* const buffer, int size)
{
	if (this->__useSize < size || size < 1)
	{
		return 0;
	}

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
	this->__freeSize += size;
	this->__useSize -= size;
	return size;
}

int RingBuffer::Peek(char* const buffer, int size)
{
	if (this->__useSize < size || size < 1)
	{
		return 0;
	}

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
	if (this->__freeSize < size || size < 1)
	{
		return 0;
	}
	this->__queueRearIndex = (this->__queueRearIndex + size) % this->__capacity;
	this->__freeSize -= size;
	this->__useSize += size;
	return size;
}

int RingBuffer::MoveFront(int size)
{
	if (this->__useSize < size || size < 1)
	{
		return 0;
	}

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
	this->__freeSize += size;
	this->__useSize -= size;
	return size;
}

void RingBuffer::ClearBuffer(void)
{
	this->__useSize = 0;
	this->__freeSize = this->__capacity;
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