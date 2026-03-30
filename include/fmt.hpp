#pragma once
//如需使用fmt
#include "fmt/format.h"

#ifdef  _WIN64

#ifdef  NDEBUG
#pragma comment (lib,"x64/fmt.lib")
#else
#pragma comment (lib,"x64/fmtd.lib")
#endif

#else

#ifdef NDEBUG
#pragma comment (lib,"x86/fmt.lib")
#else
#pragma comment (lib,"x86/fmtd.lib")
#endif // !_DEBUG

#endif