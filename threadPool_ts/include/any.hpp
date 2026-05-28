#pragma once
#include <memory>

class Any{
public:
    Any() = default;
    ~Any() = default;
    Any(const Any&) = delete;
    Any& operator=(const Any&) = delete;
    Any(Any&&) = default;
    Any& operator=(Any&&) = default;
    
    template<typename T>
    Any(T base): _base(std::make_unique<Derive<T>>(base)){

    }
    //返回存储的数据类型的指
    template<typename T>
    T cast_(){
        Derive<T> *sp = dynamic_cast<Derive<T>*>(_base.get());
        if(!sp){
            throw "type is unmath";
        }

        return sp->_data;
    }
private:
    //基类用于指向各种类型
    class Base{
    public:
        virtual ~Base() = default;

    };
    //子类保存类型数据，数据类型通过模版确定
    template<typename T>
    class Derive : public Base{
    public:
        Derive(T data):_data(data){

        }
        T _data;
    };
    //保存的数据类型
    std::unique_ptr<Base> _base;


};