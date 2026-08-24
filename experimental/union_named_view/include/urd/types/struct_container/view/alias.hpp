#pragma once

#include <type_traits>

namespace v2 {
    template<typename index_type>
    struct alias_index_arm {
        static_assert(std::is_standard_layout_v<index_type>);
        index_type index;
    };

    #define alias_header \
		template< \
			typename alias, \
			typename TARGS = void\
        >

#define DUB_INIT \
using prev_alias = std::conditional_t<std::is_union_v<alias>, \
					typename alias::prev_alias, \
					alias>; \
using AT = std::conditional_t<std::is_union_v<alias>, \
					alias, \
					typename alias::aliased_type>; \
using aliased_type = typename prev_alias::aliased_type; \
using value_type = typename prev_alias::aliased_type::value_type; \
template< \
		auto member_pointer = nullptr, \
		alias_header typename previous_alias = no_template, \
		typename alias_args = void \
	> \
using dub = prev_alias::template dub<previous_alias, member_pointer, alias_args>; \
::v2::alias_index_arm<typename prev_alias::index_type_t> index;


alias_header union no_template {
};

#define contrafactum alias_header union

template<alias_header typename T>
struct is_foo_template : std::false_type {};

template<>
struct is_foo_template<no_template> : std::true_type {};

} // namespace v2

// Export to global scope so callers don't need the v2:: prefix
using ::v2::no_template;
