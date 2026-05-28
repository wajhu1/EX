#pragma once
#include <memory>
class Any{
public:
    // 类型擦除构造：把任意类型 T 包装成 Base 指针保存。
    template<typename T>
    Any(T data):_base(std::make_unique<Derive<T>>(data)){}
    Any(){}
    ~Any() = default;

    // Any 内部使用 unique_ptr 持有数据，因此禁止拷贝，只允许移动。
    Any(const Any&) = delete;
    Any& operator=(const Any&) = delete;
    Any(Any&&) = default;
    Any& operator=(Any&&) = default;

    // 按调用者指定的类型取出数据；类型不匹配时抛出异常。
    template<typename T>
    T cast_(){
        Derive<T> *sp = dynamic_cast<Derive<T>*>(_base.get());
        if(sp == nullptr){
            throw "type is unmatch!";
        }

        return sp->_data;
    }
private:

    class Base{
    public:
        // 需要虚析构函数，保证通过 Base 指针释放派生对象时行为正确。
        virtual ~Base() = default;
    };
    template<typename T>
    class Derive : public Base{
    public:
        Derive(T data):_data(data){}
        // 实际保存的具体类型数据。
        T _data;
    };

    // 指向被类型擦除后的真实数据对象。
    std::unique_ptr<Base> _base;

};
