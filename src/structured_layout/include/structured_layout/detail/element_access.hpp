#pragma once

#include <cstddef>
#include <type_traits>

namespace structured_layout::detail {

template<typename storage_type>
class storage_element_reference {
public:
    using value_type = typename std::remove_const_t<storage_type>::value_type;

    constexpr storage_element_reference(
        storage_type& storage, std::size_t index) noexcept
        : storage_(&storage), index_(index) {}

    template<auto... members>
    constexpr decltype(auto) get() const noexcept {
        static_assert(sizeof...(members) > 0);
        return storage_->template get<members...>(index_);
    }

    constexpr decltype(auto) value() const noexcept
        requires requires (storage_type& storage) { storage.reference(0); }
    {
        return storage_->reference(index_);
    }

    constexpr value_type load() const {
        return storage_->load(index_);
    }

    constexpr void store(value_type const& value) const
        requires (!std::is_const_v<storage_type>)
    {
        storage_->store(index_, value);
    }

private:
    storage_type* storage_;
    std::size_t index_;
};

}
