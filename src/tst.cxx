/************************************************************************/
//	Internet Software Consortium (ISC) License							//
//                   Version 1, December 2015							//
// 																		//
//  Copyright (C) 2015 Loic URIEN <urien.loic.cours@gmail.com>			//
// 																		//
//  Permission to use, copy, modify, and/or distribute this software	//
//  for any purpose with or without fee is hereby granted,				//
//  provided that the above copyright notice and this permission notice // 
//  appear in all copies unless the author says otherwise.				//
// 																		//
//           		THE SOFTWARE IS PROVIDED "AS IS"					//
// AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE	//
//    INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.	//
//        IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL,		//
// DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER	//
//   RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION	//
//	 OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF 	//
//	  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.	//
// 																		//								
/************************************************************************/


#include <MemoryPool.hxx>
#include <chrono>
#include <iostream>

// Some sample and performance measurement of the MemoryPool
MemoryPool<int> testPool;

static unsigned long x=123456789, y=362436069, z=521288629;
// Shamelessly taken from a random stackoverflow article
unsigned long randim(void) 
{
    unsigned long t;
   	x ^= x << 16;
   	x ^= x >> 5;
   	x ^= x << 1;

   	t = x;
   	x = y;
   	y = z;
   	z = t ^ x ^ y;

  	return z;
}

void launchTest()
{
	for(size_t i = 0; i < 100000; i++)
  	{
   		testPool.add(randim());
   	}
}

int main(int argc, char* argv[]) 
  {

   	MemoryPool<int> myMoryPool;
   	
   	myMoryPool.resize(10000);
   	myMoryPool.add(3);
   	myMoryPool.add(1545);
   	std::cout << myMoryPool.get(1) << std::endl;
   	std::cout << myMoryPool[0] << std::endl;
   	myMoryPool[0] = 5;
   	std::cout << myMoryPool[0] << std::endl;
   	std::cout << myMoryPool[50] << std::endl;
   	int sd = 4;
   	myMoryPool.add(std::move(sd));
   	std::cout << myMoryPool[2] << std::endl;
   	std::cout << sd << std::endl;
   	std::cout << 5/2 << std::endl;
   	myMoryPool.removeAt(2);
   	std::cout << myMoryPool[2] << std::endl;

  	std::chrono::time_point<std::chrono::system_clock> begin, end;
   	MemoryPool<int> newPool;
   	std::vector<int> newVector;
  	begin = std::chrono::system_clock::now();
  	for(size_t i = 0; i < 10000; i++)
  	{
   		newPool.add(randim());
   	}
   	end = std::chrono::system_clock::now();
   	std::chrono::duration<double> dur1 = end - begin;
   	MemoryPool<int> dsq(newPool);
   	for(size_t i = 0; i < 1000; i++)
   	{
   		std::cout << dsq[i] << std::endl;
   	}
   	newVector.reserve(4096);
   	begin = std::chrono::system_clock::now();
   	for(size_t i = 0; i < 10000; i++)
   	{
   		newVector.push_back(randim());
   	}
   	end = std::chrono::system_clock::now();
   	std::chrono::duration<double> dur2 = end - begin;
   	std::cout << "First = " << dur1.count() << std::endl;
   	std::cout << "Second = " << dur2.count() << std::endl;
  	return 0;
}

