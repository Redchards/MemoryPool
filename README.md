# MemoryPool
Extremly simple but suboptimal implementation of memory pool allocator in C++ 11. Basicaly just a toy code

#Design
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
I compiled the code with clang++.
These results are given on average of 100 runs, wit a loop of 100000000 iterations, using pseudo-random integer numbers generated with simple, fast function, using -O2 optimization :

|             |   MemoryPool<int>  | std::vector<int> |
|-------------|--------------------|------------------|
|    add      | 0.77s              | 0.80s            |

So, not enough improvement to justify anything. Moreover, using -O3 optimizations, we get a (bad) surprise

|             |   MemoryPool<int>  | std::vector<int> |
|-------------|--------------------|------------------|
|    add      | 1.08s              | 0.77s            |

It may be due to compiler trying to be smart with prefetching, but invalidating it each time because of pointer chasing. I would have been happy with just same performances, or slightly worse than std::vector in this particular case.

#Build
Simply using 
```
	make // For default debug config
	make config=release // For actual release config. Should do performance tests on this one
	make config=debug/release clean // To clean .obj and generated binaries
```
Build results can then be found under ./bin/$config/ where $config is either debug or release

#Summary
It was a fun study case, but definitly not worth it. However, it proved that for the majority of cases, std::vector is more than fast enough. Current impementation (at least on clang and gcc) is very fast.
Maybe some other technics, like pointer bumping, and make the deletion feature goes away could vastly improve performances of such container. But this is another story ...
However, if you think there's something that can vatly improve this implementation, just let me know.

#License
Here is the license. But if for some reason you'd like to use my code (u mad ?) but don't want to include the license notice, please just refer to me.

```
		Internet Software Consortium (ISC) License							
	                   Version 1, December 2015							
	 																		
	  Copyright (C) 2015 Loic URIEN <urien.loic.cours@gmail.com>			
	 																		
	  Permission to use, copy, modify, and/or distribute this software	
	  for any purpose with or without fee is hereby granted,				
	  provided that the above copyright notice and this permission notice 
	  appear in all copies unless the author says otherwise.				
 														
	  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
   	  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
	  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
	  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
	  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
	  OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
	  CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
	
```																						

