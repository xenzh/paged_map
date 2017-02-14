// paged-map, a.ksenzhuk, 2017

#pragma once

#include <cstdint>
#include <stdexcept>

#include "node.h"
#include "block_item_utils.h"


namespace paged_map {


template<typename item_t, template<typename ...param_ts> class item_wrapper_t = block_item>
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
        const auto this_idx = next_free_idx();
        auto &block = _storage[this_idx];

        try {
            block.item.construct(std::forward<param_ts>(params)...);
            block.state = block::block_state::Used;
            return this_idx;
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
        item_wrapper_t<item_t> item;
        enum block_state : uint8_t { Used, Free } state;
    };
    size_t _last_allocated;
    size_t _current_size;
    std::vector<block> _storage;
};


} // namespace paged_map