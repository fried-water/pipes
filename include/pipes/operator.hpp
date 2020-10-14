#ifndef OPERATOR_HPP
#define OPERATOR_HPP

#include "pipes/impl/concepts.hpp"
#include "pipes/impl/pipes_assembly.hpp"

#include <type_traits>

namespace pipes
{

// range >>= pipeline (rvalue ranges)
    
    template<typename Range, typename Pipeline, detail::IsARange<Range> = true, detail::IsAPipeline<Pipeline> = true>
    auto operator>>=(Range&& range, Pipeline&& pipeline) -> std::enable_if_t<!std::is_reference<Range>::value, decltype(FWD(pipeline).sink())>
    {
        using std::begin;
        using std::end;

        auto it = std::make_move_iterator(begin(range));
        auto end_ = std::make_move_iterator(end(range));
        for(; it != end_; ++it) {
            *pipeline++ = *it;
        }

        return FWD(pipeline).sink();
    }

// range >>= pipeline (lvalue ranges)
    
    template<typename Range, typename Pipeline, detail::IsARange<Range> = true, detail::IsAPipeline<Pipeline> = true>
    auto operator>>=(Range&& range, Pipeline&& pipeline) -> std::enable_if_t<std::is_reference<Range>::value, decltype(FWD(pipeline).sink())>
    {
        using std::begin;
        using std::end;

        auto it = begin(range);
        auto end_ = end(range);
        for(; it != end_; ++it) {
            *pipeline++ = *it;
        }

        return FWD(pipeline).sink();
    }

// pipe >>= pipe
    
    template<typename Pipe1, typename Pipe2, detail::IsAPipe<Pipe1> = true, detail::IsAPipe<Pipe2> = true>
    auto operator>>=(Pipe1&& pipe1, Pipe2&& pipe2)
    {
        return detail::CompositePipe<std::decay_t<Pipe1>, std::decay_t<Pipe2>>(FWD(pipe1), FWD(pipe2));
    }

// pipe >>= pipeline
    
    template<typename Pipe, typename Pipeline, detail::IsAPipe<Pipe> = true, detail::IsAPipeline<Pipeline> = true>
    auto operator>>=(Pipe&& pipe, Pipeline&& pipeline)
    {
        return make_generic_pipeline(FWD(pipe), FWD(pipeline));
    }
    
} // namespace pipes

#endif /* OPERATOR_HPP */
