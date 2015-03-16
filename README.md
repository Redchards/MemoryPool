# MemoryPool
Extremyl simple but suboptimal implementation of memory pool allocator in C++ 11

#The design
Like all other MemoryPool, we are first allocating chunks of memory by chunks of memory. Each chunk is linked by a std::vector<Node*_>, where Node_ is a simple union :

```C++
	union Node_ // Don't mind the trailing dash, it's here to indicate that it is an internal thing
	{
		T* value_;
		Node_* next_;
	};
```
The interest of this technic is that, a Node_ is implicitly convertible to T*, so we can build a free list easily, using the memory allocated but not used. So, instead of a value of T, in each available memory address, we set the free list next pointers. Thus, each allocation, all we have to do is

```C++
	Node_* returnPtr = nullptr;
	returnPtr = freeNode_ != nullptr ? freeNode_ : newBlock(capacity_);
	freeNode_ = freeNode_->next_;
```
