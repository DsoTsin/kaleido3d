#pragma once 
#ifndef __Library_hpp__
#define __Library_hpp__
#include <memory>
#include <string>

class Function;

#define sp std::shared_ptr

class Library {
public:
	
	sp<Function> newFunctionWithName(std::string name);

private:
	std::string *functionNames;
};

#endif