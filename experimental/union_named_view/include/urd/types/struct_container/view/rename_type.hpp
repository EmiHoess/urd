#pragma once
#include <type_traits>
#include <urd/types/struct_container/view/array_index.hpp>
#include <urd/types/struct_container/view/alias.hpp>
#include <urd/types/struct_container/view/allocation.hpp>
#include <urd/types/struct_container/view/element_ref.hpp>
#include <urd/types/struct_container/view/field_ref.hpp>
#include <urd/meta/traits/member_pointer.hpp>
#include <urd/types/struct_container/soa_offset.hpp>
#include <urd/meta/traits/collection.hpp>

namespace urd::v2 {

// --- helpers ----------------------------------------------------------------

template<typename T>
struct is_soa2 : urd::has_soa_layout<T> {};

template<typename T>
concept HasStaticIndex = requires {
    { T::index } -> std::convertible_to<std::size_t>;
};

// Partial-specialisation helpers — avoids instantiating pointer_to_member
// traits on nullptr_t (MSVC does not suppress if constexpr inside lambdas).

template<bool IsMember, auto mptr, typename Original>
struct aliased_type_of { using type = Original; };

template<auto mptr, typename Original>
struct aliased_type_of<true, mptr, Original> {
    using type = urd::member_field_t<decltype(mptr)>;
};

template<bool IsMember, size_t N, auto mptr>
struct member_offset_of { static constexpr size_t value = 0; };

template<size_t N, auto mptr>
struct member_offset_of<true, N, mptr> {
    static constexpr size_t value = urd::v2::soa_offset_of<N>(mptr);
};

template<bool IsMember, size_t base, size_t N, auto mptr, size_t moff>
struct current_offset_of { static constexpr size_t value = base; };

template<size_t base, size_t N, auto mptr, size_t moff>
struct current_offset_of<true, base, N, mptr, moff> {
    static constexpr bool soa = is_soa2<urd::member_class_t<decltype(mptr)>>::value;
    static constexpr size_t value = base + (soa ? moff : 0);
};

// --- rename_type ------------------------------------------------------------

template<size_t _offset, size_t N, auto _member_pointer, typename _original_type>
struct rename_type {
    static constexpr auto member_pointer  = _member_pointer;
    static constexpr bool is_member_alias = (member_pointer != nullptr);

    using original_type = _original_type;
    using aliased_type  = typename aliased_type_of<is_member_alias, _member_pointer, _original_type>::type;

    static constexpr size_t member_offset   = member_offset_of<is_member_alias, N, _member_pointer>::value;
    static constexpr size_t previous_offset = _offset;
    static constexpr size_t current_offset  = current_offset_of<is_member_alias, _offset, N, _member_pointer, member_offset>::value;
    static constexpr size_t previous_N      = N;
    static constexpr size_t current_coeff   = current_offset / N;

    using index_type_t = urd::v2::array_index<aliased_type, N>;
    using prev_alias   = nullptr_t;

    template<auto ptr43>
    using next_alias_tmp = urd::v2::rename_type<current_offset, N, ptr43, original_type>;

    template<alias_header typename alias4, auto ptr2, typename alias_args>
    static constexpr auto instance() {
        if constexpr (std::is_same_v<decltype(ptr2), aliased_type>) {
            return alias4<urd::v2::rename_type<current_offset, N, nullptr, original_type>, alias_args>{};
        } else if constexpr (std::is_union_v<urd::member_class_t<decltype(ptr2)>>) {
            return urd::member_field_t<decltype(ptr2)>{};
        } else if constexpr (HasStaticIndex<alias_args>) {
            using type_ptr = urd::member_field_t<decltype(ptr2)>;
            using arty     = urd::collection_item_type<type_ptr>;
            static constexpr size_t M = std::tuple_size_v<std::remove_reference_t<type_ptr>>;
            using d4_alias = urd::v2::rename_type<alias_args::index * sizeof(arty), N * M, nullptr, arty>;
            return urd::v2::element_ref<d4_alias, 0, N * M, alias_args::index * sizeof(arty)>{};
        } else {
            using type_1 = urd::v2::field_ref<
                current_coeff + (is_soa2<urd::member_class_t<decltype(ptr2)>>::value
                                 ? urd::v2::soa_offset_of<1>(ptr2) : 0),
                N,
                ptr2
            >;
            using type_2 = alias4<next_alias_tmp<ptr2>, alias_args>;
            using type   = std::conditional_t<::v2::is_foo_template<alias4>::value, type_1, type_2>;
            return type{};
        }
    }

    template<alias_header typename alias5, auto ptr2, typename alias_args = void>
    using dub = decltype(instance<alias5, ptr2, alias_args>());
};

} // namespace urd::v2
