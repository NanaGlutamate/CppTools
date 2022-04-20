#ifndef __VECTORN_HPP__
#define __VECTORN_HPP__

#include <array>
#include <cmath>

template <typename E, int N>
class VectorN {
public:
    std::array<E, N> data;

    // template <typename... Args>
    // VectorN(Arg...args): data({std::forward<std::enable_if<sizeof...(Args)==N, E>::type>(args),...}) {};
    // VectorN():data() {};
    // VectorN(const VectorN& other) : data(other.data) {};
    // VectorN(VectorN&& other) : data(other.data) {};
    
    VectorN& operator=(const VectorN& other) {
        data = other.data;
        return *this;
    };
    VectorN& operator+=(const VectorN& other) {
        for (int i = 0; i < N; i++) {
            data[i] += other.data[i];
        }
        return *this;
    };
    VectorN& operator-=(const VectorN& other) {
        for (int i = 0; i < N; i++) {
            data[i] -= other.data[i];
        }
        return *this;
    };
    VectorN& operator*=(const E& other) {
        for (int i = 0; i < N; i++) {
            data[i] *= other;
        }
        return *this;
    };
    VectorN& operator/=(const E& other) {
        for (int i = 0; i < N; i++) {
            data[i] /= other;
        }
        return *this;
    };
    VectorN operator+(const VectorN& other) const {
        VectorN result = *this;
        result += other;
        return result;
    };
    VectorN operator-(const VectorN& other) const {
        VectorN result = *this;
        result -= other;
        return result;
    };
    VectorN operator*(const E& other) const {
        VectorN result = *this;
        result *= other;
        return result;
    };
    VectorN operator/(const E& other) const {
        VectorN result = *this;
        result /= other;
        return result;
    };
    E& operator[](int i) const {
        return data[i];
    };
    E dot(const VectorN& other) const {
        E result = 0;
        for (int i = 0; i < N; i++) {
            result += data[i] * other[i];
        }
        return result;
    };
    E norm() const {
        return sqrt(dot(*this));
    };
};

#endif
