#ifndef MEMORYPOOL
#define MEMORYPOOL

/************************************************************************/
//	Internet Software Consortium (ISC) License			//
//                   Version 1, December 2015				//
// 									//
//  Copyright (C) 2015 Loic URIEN <urien.loic.cours@gmail.com>		//
// 									//
//  Permission to use, copy, modify, and/or distribute this software	//
//  for any purpose with or without fee is hereby granted,		//
//  provided that the above copyright notice and this permission notice // 
//  appear in all copies unless the author says otherwise.		//
// 									//
//           	       THE SOFTWARE IS PROVIDED "AS IS"	        	//
// AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE	//
//    INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.	//
//        IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL,	//
// DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER	//
//   RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION	//
//    OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF 	//
//     OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.   //								
/************************************************************************/

#include <Allocator.hxx>
#include <memory>
#include <cassert>
#include <vector>

template<typename T, 
		 class AllocationPolicy = DefaultAllocationPolicy<T>,
		 bool aligned = true>
class BasicMemoryPoolAllocationPolicy 
    : public AllocationPolicy::template rebind<T*>::other
{
	protected:
	union Node_;
	typedef typename AllocationPolicy::template rebind<T*>::other Alloc;
	
	public:
	explicit BasicMemoryPoolAllocationPolicy(allocation_size_type capacity = 4096);
	explicit BasicMemoryPoolAllocationPolicy(const BasicMemoryPoolAllocationPolicy& other);
	explicit BasicMemoryPoolAllocationPolicy(BasicMemoryPoolAllocationPolicy&& other);
	~BasicMemoryPoolAllocationPolicy();
	
	BasicMemoryPoolAllocationPolicy& operator=(const BasicMemoryPoolAllocationPolicy& other);
	BasicMemoryPoolAllocationPolicy& operator=(BasicMemoryPoolAllocationPolicy&& other);
	
	T* allocate(allocation_size_type size, const T* hint = nullptr);
	void deallocate(T* ptr, allocation_size_type size);
	
	allocation_size_type getCapacity() const
	{
		return capacity_;
	}
	
	private:
		
	template<class S, S val>
	struct ValueHolder
	{
		constexpr static S value = val;
	};
	
	template<uint8 val>
	using ValueOf = ValueHolder<uint8, val>;

	protected:
	template<bool isAligned = aligned>
	typename std::enable_if<isAligned, Node_>::type* newBlock(allocation_size_type size);
	
	template<bool isAligned = aligned>
	typename std::enable_if<!isAligned, Node_>::type* newBlock(allocation_size_type size);
	
	void buildFreeList();

	protected:
	template<typename U>
	struct rebind
	{
		typedef BasicMemoryPoolAllocationPolicy<U, typename AllocationPolicy::template rebind<U>::other> other;
	};
	
	protected:
	union Node_
	{
		T* value_;
		Node_* next_;
	};
	
	protected:
	allocation_size_type capacity_;
	Node_* freeNode_;
	Node_* currentNode_;
	std::vector<Node_*> firstNode_;
	int64 currentBlock_;
	
	static constexpr uint64 alignement = std::conditional<alignof(T) <= sizeof(std::max_align_t), ValueOf<alignof(T)>, ValueOf<sizeof(std::max_align_t)>>::type::value;
	
	//static constexpr uint8 offset = (offset + alignment - 1) & ~(alignment -1); 
};

template<typename T,
		 class AllocationPolicy,
		 bool aligned>
BasicMemoryPoolAllocationPolicy<T, AllocationPolicy, aligned>::BasicMemoryPoolAllocationPolicy(allocation_size_type capacity) 
: capacity_(capacity),
  freeNode_(nullptr),
  currentNode_(nullptr),
  currentBlock_(-1),
  firstNode_()
{
	newBlock(capacity);
}

template<typename T,
		 class AllocationPolicy,
		 bool aligned>
BasicMemoryPoolAllocationPolicy<T, AllocationPolicy, aligned>::BasicMemoryPoolAllocationPolicy(const BasicMemoryPoolAllocationPolicy& other) 
: capacity_(other.capacity_),
  freeNode_(other.freeNode_),
  currentNode_(other.currentNode_),
  currentBlock_(other.currentBlock_),
  firstNode_()
{
	for(auto e : other.firstNode_)
	{
		newBlock(capacity_);
		std::copy(*other.firstNode_.begin(), *other.firstNode_.begin() + capacity_, currentNode_);
	}
}

template<typename T,
		 class AllocationPolicy,
		 bool aligned>
BasicMemoryPoolAllocationPolicy<T, AllocationPolicy, aligned>::BasicMemoryPoolAllocationPolicy(BasicMemoryPoolAllocationPolicy&& other) 
: capacity_(other.capacity_),
  freeNode_(other.freeNode_),
  currentNode_(other.currentNode_),
  currentBlock_(other.currentBlock_),
  firstNode_(other.firstNode_)
{
	other.firstNode_.clear();
	other.freeNode_ = nullptr;
	other.currentNode_ = nullptr;
	other.currentBlock_ = 0;
}


template<typename T,
		 class AllocationPolicy,
		 bool aligned>
BasicMemoryPoolAllocationPolicy<T, AllocationPolicy, aligned>::~BasicMemoryPoolAllocationPolicy()
{
	for(auto e : firstNode_)
	{
		operator delete(reinterpret_cast<T*>(e));
	}
}


template<typename T,
		 class AllocationPolicy,
		 bool aligned>
template<bool isAligned>
typename std::enable_if<!isAligned, typename BasicMemoryPoolAllocationPolicy<T, AllocationPolicy, aligned>::Node_>::type*
BasicMemoryPoolAllocationPolicy<T, AllocationPolicy, aligned>::newBlock(allocation_size_type size)
{
	currentNode_ = static_cast<Node_*>(operator new(size * sizeof(Node_) + sizeof(Node_)));
	
	firstNode_.push_back(currentNode_);
	buildFreeList();
	
	return currentNode_;
}

template<typename T,
		 class AllocationPolicy,
		 bool aligned>
template<bool isAligned>
typename std::enable_if<isAligned, typename BasicMemoryPoolAllocationPolicy<T, AllocationPolicy, aligned>::Node_>::type*
BasicMemoryPoolAllocationPolicy<T, AllocationPolicy, aligned>::newBlock(allocation_size_type size)
{
	currentNode_ = static_cast<Node_*>(operator new(size * sizeof(Node_) + sizeof(Node_)*2));

	currentNode_ += (-reinterpret_cast<uintptr_t>(currentNode_)) & (alignement - 1);
	
	firstNode_.push_back(currentNode_);
	buildFreeList();
	
	return currentNode_;
}

template<typename T,
		 class AllocationPolicy,
		 bool aligned>
void BasicMemoryPoolAllocationPolicy<T, AllocationPolicy, aligned>::buildFreeList()	
{
	Node_* currentNode = currentNode_;
	Node_* tmp = freeNode_;
	
	for(allocation_size_type i = 0 ; i < capacity_-1; ++i)
	{
		freeNode_ = currentNode;
		freeNode_->next_ = ++currentNode;
	}
	freeNode_->next_ = nullptr;
	freeNode_ = tmp != nullptr ? tmp : currentNode_;
}
template<typename T,
		 class AllocationPolicy,
		 bool aligned>
T* BasicMemoryPoolAllocationPolicy<T, AllocationPolicy, aligned>::allocate(allocation_size_type size, const T* hint)
{
	Node_* returnPtr = nullptr;
	
	returnPtr = freeNode_ != nullptr ? freeNode_ : newBlock(capacity_);
	freeNode_ = freeNode_->next_;
	return reinterpret_cast<T*>(returnPtr);
}

// Do not rely on this to initialize memory to 0 after "deletion".
// After deletion, it will only update free nodes implicit list
template<typename T,
		 class AllocationPolicy,
		 bool aligned>
void BasicMemoryPoolAllocationPolicy<T, AllocationPolicy, aligned>::deallocate(T* ptr, allocation_size_type size)
{
#	ifdef	DEBUG
	bool t = false;
	for(auto it = firstNode_.begin(); (it != firstNode_.end()) && !t; ++it)
	{
		t = ((reinterpret_cast<Node_*>(ptr) - *it + size - 1) <= capacity_);
	}
	assert(t == true); // need true assert here, with message
#	endif

	Node_* tmp;
	for(ptrdiff i = 0; i != size; ++i)
	{
		tmp = freeNode_;
		freeNode_ = reinterpret_cast<Node_*>(ptr);
		(*reinterpret_cast<T*>(freeNode_)).~T();
		freeNode_->next_= tmp;
	}
}


// Alias for the memory pool allocator 
// Todo : Switch to thread-safe version if requested
template<typename T, 
		 class AllocationPolicy = DefaultAllocationPolicy<T>,
		 bool aligned = true>
using BasicMemoryPoolAllocator = Allocator<T, BasicMemoryPoolAllocationPolicy<T, AllocationPolicy, aligned>>;

// Basically just adding a subset of std::vector functionalities
template<typename T,
		 class AllocationPolicy = BasicMemoryPoolAllocator<T, DefaultAllocationPolicy<T>>>
class MemoryPool : public AllocationPolicy
{
	using Allocator = AllocationPolicy;
	
	public:
	explicit MemoryPool(allocation_size_type capacity = 4096) 
	: Allocator(capacity),
	  size_(0)
	{}
	
	explicit MemoryPool(MemoryPool const& other)
	 : Allocator(other),
	   size_(other.size_)
	{}
	
	explicit MemoryPool(MemoryPool&& other) 
	: Allocator(other), 
	  size_(other.size_)
	{
		size_ = 0;
	}
	~MemoryPool() = default;
	
	template<typename U>
	void add(U&& element);
	
	void remove(T* elementPtr);
	void removeAt(allocation_size_type position);
	void removeFirst();
	void removeLast();
	
	void resize(allocation_size_type size);
	
	T get(allocation_size_type position) const;
	T& operator[](allocation_size_type position) const;
	
	bool isDeleted(allocation_size_type position) const;
	bool isDeleted(T* ptr) const;

	allocation_size_type getSize()
	{
		return size_;
	}
	
	private:
	size_t size_;
};

// Test if atomic or object and call the right method
template<typename T,
		 class AllocationPolicy>
template<typename U>
inline void MemoryPool<T, AllocationPolicy>::add(U&& element)
{
	*Allocator::allocate(1) = std::forward<T>(element);
	++size_;
}

template<typename T,
		 class AllocationPolicy>
void MemoryPool<T, AllocationPolicy>::remove(T* elementPtr)
{
	Allocator::deallocate(elementPtr, 1);
}

template<typename T,
		 class AllocationPolicy>
void MemoryPool<T, AllocationPolicy>::removeAt(allocation_size_type position)
{
	Allocator::deallocate(reinterpret_cast<T*>(Allocator::firstNode_[position / Allocator::capacity_] + position), 1);
}

template<typename T, 
		 class AllocationPolicy>
void MemoryPool<T, AllocationPolicy>::removeFirst()
{
	removeAt(0);
}

template<typename T,
		 class AllocationPolicy>
void MemoryPool<T, AllocationPolicy>::removeLast()
{
	remove(Allocator::currentNode_);
}

template<typename T,
		 class AllocationPolicy>
void MemoryPool<T, AllocationPolicy>::resize(allocation_size_type size)
{
	size_ = size;
	uint64 offset = (size / Allocator::capacity_) - Allocator::firstNode_.size();
	for(uint64 i = 0; i < offset; ++i)
	{
		Allocator::newBlock(Allocator::capacity_);
	}
}

template<typename T,
		 class AllocationPolicy>
T MemoryPool<T, AllocationPolicy>::get(allocation_size_type position) const
{
	return operator[](position);
}

template<typename T,
		 class AllocationPolicy>
T& MemoryPool<T, AllocationPolicy>::operator[](allocation_size_type position) const
{
	typedef typename Allocator::Node_ Node;

	assert(position <= (size_ * Allocator::firstNode_.size()));
	
	uint64 offset = (position / Allocator::capacity_);
	return *(reinterpret_cast<T*>((Allocator::firstNode_[offset] + position - (offset * Allocator::capacity_))));
}

template<typename T,
		 class AllocationPolicy>
bool MemoryPool<T, AllocationPolicy>::isDeleted(allocation_size_type position) const
{
	return isDeleted(&operator[](position));
}

template<typename T,
		 class AllocationPolicy>
bool MemoryPool<T, AllocationPolicy>::isDeleted(T* ptr) const
{
	typedef typename Allocator::Node_ Node;
	Node* tmp = Allocator::freeNode_;
	while(tmp != nullptr)
	{
		if(reinterpret_cast<T*>(tmp) == ptr)
		{
			return true;
		}
		tmp = tmp->next_;
	}
	return false;
}


#endif // MEMORYPOOL
