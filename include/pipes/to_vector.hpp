#ifndef TO_VECTOR_HPP
#define TO_VECTOR_HPP

#include "pipes/accumulate.hpp"

#include <vector>

namespace pipes
{
    template<typename T>
    auto to_vector()
    {
        return accumulate(std::vector<T>{}, [](std::vector<T> acc, auto&& ele) {
            acc.emplace_back(FWD(ele));
            return acc;
        });
    }
}

#endif /* TO_VECTOR_HPP */
