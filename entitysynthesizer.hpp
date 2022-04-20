#ifndef __ENTITY_SYNTHESIZER_HPP__
#define __ENTITY_SYNTHESIZER_HPP__

#include <tuple>
#include <vector>

// 从多个vector<tuple<ID, component>>中取出第一个ID等于给定ID的component合并成tuple
template<typename _Index, typename... _Rest>
class IDMatcher{};

template<typename _Index, typename _First, typename... _Rest>
class IDMatcher<_Index, _First, _Rest...>: private IDMatcher<_Index, _Rest...>{
public:
    using ID = _Index;
    using F = _First;
    using Fvec = std::vector<std::tuple<ID, F>>;
    using Self = IDMatcher<ID, F, _Rest...>;
    using Succ = IDMatcher<ID, _Rest...>;

    const Fvec& fvec;
    IDMatcher(const Fvec& fvec, const std::vector<std::tuple<ID, _Rest>>&... succ):fvec(fvec), Succ(succ...) {};
    std::tuple<const F&, const _Rest&...> find(ID id){
        auto tar = std::find_if(fvec.begin(), fvec.end(), [&](const std::tuple<ID, F>& t){
            return std::get<0>(t) == id;
        });
        auto ret = std::tuple_cat(std::tuple<const F&>(std::get<1>(*tar)), Succ::find(id));
        return ret;
    };
};

template<typename _Index>
class IDMatcher<_Index>{
public:
    using ID = _Index;
    using Self = IDMatcher<ID>;
    IDMatcher(){};
    std::tuple<> find(ID id){
        return std::make_tuple();
    };
};


// 遍历主键元素，与剩余vector中ID相同的组件配为tuple返回
template <typename _Index, typename _MainKey, typename... _Rest>
class EntitySynthesizer{
public:
    using ID = _Index;
    using M = _MainKey;
    using Mvec = std::vector<std::tuple<ID, M>>;
    using Self = EntitySynthesizer<ID, _MainKey, _Rest...>;
    using R = std::tuple<M&, const _Rest&...>;
private:
    IDMatcher<_Index, _Rest...> matcher;
    Mvec& mvec;
public:
    struct iter{
        ID id;
        Self * self;
        bool operator!=(const iter& other){
            return self != other.self || id != other.id;
        };
        iter& operator++(){
            id++;
            return *this;
        };
        iter operator++(int){
            iter ret = *this;
            id++;
            return ret;
        };
        R operator*(){
            return self->operator[](id);
        };
    };
    EntitySynthesizer(Mvec& mainKey, const std::vector<std::tuple<_Index, _Rest>>&... rest):matcher(rest...), mvec(mainKey){};

    iter begin(){
        return iter{0, this};
    };
    iter end(){
        return iter{size(), this};
    };
    size_t size(){
        return mvec.size();
    }
    R operator[](ID id){
        return std::tuple_cat(std::tuple<M&>(std::get<1>(mvec[id])), matcher.find(std::get<0>(mvec[id])));
    };
};

template<typename _Index, typename _Key, typename... _Element>
EntitySynthesizer<_Index, _Key, _Element...> make_entity(std::vector<std::tuple<_Index, _Key>>& mainKey, const std::vector<std::tuple<_Index, _Element>>&... rest){
    return EntitySynthesizer<_Index, _Key, _Element...>(mainKey, rest...);
};

#endif
