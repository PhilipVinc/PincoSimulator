//
// Created by Filippo Vicentini on 12/03/2018.
//

#ifndef SIMULATOR_HETEROGENEOUSCONTAINER_HPP
#define SIMULATOR_HETEROGENEOUSCONTAINER_HPP

#include <vector>
#include <tuple>
#include <functional>
#include <iostream>
#include <typeindex>
#include <map>
#include <array>


#ifdef MPI_SUPPORT
#include "Base/Serialization/SerializationArchiveFormats.hpp"
#include <Libraries/eigen_cereal_serialization.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#endif

template <typename T>
struct allVars
{
    static std::map<T, std::type_index> varTypes;
    static std::map<T, const std::string> varNames;
    static std::map<std::string, T> varEnums;
    static std::map<T, const size_t> varFormats;
};

template <typename C>
struct saveFormat {
    const static size_t format;
};

template <typename T, class C>
struct variable
{
    const T enumVal;
    const std::string _name;
    variable (const T val, const std::string name) : enumVal(val), _name(name)
    {
        allVars<T>::varTypes.insert(std::pair<T,std::type_index>(val, typeid(C)));
        allVars<T>::varNames.insert(std::pair<T,std::string>(val, name));
        allVars<T>::varFormats.insert(std::pair<T,size_t>(val, saveFormat<C>::format));
        allVars<T>::varEnums.insert(std::pair<std::string, T>(name, val));
    };
};

template <class T1, class T2>
struct SameType
{
    static const bool value = false;
};

template<class T>
struct SameType<T, T>
{
    static const bool value = true;
};

typedef const void* rawT1;
typedef  size_t rawT2;
typedef std::tuple<rawT1, rawT2> rawTuple;

template <class C>
rawTuple getData(const C& data);


template <class C>
C setData(rawTuple data, size_t frames, const std::vector<size_t>& dimensions);


template <typename enumVar, typename... Types>
class MyClass
{
public:
    typedef std::tuple<std::map<enumVar, Types>...> vtype;
    vtype vectors;

    template<int N, typename T>
    struct VectorOfType: SameType<T,
            typename std::tuple_element<N, vtype>::type::map::mapped_type>
    { };

    template <int N, class T, class Tuple, bool Match = false> // this =false is only for clarity
    struct MatchingField
    {
        static std::map<enumVar,T>& get(Tuple& tp)
        {
            // The "non-matching" version
            return MatchingField<N+1, T, Tuple,
                    VectorOfType<N+1, T>::value>::get(tp);
        }

        static const std::map<enumVar,T>& get(const Tuple& tp)
        {
            // The "non-matching" version
            return MatchingField<N+1, T, Tuple,
                    VectorOfType<N+1, T>::value>::get(tp);
        }

    };

    template <int N, class T, class Tuple>
    struct MatchingField<N, T, Tuple, true>
    {
        static std::map<enumVar, T>& get(Tuple& tp)
        {
            return std::get<N>(tp);
        }

        static const std::map<enumVar, T>& get(const Tuple& tp)
        {
            return std::get<N>(tp);
        }
    };

    template <typename T>
    std::vector<T>& access()
    {
        return MatchingField<0, T, vtype,
                VectorOfType<0, T>::value>::get(vectors);
    }

    template <typename T>
    void set(enumVar varName, T && el)
    {
        std::map<enumVar,T>& vec = MatchingField<0, T, vtype, VectorOfType<0, T>::value>::get(vectors);
        vec[varName] = el;
    }

    template <typename T>
    void set(enumVar varName, T & el)
    {
        std::map<enumVar,T>& vec = MatchingField<0, T, vtype, VectorOfType<0, T>::value>::get(vectors);
        vec[varName] = el;
    }

    // Used to add a type of unknown type
    template <typename T>
    void setIfRightType(enumVar varName, rawTuple data, size_t frames, const std::vector<size_t>& dimensions) {
        if (allVars<enumVar>::varTypes.at(varName) == typeid(T)) {
            std::map<enumVar, T> &theMap = MatchingField<0, T, vtype, VectorOfType<0, T>::value>::get(vectors);
            theMap[varName] = setData<T>(data, frames, dimensions);
        }
    }

    void setUnknownType(enumVar varName, rawTuple data, size_t frames, const std::vector<size_t>& dimensions) {
        int trash[] = {(setIfRightType<Types>(varName,data, frames, dimensions),0)...};
    }

    template <typename T>
    T& get(enumVar varName)
    {
        return MatchingField<0, T, vtype,
                VectorOfType<0, T>::value>::get(vectors)[varName];
    }

    template <typename T>
    std::tuple<const void*, size_t> getRaw(enumVar varName)
    {
        return getData(this->get<T>(varName));
    }

    template <typename T>
    std::vector<rawTuple> getAllTypeRaw() const
    {
        std::vector<rawTuple> res;
        const std::map<enumVar, T>& vec = MatchingField<0, T, vtype, VectorOfType<0, T>::value>::get(vectors);

        for (auto el :vec) {
            res.push_back(getData(el.second));
        }
        return res;
    }

    template <typename T>
    std::vector<std::tuple<enumVar, rawTuple>> getAllIndexedTypeRaw() const
    {
        std::vector<std::tuple<enumVar, rawTuple>> res;
        const std::map<enumVar, T>& vec = MatchingField<0, T, vtype, VectorOfType<0, T>::value>::get(vectors);

        for (auto el :vec) {
            res.emplace_back(el.first, getData(el.second));
        }
        return res;
    }


    std::vector<rawTuple> GetAllRaw() const {
        std::vector<rawTuple> result;
        std::vector<std::vector<rawTuple>>  tmp1= { (this->getAllTypeRaw<Types>())... };
        for (auto el : tmp1) {
            result.insert(result.end(), std::make_move_iterator(el.begin()), std::make_move_iterator(el.end()));
        }
        return result;
    }

    rawTuple GetSingleRaw(enumVar key) const {
        std::vector<std::vector<std::tuple<enumVar, rawTuple>>>  tmp1= { (this->getAllIndexedTypeRaw<Types>())... };
        for (auto el : tmp1) {
            auto cc =  std::find_if(el.begin(), el.end(), [&key](const std::tuple<enumVar, rawTuple>& e) {return std::get<0>(e) == key;});
            if (cc != el.end()) {
                auto kk = *cc;
                return std::get<1>(kk);
                break;
            }
        }
        return std::make_tuple<rawT1, rawT2>(nullptr, 0);
    }
#ifdef MPI_SUPPORT
public:
    template<class Archive>
    void serialize(Archive & ar)
    {
        ar(vectors);
    };
#endif

};


#endif //SIMULATOR_HETEROGENEOUSCONTAINER_HPP
