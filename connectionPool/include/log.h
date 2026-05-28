#pragma once
#define log(str) \
    std::cout << __FILE__ << ":" << __LINE__ <<" " << \
    __TIMESTAMP__ << ":" <<str <<std::endl;