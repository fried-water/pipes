#ifndef ACCUMULATE_HPP
#define ACCUMULATE_HPP

#include "pipes/base.hpp"
#include "pipes/helpers/assignable.hpp"
#include "pipes/helpers/invoke.hpp"
#include "pipes/helpers/FWD.hpp"

namespace pipes
{
    template<typename Ele, typename Function>
    class accumulate_pipeline : public pipeline_base<accumulate_pipeline<Ele, Function>>
    {
    public:
        template<typename... T>
        void onReceive(T&&... values)
        {
            acc_ = detail::invoke(function_.get(), std::move(acc_), FWD(values)...); 
        }

        template<typename T>
        explicit accumulate_pipeline(T&& acc, Function function) : acc_(FWD(acc)), function_(function) {}

        template<typename Ele_ = Ele, typename = std::enable_if_t<std::is_copy_constructible<Ele_>::value>>
        Ele sink() & { return acc_; }
        Ele sink() && { return std::move(acc_); }
        
    private:
        Ele acc_;
        detail::assignable<Function> function_;
    };
    
    template<typename T, typename Function>
    auto accumulate(T&& t, Function function)
    {
        return accumulate_pipeline<std::decay_t<T>, Function>(FWD(t), function);
    }
}

#endif /* ACCUMULATE_HPP */
