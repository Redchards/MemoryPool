# MemoryPool
Extremyl simple but suboptimal implementation of memory pool allocator in C++ 11. Basicaly just a toy code

#The design
Like all other MemoryPool, we are first allocating chunks of memory by chunks of memory. Each chunk is linked by a std::vector<Node*_>, where Node_ is a simple union :

```C++
	union Node_ // Don't mind the trailing dash, it's here to indicate that it is an internal thing
	{
		T* value_;
		Node_* next_;
	};
```
The interest of this technic is that, a Node_ is implicitly convertible to T*, so we can build a free list easily, using the memory allocated but not used. So, instead of a value of T, in each available memory address, we set the free list next pointers. Thus, each address retrieval, all we have to do is :

```C++
	Node_* returnPtr = nullptr;
	returnPtr = freeNode_ != nullptr ? freeNode_ : newBlock(capacity_);
	freeNode_ = freeNode_->next_;
```
It come at the cost of more pointer chasing, but has the advantage to enable simple removal. To remove an address, we simply need to say it's invalid, include it in the free list, and let it be then.

#Performances
Of course, as we already said, it could be much more powerful, but not without drawbacks. For example, arenas are an other kind of memory pool, but generaly don't allow deletion.
However, this is not as bad as it sounds. Even with this toy code, we get some "nice" results
You can see the code in "src/tst.cxx" and play with it.

|	|   MemoryPool<int>  | std::vector<int> |
|	|--------------------|------------------|
|	| Content Cell       | Content Cell     |
|------	| Content Cell       | Content Cell     |
