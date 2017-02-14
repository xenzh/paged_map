// paged-map, a.ksenzhuk, 2017

#pragma once

#include <cstddef>
#include <vector>


namespace paged_map {


template<typename address_t, typename value_t>
class page {
public:
    typedef std::vector<value_t> data_t;

    page() : _granularity(0), _start_idx(0), _list(nullptr) { } // temp to allow tree to own empty page
    page(address_t start, data_t *list, size_t granularity) : _granularity(granularity), _start_idx(idx(start)), _list(list) { }

    const value_t &operator[](address_t address) const { return (*_list)[idx(address)]; }
    value_t &operator[](address_t address) { return (*_list)[idx(address)]; }

    size_t size() const { return _list->size() * _granularity; }
    size_t elem_size() const { return _granularity; }
    address_t lower() const { return _start_idx * _granularity; }

private:
    address_t idx(address_t address) const { return address / _granularity; }

private:
    size_t _granularity;
    address_t _start_idx;
    data_t *_list;
};


} // namespace paged_map