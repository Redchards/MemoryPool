# MemoryPool
Simple but suboptimal implementation of memory pool allocator in C++ 11

#The design
Like all other MemoryPool, we are first allocating chunks of memory by chunks of memory. Each chunk is linked by a std::vector<Node*_>, where Node_ is a simple union :

```
	union Node_
	{
		T* value_;
		Node_* next_;
	};
```
