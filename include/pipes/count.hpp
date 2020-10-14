#ifndef COUNT_HPP
#define COUNT_HPP

#include "pipes/accumulate.hpp"

#include <vector>

namespace pipes
{
    template<typename Number = std::size_t>
    auto count()
    {
        return accumulate(Number{}, [](Number acc, const auto&) { return acc + 1; });
    }
}

#endif /* COUNT_HPP */
