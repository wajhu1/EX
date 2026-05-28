#pragma once
// 简单日志宏：输出文件名、行号、编译时间戳和日志内容。
#define log(str) \
    std::cout << __FILE__ << ":" << __LINE__ <<" " << \
    __TIMESTAMP__ << ":" <<str <<std::endl;
