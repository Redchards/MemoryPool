#ifndef ALLOCATOR
#define ALLOCATOR

/************************************************************************/
// Internet Software Consortium (ISC) License 
// Version 1, December 2015 
// 
// Copyright (C) 2015 Loic URIEN <urien.loic.cours@gmail.com> 
// 
// Permission to use, copy, modify, and/or distribute this software 
// for any purpose with or without fee is hereby granted, 
// provided that the above copyright notice and this permission notice 
// appear in all copies unless the author says otherwise. 
// 
// THE SOFTWARE IS PROVIDED "AS IS" 
// AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
// INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. 
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, 
// DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER 
// RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION 
// OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF 
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
/************************************************************************/

#include <CommonTypes.hxx>

#include <type_traits>
#include <cstdlib>
#include <limits>
#include <memory>
	
#ifdef DEBUG
#	define MEMORY_DEBUG true
#else
#	define MEMORY_DEBUG false
#endif // DEBUG	
	
// Some helper to simplify building of Allocator (even standards ones)
// @Note : why did I not make the IStdAllocationPolicy a true interface with virtual allocate and deallocate functions ?
// I'm aware that virtual function cost are negligible today, but allocate and deallocate could be called a lot
// (think about a particle system for instance, small object allocator), and I'm not willing to pay for that.
// Moreover, this is only helpers, not a true allocator system in any way
// Still, I find allocator easy enough to write


constexpr const char* defaultAllocatorName = "unknown";

template<typename T, template<class...> class DerivedPolicy>
class StlAllocationPolicy
{
	public:
	typedef T value_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef allocation_size_type size_type;
	typedef ptrdiff difference_type; 
	
	public: // Move this to the allocator class ?
	inline explicit StlAllocationPolicy() = default;
	inline explicit StlAllocationPolicy(const StlAllocationPolicy&) = default;
	inline explicit StlAllocationPolicy(StlAllocationPolicy&&) = default;
	template<typename U>
	inline explicit StlAllocationPolicy(const DerivedPolicy<U>&){};
	inline ~StlAllocationPolicy() = default;
	
	//template<typename U>
	//constexpr bool Equals(const DerivedPolicy<U>& ref) const{return static_cast<DerivedPolicy<T>>(this)->Equals(ref);}
	template<typename T1, typename T2>
	friend bool operator ==(const DerivedPolicy<T1>& lhs, const DerivedPolicy<T2>& rhs)
	{
		return lhs.equals(rhs);
	}
	
	template<typename T1, typename T2>
	friend bool operator !=(const DerivedPolicy<T1>& lhs, const DerivedPolicy<T2>& rhs)
	{
		return !(lhs == rhs);
	}
	
	constexpr inline size_type max_size() const
	{ 
        return std::numeric_limits<size_type>::max()/sizeof(T); 
    }
    
    pointer address(reference ref) const{ return std::addressof(ref); }
    
    const_pointer address(const_reference ref) const{ return std::addressof(ref); }
    
    size_type getAlignment(pointer ptr, size_type alignment)
    {
    	
    }
    
    template<typename U>
	struct rebind
	{
		typedef DerivedPolicy<U> other;
	};
};


// Default class. The only purpose is to demonstrate how easy it is to define a new allocation policy
// Derivating from IStdAllocationPolicy is optional, and used to define a standard allocation policy
// Custom container in Dawn only need methods implemented here.
// This method allow you to switch to a standard version of a custom allocator only by deriving.
// Note : You may also want to change types in StdAllocationPolicy, but as long as in general, size type and
// difference types are general throughout the program, method to change it without modifying the IStdAllocationPolicy
// is not implemented (will incure a bit more typing, and as it's already a bit complex, better not add anything)
// Easier to read, easier to implement, good for soul !
// This type is the default type for almost all container and allocators

// WARNING : Although an AllocationPolicy can have multiple template arguments, appart from the type, 
// every other should have default value !
template<typename T>
class DefaultAllocationPolicy 
    : public StlAllocationPolicy<T, DefaultAllocationPolicy>
{   
	public:
	using StlAllocationPolicy<T, ::DefaultAllocationPolicy>::StlAllocationPolicy;	
	// Inherit constructors, a bit ugly, need to figure out a better way to do it
	// BUT HEY ! WATCH OUT !!! OPERATOR NEW[] AND NEW ARE DIFFERENTS !! TAKE THIS IN ACCOUNT !!
	T* allocate(allocation_size_type size, const T* hint = nullptr)
	{
		return static_cast<T*>(operator new[](size * sizeof(T)));
	}
	void deallocate(T* ptr, allocation_size_type size)
	{
		operator delete[](ptr);
	}
	void construct(T* ptr, const T& ref)
	{
		new(ptr) T(ref); 
	}
	void destroy(T* ptr)
	{
		ptr->~T();
	}
	
	template<typename U>
	bool equals(DefaultAllocationPolicy<U>) const
	{
		return true;
	}
};

class ReleaseAllocatorPolicy
{
	public:
	inline explicit ReleaseAllocatorPolicy(const char* name = defaultAllocatorName){} // default
	inline explicit ReleaseAllocatorPolicy(const ReleaseAllocatorPolicy& other, const char* name = defaultAllocatorName){}
	inline ~ReleaseAllocatorPolicy() = default;
	
	const char* getName() const{return "";};
	void setName(const char* newName){};
};

class DebugAllocatorPolicy
{
	public:
	inline explicit DebugAllocatorPolicy(const char* name = defaultAllocatorName)
	: name_(name ? name : defaultAllocatorName){}
	inline explicit DebugAllocatorPolicy(const DebugAllocatorPolicy& other, const char* name = defaultAllocatorName)
	: name_(name ? name : defaultAllocatorName){}
	inline ~DebugAllocatorPolicy() = default;
	
	const char* getName() const{ return name_; }
	void setName(const char* newName){ name_ = newName; }
	
	protected:
	const char* name_;
};
// Conditionnaly add a name_ field depending on build mod, without #if macros
// DebugAllocatorPolicy and ReleaseAllocatorPolicy could also be extended
// A macro would be less code, but I hate macro in C++, I just use them when they're is nothing else to do 
// (or way too much code to avoid it)
template<typename T,
         class AllocationPolicy = DefaultAllocationPolicy<T>>
class Allocator 
    : public std::conditional<MEMORY_DEBUG, DebugAllocatorPolicy, ReleaseAllocatorPolicy>::type,
      public AllocationPolicy
{
	typedef std::conditional<MEMORY_DEBUG, DebugAllocatorPolicy, ReleaseAllocatorPolicy>::type ConfigPolicy;
	public:
	// Import constructors from parent class
	using ConfigPolicy::ConfigPolicy;
	using AllocationPolicy::AllocationPolicy;
	
	T* allocate(allocation_size_type size, const T* hint = nullptr)
	{
		return static_cast<AllocationPolicy*>(this)->allocate(size, hint);
	}
	
	void deallocate(T* ptr, allocation_size_type size)
	{
		static_cast<AllocationPolicy*>(this)->deallocate(ptr, size);
	}
	
	void construct(T* ptr, const T& ref)
	{
		static_cast<AllocationPolicy*>(this)->construct(ptr, ref);
	}
	
	void destroy(T* ptr)
	{
		static_cast<AllocationPolicy*>(this)->destroy(ptr);
	}
		
	template<typename U>
	struct rebind
	{
		typedef Allocator<U, typename AllocationPolicy::template rebind<U>::other> other;
	};
};



#endif // ALLOCATOR
