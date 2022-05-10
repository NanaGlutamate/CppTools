#ifndef _SRC_LINEARALGEBRA_MAT_H__
#define _SRC_LINEARALGEBRA_MAT_H__

#include <type_traits>
#include <memory>
#include <iostream>
#include <array>
#include <initializer_list>
#include <cmath>

using DEFAULT_ELEMENT = float;
template <typename _Ty>
using DEFAULT_ALLOCATOR = std::allocator<_Ty>;

template <typename T>
inline bool equal(const T& a, const T& b){
    return a == b;
};

template <>
inline bool equal<float>(const float& a, const float& b){
    //using std::fabs;
    //return abs(reinterpret_cast<uint32_t>(a-b))&(!uint32_t(0x03))==0;
    return fabs(a-b)<=1e-4;
}

template <>
inline bool equal<double>(const double& a, const double& b){
    //using std::fabs;
    //return abs(reinterpret_cast<uint64_t>(a-b))&(!uint64_t(0x03))==0;
    return fabs(a-b)<=1e-12;
}

template<size_t d1, size_t d2, size_t d3, typename E>
inline void mat_mul(const E e1[d1][d2], const E e2[d2][d3], E t[d1][d3]){
    #pragma omp parallel for
    for(size_t i = 0; i < d1; ++i){
        for(size_t j = 0; j < d2; ++j){
            E tmp = e1[i][j];
            for(size_t k = 0; k < d3; ++k){
                t[i][k] += tmp * e2[j][k];
            }
        }
    }
};

template<size_t d1, size_t d2, typename E>
inline void mat_add(const E e1[d1][d2], const E e2[d1][d2], E t[d1][d2]){
    #pragma omp parallel for
    for(size_t j = 0; j < d1*d2; ++j){
        t[0][j] = e1[0][j] + e2[0][j];
    }
};

template<size_t d1, size_t d2, typename E>
inline void mat_sub(const E e1[d1][d2], const E e2[d1][d2], E t[d1][d2]){
    #pragma omp parallel for
    for(size_t j = 0; j < d1*d2; ++j){
        t[0][j] = e1[0][j] - e2[0][j];
    }
};

template<size_t d1, size_t d2, typename E>
inline std::ostream& print_mat(const E m[d1][d2], std::ostream& os){
    for(size_t i=0;i<d1;++i){
        for(size_t j=0;j<d2;++j){
            os << m[i][j] << ",\t";
        }
        os << std::endl;
    }
    return os;
};

template <size_t d1, size_t d2, typename E=DEFAULT_ELEMENT, std::enable_if_t<!std::is_pointer_v<E>, int> = 0>
class mat{
public:
    E e[d1][d2];
    template <size_t side, typename OtherE>
    decltype(auto) operator*(const mat<d2, side, OtherE>& other)const{
        mat<d1, side, decltype(e[0][0]*other.e[0][0])> res={};
        mat_mul<d1, d2, side, E, OtherE, decltype(e[0][0]*other.e[0][0])>(e, other.e, res.e);
        return res;
    };
    friend std::ostream& operator<<(std::ostream& os, const mat<d1, d2, E>& m){
        return print_mat<d1, d2, E>(m.e, os);
    };
    mat operator+(const mat& other)const{
        mat res={};
        mat_add<d1, d2, E>(other.e, e, res.e);
        return res;
    };
    mat operator-(const mat& other)const{
        mat res={};
        mat_sub<d1, d2, E>(e, other.e, res.e);
        return res;
    };
    bool operator==(const mat& other)const{
        bool eq = true;
        for(size_t i=0;i<d1;++i){
            for(size_t j=0;j<d2;++j){
                eq = eq && equal(e[i][j], other.e[i][j]);
            }
            if(!eq)return false;
        }
        return true;
    }
    bool operator!=(const mat& other)const{
        return !(*this == other);
    }
};

template <size_t d1, size_t d2, typename E=DEFAULT_ELEMENT, template <typename T> typename _Alloc=DEFAULT_ALLOCATOR>
class heap_mat{
public:
    using Alloc =  _Alloc<mat<d1, d2, E>>;
private:
    Alloc allocator;
    mat<d1, d2, E> *e;
public:
    constexpr mat<d1, d2, E>* operator->(){return e;};

    constexpr const mat<d1, d2, E>* operator->()const{return e;};

    void set(E init_value){
        std::uninitialized_fill_n(&(e->e[0][0]), d1*d2, init_value);
    };

    heap_mat():allocator(),e(allocator.allocate(1)){
        //std::cout << e->e << "::construct(" << ")" << std::endl;
    };

    explicit heap_mat(E init_value):heap_mat(){set(init_value);};

private:
    explicit heap_mat(mat<d1, d2, E>* src):allocator(),e(src){
        //std::cout << e->e << "::manage(" << src << ")" << std::endl;
    };

public:
    template <template <typename T> typename _OtherAlloc=_Alloc>
    heap_mat(const heap_mat<d1, d2, E, _OtherAlloc>& other):heap_mat(){
        //std::cout << e->e << "::copy(" << other->e << ")" << std::endl;
        std::uninitialized_copy(&(other->e[0][0]), &(other->e[d1][0]), &(e->e[0][0]));
    };

    heap_mat(const heap_mat& other):heap_mat(){
        //std::cout << e->e << "::copy(" << other->e << ")" << std::endl;
        std::uninitialized_copy(&(other.e->e[0][0]), &(other.e->e[d1][0]), &(e->e[0][0]));
    };

    heap_mat(heap_mat&& other):allocator(other.allocator),e(other.e){
        //std::cout << e->e << "::move(" << other->e << ")" << std::endl;
        other.e = nullptr;
    }
    
    // template <typename OldE, std::enable_if_t<!std::is_same_v<OldE, E>, int> = 0>
    // heap_mat(heap_mat<d1, d2, OldE, _Alloc>&& other):allocator(),e(reinterpret_cast<mat<d1, d2, NewE>*>(dst.e)){
    //     other.e = nullptr;
    // }

    // TODO:
    // heap_mat(const std::initializer_list<std::array<E, d2>>& l):heap_mat(static_cast<E>(0)){
    //     //assert(l.size() <= d1);
    //     for(size_t i=0;i<l.size();++i){
    //         //assert(l[i].size() <= d1);
    //         std::uninitialized_copy(&((l.begin()+i)[0]), &((l.begin()+i)[d2]), &(e->e[i][0]));
    //     }
    // };
    // TODO: heap_mat(E src [d1][d2]):heap_mat(){std::uninitialized_copy(&(src[0][0]), &(src[d1][0]), &(e->e[0][0]));};

    ~heap_mat(){
        if(e){
            //std::cout << e->e << "::destruct(" << ")" << std::endl;
            allocator.deallocate(e, 1);
        }else{
            //std::cout << "0::destruct()" << std::endl;
        }
    }
    
    template <size_t side>
    heap_mat<d1, side, E, _Alloc> operator*(const heap_mat<d2, side, E, _Alloc>& other)const{
        heap_mat<d1, side, E, _Alloc> res{static_cast<E>(0)};
        mat_mul<d1, d2, side, E>(e->e, other.e->e, res.e->e);
        return std::move(res);
    };

    // TODO: small n*n matrix optimize, reduce 1 allocate, add 1 copy
    // template <std::enable_if_t<(d1==d2) && (d1*d1*sizeof(E)<64U), int> = 0>
    // friend decltype(auto) operator*(heap_mat&& fst, const heap_mat& sec){
    //     static heap_mat res;
    //     res.set(static_cast<E>(0));
    //     mat_mul<d1, d2, side, E>(fst.e->e, sec.e->e, res.e->e);
    //     std::uninitialized_copy(&(res.e->e[0][0]), &(res.e->e[d1][0]), &(fst->e[0][0]));
    //     return std::move(fst);
    // };
    //
    // template <std::enable_if_t<(d1==d2) && (d1*d1*sizeof(E)<64U), int> = 0>
    // friend decltype(auto) operator*(const heap_mat& fst, heap_mat&& sec){
    //     static heap_mat res;
    //     res.set(static_cast<E>(0));
    //     mat_mul<d1, d2, side, E>(fst.e->e, sec.e->e, res.e->e);
    //     std::uninitialized_copy(&(res.e->e[0][0]), &(res.e->e[d1][0]), &(sec->e[0][0]));
    //     return std::move(sec);
    // };

    friend std::ostream& operator<<(std::ostream& os, const heap_mat<d1, d2, E, _Alloc>& m){
        return print_mat<d1, d2, E>(m.e->e, os);
    };

    heap_mat operator+(const heap_mat& other)const{
        heap_mat res{static_cast<E>(0)};
        mat_add<d1, d2, E>(e->e, other.e->e, res.e->e);
        return std::move(res);
    };

    heap_mat operator+(heap_mat&& other)const{
        mat_add<d1, d2, E>(e->e, other.e->e, other.e->e);
        return std::move(other);
    };

    friend heap_mat operator+(heap_mat&& other, const heap_mat& s){
        mat_add<d1, d2, E>(s.e->e, other.e->e, other.e->e);
        return std::move(other);
    };

    heap_mat operator-(const heap_mat& other)const{
        heap_mat res{static_cast<E>(0)};
        mat_sub<d1, d2, E>(e->e, other.e->e, res.e->e);
        return std::move(res);
    };

    heap_mat operator-(heap_mat&& other)const{
        mat_add<d1, d2, E>(e->e, other.e->e, other.e->e);
        return std::move(other);
    };

    friend heap_mat operator-(heap_mat&& other, const heap_mat& s){
        mat_add<d1, d2, E>(other.e->e, s.e->e, other.e->e);
        return std::move(other);
    };

    bool operator==(const heap_mat& other)const{
        return (*e)==(*(other.e));
    };
    
    bool operator!=(const heap_mat& other)const{
        return (*e)!=(*(other.e));
    };

    heap_mat copy() const{
        heap_mat ret(*this);
        return std::move(ret);
    };

    template <typename NewE, size_t od1, size_t od2, typename _E,
                template <typename _Ty> typename __Alloc, 
                std::enable_if_t<sizeof(NewE)==sizeof(_E) && !(std::is_same_v<NewE, _E>), int>>
    friend heap_mat<od1, od2, NewE, __Alloc> astype(heap_mat<od1, od2, _E, __Alloc>&& dst);

    template <typename NewE, size_t od1, size_t od2, typename _E, 
                template <typename _Ty> typename __Alloc, 
                std::enable_if_t<std::is_same_v<NewE, _E>, int>>
    friend heap_mat<od1, od2, _E, __Alloc> astype(heap_mat<od1, od2, _E, __Alloc>&& dst);

    template <typename NewE, size_t od1, size_t od2, typename _E, 
                template <typename _Ty> typename __Alloc, 
                std::enable_if_t<!std::is_same_v<NewE, _E>, int>>
    friend heap_mat<od1, od2, NewE, __Alloc> astype(const heap_mat<od1, od2, _E, __Alloc>& dst);

    template <typename NewE, size_t od1, size_t od2, typename _E, 
                template <typename _Ty> typename __Alloc, 
                std::enable_if_t<std::is_same_v<NewE, _E>, int>>
    friend heap_mat<od1, od2, NewE, __Alloc> astype(const heap_mat<od1, od2, _E, __Alloc>& dst);

    template <size_t nd1, size_t nd2, size_t od1, size_t od2, typename _E, 
                template <typename _Ty> typename __Alloc, 
                std::enable_if_t<(nd1 * nd2)==(od1 * od2), int>>
    friend heap_mat<nd1, nd2, _E, __Alloc> reintrepret(heap_mat<od1, od2, _E, __Alloc>&& dst);

    template <size_t nd1, size_t nd2, size_t od1, size_t od2, typename _E, 
                template <typename _Ty> typename __Alloc, 
                std::enable_if_t<(nd1 * nd2)==(od1 * od2), int>>
    friend heap_mat<nd1, nd2, _E, __Alloc> reintrepret(const heap_mat<od1, od2, _E, __Alloc>& dst);
};

template <typename NewE, size_t od1, size_t od2, typename _E, template <typename _Ty> typename __Alloc, std::enable_if_t<sizeof(NewE)==sizeof(_E) && !(std::is_same_v<NewE, _E>), int> = 0>
heap_mat<od1, od2, NewE, __Alloc> astype(heap_mat<od1, od2, _E, __Alloc>&& dst){
    // std::cout << "NewE astype(E&&)" << std::endl;
    heap_mat<od1, od2, NewE, __Alloc> ret(reinterpret_cast<mat<od1, od2, NewE>*>(dst.e));
    dst.e = nullptr;
    _E* src = reinterpret_cast<_E*>(&(ret->e[0][0]));
    #pragma omp parallel for
    for(size_t i=0;i<od1*od2;++i)ret->e[0][i]=static_cast<NewE>(src[i]);
    return std::move(ret);
};

template <typename NewE, size_t od1, size_t od2, typename _E, template <typename _Ty> typename __Alloc, std::enable_if_t<std::is_same_v<NewE, _E>, int> = 0>
heap_mat<od1, od2, _E, __Alloc> astype(heap_mat<od1, od2, _E, __Alloc>&& dst){
    // std::cout << "E astype(E&&)" << std::endl;
    return std::move(dst);
};

template <typename NewE, size_t od1, size_t od2, typename _E, template <typename _Ty> typename __Alloc, std::enable_if_t<!std::is_same_v<NewE, _E>, int> = 0>
heap_mat<od1, od2, NewE, __Alloc> astype(const heap_mat<od1, od2, _E, __Alloc>& dst){
    // std::cout << "NewE astype(const E&)" << std::endl;
    heap_mat<od1, od2, NewE, __Alloc> cpy;
    #pragma omp parallel for
    for(size_t i=0;i<od1*od2;++i)cpy->e[0][i]=static_cast<NewE>(dst->e[0][i]);
    return std::move(cpy);
};

// TODO: may cause bug? after return dst, caller's memory been deallocate
template <typename NewE, size_t od1, size_t od2, typename _E, template <typename _Ty> typename __Alloc, std::enable_if_t<std::is_same_v<NewE, _E>, int> = 0>
heap_mat<od1, od2, NewE, __Alloc> astype(const heap_mat<od1, od2, _E, __Alloc>& dst){
    // std::cout << "E astype(const E&)" << std::endl;
    return dst;
};

// TODO:
template <size_t nd1, size_t nd2, size_t od1, size_t od2, typename _E, template <typename _Ty> typename __Alloc, std::enable_if_t<(nd1 * nd2)==(od1 * od2), int> = 0>
heap_mat<nd1, nd2, _E, __Alloc> reintrepret(heap_mat<od1, od2, _E, __Alloc>&& dst){
    heap_mat<nd1, nd2, _E, __Alloc> ret(reinterpret_cast<mat<nd1, nd2, _E>*>(dst.e));
    dst.e = nullptr;
    return std::move(ret);
};

template <size_t nd1, size_t nd2, size_t od1, size_t od2, typename _E, template <typename _Ty> typename __Alloc, std::enable_if_t<(nd1 * nd2)==(od1 * od2), int> = 0>
heap_mat<nd1, nd2, _E, __Alloc> reintrepret(const heap_mat<od1, od2, _E, __Alloc>& dst){
    heap_mat dst1(dst);
    using NewSizeMat_p = typename mat<nd1, nd2, _E>*;
    heap_mat<nd1, nd2, _E, __Alloc> ret(reinterpret_cast<NewSizeMat_p>(dst1.e));
    dst1.e = nullptr;
    return std::move(ret);
};

template<size_t d1, size_t d2, typename E, template <typename T> typename Alloc=DEFAULT_ALLOCATOR>
heap_mat<d1, d2, E, Alloc> to_heap(const mat<d1, d2, E>& src){
    heap_mat<d1, d2, E, Alloc> ret;
    std::uninitialized_copy(&(src.e[0][0]), &(src.e[d1][0]), &(ret->e[0][0]));
    return ret;
};

template<size_t d1, size_t d2, typename E, template <typename T> typename Alloc>
mat<d1, d2, E> to_stack(const heap_mat<d1, d2, E, Alloc>& src){
    return *(reinterpret_cast<mat<d1, d2, E>*>(src->e));
};

template <size_t d, typename E=DEFAULT_ELEMENT, template <typename T> typename Alloc=DEFAULT_ALLOCATOR>
heap_mat<d, d, E, Alloc> eyes(){
    heap_mat<d, d, E, Alloc> ret{static_cast<E>(0)};
    #pragma omp parallel for
    for(size_t i=0; i<d; ++i)ret->e[i][i]=static_cast<E>(1);
    return std::move(ret);
};

template <size_t d1, size_t d2, typename E=DEFAULT_ELEMENT, template <typename T> typename _Alloc=DEFAULT_ALLOCATOR>
heap_mat<d1, d2, E, _Alloc> zeros(){
    return heap_mat<d1, d2, E, _Alloc>{static_cast<E>(0)};
};

#endif
