#ifndef __ZIP_HPP__
#define __ZIP_HPP__

#ifndef _CMATH_
template<typename E>
E min(E x, E y){
    return x < y ? x : y;
};
#endif

template <typename _F, typename... _Rest>
class zip: private zip<_Rest...>{
public:
    std::vector<_F>* fp;
    zip(std::vector<_F>& f, std::vector<_Rest>&... rest):fp(&f), zip<_Rest...>(rest...){};
    zip(const zip<_F, _Rest...>& other)=delete;
    zip(zip<_F, _Rest...>&& other):fp(other.fp), zip<_Rest...>(std::move((zip<_Rest...>)other)){};
    struct iter{
        size_t id;
        zip<_F, _Rest...>* self;
        iter operator++(int){
            iter tmp = *this;
            ++id;
            return tmp;
        }
        iter& operator++(){
            ++id;
            return *this;
        }
        bool operator==(const iter& other){
            return self == other.self && id == other.id;
        }
        bool operator!=(const iter& other){
            return !(*this == other);
        }
        std::tuple<_F&, _Rest&...> operator*(){
            return std::tuple<_F&, _Rest&...>(self->operator[](id));
        }
    };
    size_t size(){
        return min(fp->size(), zip<_Rest...>::size());
    };
    std::tuple<_F&, _Rest&...> operator[](size_t i){
        return std::tuple_cat(std::tuple<_F&>(fp->at(i)), zip<_Rest...>::operator[](i));
    };
    iter begin(){
        return iter{0, this};
    };
    iter end(){
        return iter{size(), this};
    };
};

template <typename _F>
class zip<typename _F>{
public:
    std::vector<_F>* fp;
    zip(std::vector<_F>& f):fp(&f){};
    zip(const zip<_F>& other)=delete;
    zip(zip<_F>&& other):fp(other.fp){};
    struct iter{
        size_t id;
        zip<_F>* self;
        iter operator++(int){
            iter tmp = *this;
            ++id;
            return tmp;
        }
        iter& operator++(){
            ++id;
            return *this;
        }
        bool operator==(const iter& other){
            return self == other.self && id == other.id;
        }
        bool operator!=(const iter& other){
            return !(*this == other);
        }
        std::tuple<_F&> operator*(){
            return std::tuple<_F&>(self->operator[](id));
        }
    };
    size_t size(){
        return fp->size();
    };
    std::tuple<_F&> operator[](size_t i){
        return std::tuple<_F&>{fp->at(i)};
    };
    iter begin(){
        return iter{0, this};
    };
    iter end(){
        return iter{size(), this};
    };
};

template<typename... _Args>
decltype(auto) make_zip(std::vector<_Args>&... args){
    return zip<_Args...>(args...);
};


#endif
