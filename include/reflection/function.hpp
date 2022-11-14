
#pragma once

#include "detail.hpp"
#include "traits/func_traits.hpp"
#include "type_helper.hpp"

#include <cstddef>
#include <string_view>
#include <memory>
#include <tuple>

namespace refl
{

template <class Target, std::size_t I>
using refl_func_info =
    typename Target::template detail_function_reflection<I, struct detail_function_tag>;

template <class Target, std::size_t I>
constexpr auto func_name_v = refl_func_info<Target, I>::m_name;

template <class Target, std::size_t I>
using func_type_t = typename refl_func_info<Target, I>::template type<Target>;

template <class Target, std::size_t I>
using func_trait_t = method_traits<func_type_t<Target, I>>;

template <class Target, std::size_t I>
constexpr auto func_ptr_v = refl_func_info<Target, I>::template offset_v<Target>;

template <class Target>
constexpr size_t count_function =
    detail::index<struct function_counter_tag,
                  Target::template detail_function_reflection>::value;

#define INFER_FUNC_TYPE(NAME)                                                            \
    template <class C, typename... Args>                                                 \
    using func_type =                                                                    \
        decltype(std::declval<C>().NAME(std::declval<Args>()...)) (C::*)(Args...);

#define REFLECT_FUNCTION(NAME, ...)                                                      \
    template <std::size_t, class>                                                        \
    struct detail_function_reflection;                                                   \
    static constexpr size_t detail_##NAME##_function_index =                             \
        refl::detail::index<struct detail_##NAME##_function_tag,                         \
                            detail_function_reflection>::value;                          \
    template <class T>                                                                   \
    struct detail_function_reflection<detail_##NAME##_function_index, T>                 \
    {                                                                                    \
        INFER_FUNC_TYPE(NAME);                                                           \
        static constexpr std::string_view m_name = #NAME;                                \
        template <class Target>                                                          \
        using type = func_type<Target, __VA_ARGS__>;                                     \
        template <class Target>                                                          \
        static constexpr type<Target> offset_v = &Target::NAME;                          \
    };

template <class T, std::size_t Index>
struct dummy_t
{
};
class refl_func_t
{

public:
    template <class Target, std::size_t Index>
    constexpr refl_func_t(dummy_t<Target, Index>)
    {
        using func  = func_type_t<Target, Index>;
        using trait = method_traits<func>;

        using interface_type =
            interface_t<typename trait::result_type, typename trait::args_type>;
        using function_type = actual_func_t<typename trait::result_type,
                                            typename trait::args_type,
                                            Target,
                                            Index>;

        m_ptr  = static_cast<holder_t const*>(function_type::get_instance());
        m_argc = trait::args_count;
        m_name = function_type::m_name;
    }

public:
    template <typename R, typename... Args>
    R Invoke(void* ptr, Args... args) const
    {
        using interface_type = interface_t<R, std::tuple<Args...>>;
        // auto iface           = static_cast<interface_type*>(m_instance.get());
        auto iface = static_cast<interface_type const*>(m_ptr);

        return iface->InvokeInternal(ptr, std::forward_as_tuple(args...));
    }

private:
    struct holder_t
    {
    };

    template <typename R, typename T>
    struct interface_t;

    template <typename R, typename... Args>
    struct interface_t<R, std::tuple<Args...>> : public holder_t
    {
        constexpr interface_t() = default;
        virtual ~interface_t()  = default;
        virtual R InvokeInternal(void* obj, std::tuple<Args...>) const { return {}; }
    };
    template <typename R, typename T, class Target, std::size_t Index>
    struct actual_func_t : public interface_t<R, T>
    {
        using type        = func_type_t<Target, Index>;
        using trait       = method_traits<type>;
        using result_type = typename trait::result_type;
        using args_type   = typename trait::args_type;
        using owner_type  = typename trait::owner_type;

        consteval actual_func_t() = default;

        template <class Tuple, size_t... Idx>
        result_type InvokeInternalImpl(void* obj,
                                       Tuple&& args,
                                       std::index_sequence<Idx...>) const
        {
            return std::invoke(ptr,
                               static_cast<owner_type*>(obj),
                               std::get<Idx>(std::forward<Tuple>(args))...);
        }

        virtual result_type InvokeInternal(void* obj, T args) const override
        {
            std::cout << "Done!!" << std::tuple_size_v<T>;

            // unpack and apply
            using Indices =
                std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>>;
            return InvokeInternalImpl(obj, std::forward<T>(args), Indices{});
        }
        static constexpr actual_func_t const* get_instance() { return &instance; }

    public:
        static constexpr type ptr                = func_ptr_v<Target, Index>;
        static constexpr std::string_view m_name = func_name_v<Target, Index>;
        static constinit const actual_func_t instance;
    };

private:
    holder_t const* m_ptr;
    std::size_t m_argc;
    std::string_view m_name;
};

template <typename R, typename T, class Target, std::size_t Index>
constinit const refl_func_t::actual_func_t<R, T, Target, Index>
    refl_func_t::actual_func_t<R, T, Target, Index>::instance = {};

}; // namespace refl
