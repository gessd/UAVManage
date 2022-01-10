#pragma once

//libhvÈý·½¿â
#include "TcpClient.h"
#include "htime.h"
#ifdef _DEBUG
#pragma comment(lib, "./libhv/lib/hv_staticd.lib") 
#else
#pragma comment(lib, "./libhv/lib/hv_static.lib") 
#endif // _DEBUG
