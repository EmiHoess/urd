#pragma once

#include <cstddef>
#include <type_traits>

#include <urd/member_pointer.hpp>
#include <structured_layout/detail/element_access.hpp>

namespace structured_layout {

template<typename storage_type>
class basic_element_reference : public detail::storage_element_reference<storage_type> {
public:
    using detail::storage_element_reference<storage_type>::storage_element_reference;
};

template<typename storage_type, auto member>
class basic_field_view {
public:
    using record_type = typename std::remove_const_t<storage_type>::value_type;
    using value_type = urd::member_value_t<member>;

    static_assert(std::is_same_v<record_type, urd::member_owner_t<member>>);

    constexpr basic_field_view(
        storage_type& storage,
        std::size_t extent) noexcept
        : storage_(&storage), extent_(extent) {}

    constexpr std::size_t size() const noexcept {
        return extent_;
    }

    constexpr decltype(auto) operator[](std::size_t index) const noexcept {
        return storage_->template get<member>(index);
    }

    constexpr value_type load(std::size_t index) const {
        return storage_->load(index).*member;
    }

    constexpr void store(std::size_t index, value_type const& value) const
        requires (!std::is_const_v<storage_type>)
    {
        auto record = storage_->load(index);
        record.*member = value;
        storage_->store(index, record);
    }

private:
    storage_type* storage_;
    std::size_t extent_;
};

}
