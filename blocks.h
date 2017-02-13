// paged-map, a.ksenzhuk, 2017

#pragma once

#include <cstdint>
#include <stdexcept>
#include "node.h"


namespace paged_map {


constexpr size_t next_power_of_2_impl(size_t i) { return i == 0 ? 1 : next_power_of_2_impl(i >> 1) << 1; }
constexpr size_t next_power_of_2(size_t i) { return i == 0 ? 0 : next_power_of_2_impl(i - 1); }


template<typename value_t>
class block_item {
public:
    operator const value_t&() const { return reinterpret_cast<const value_t &>(_data); }
    operator value_t&() { return reinterpret_cast<value_t &>(_data); }

    template<typename ...param_ts>
    void construct(param_ts &&...params) {
        new (reinterpret_cast<void *>(&_data)) value_t(std::forward<param_ts>(params)...)
    }

    void destruct() {
        reinterpret_cast<value_t *>(&_data)->~value_t();
        std::fill_n(_data, sizeof(_data) / sizeof(_data[0]), 0xDA);
    }

private:
    typedef data[next_power_of_2(sizeof(value_t))] data_buffer_t;
    alignas(value_t) data_buffer_t _data;
};


template<typename item_t>
class blocks {
public:
    blocks(size_t init_count = 256)
        : _last_allocated(0)
        , _current_size(init_count)
        , _storage(init_count) { }

    const item_t &operator[](size_t idx) const {
        const auto &block = _storage[idx];
        if ( block.state == block_state::Free )
            throw std::out_of_range("this element is not allocated");
        return block.item;
    }

    item_t &operator[](size_t idx) {
        auto &block = _storage[idx];
        if ( block.state == block::block_state::Free )
            throw std::out_of_range("this element is not allocated");
        return block.item;
    }

    template<typename ...param_ts>
    size_t emplace(param_ts &&...params) {
        const auto old_size = _current_size;
        auto &block = _storage[next_free_idx()];

        try {
            block.item.construct(std::forward<param_ts>(params)...);
            block.state = block::block_state::Used;
        }
        catch (...) {
            block.item.destruct();
            resize(old_size);
            throw;
        }
    }

    void erase(size_t idx) {
        auto &block = _storage[idx];
        if ( block.state != block::block_state::Used )
            throw std::out_of_range("this element is not allocated");

        block.item.destruct();
        block.state = block::block_state::Free;
    }

private:
    size_t resize(size_t new_size) {
        const auto old_size = _current_size;
        _storage.resize(new_size);
        _current_size = new_size;
        return old_size;
    }

    size_t next_free_idx() {
        for ( size_t current = _last_allocated; current < _storage.size(); ++current )
            if ( _storage[current].state == block::block_state::Free )
                return _last_allocated = current;

        for ( size_t current = 0; current < _last_allocated; ++current )
            if ( _storage[current].state == block::block_state::Free )
                return _last_allocated = current;

        const auto old_size = resize(next_power_of_2(_storage.size() + 1));
        return _storage[_last_allocated = old_size];
    }

private:
    struct block {
        block_item<item_t> item;
        enum block_state : uint8_t { Used, Free } state;
    };
    size_t _last_allocated;
    size_t _current_size;
    std::vector<block> _storage;
};


} // namespace paged_map