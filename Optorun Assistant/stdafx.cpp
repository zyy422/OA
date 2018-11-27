// stdafx.cpp : source file that includes just the standard includes
// Optorun Assistant.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

//必须添加 ApiGpib1.lib 来进行 GPIB 编程链接.
//有趣的是 在 #include 中，表示目录的地方只用一个 反斜杠，而 #pragma 中却需要两个 反斜杠 
// #include "..\XMLMarkup\Markup.h"
// #pragma comment(lib,"..\\GpibSource\\ApiGpib1.lib")
