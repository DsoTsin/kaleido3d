#pragma once

#include <iostream>
#include <vector>
#include <list>
#include <thread>
#include <string>
#include <memory>
#include <map>
#include <unordered_map>
#include <initializer_list>
#include <functional>
#include <cassert>

#if K3DPLATFORM_OS_WIN
typedef std::wstring kString;
#else
typedef std::string kString;
#endif