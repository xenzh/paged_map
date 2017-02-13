// paged-map, a.ksenzhuk, 2017

#pragma once

#include <memory>
#include <stdexcept>
#include "node.h"
#include "blocks.h"


namespace paged_map {


// self-scaling tree of pages
// * manages pages (owns, re-uses already allocated ones)
// * scales page structure (zoom_in, zoom_out)
// * accesses given address (access: operator[], insertion: insert())
template<typename address_t, typename value_t, typename page_t, size_t max_page_size>
class tree {
public:
    typedef size_t page_link_t; // make it smaller?..
    typedef node<address_t, value_t, page_link_t> node_t;
    typedef typename page_t::data_t page_data_t;

    // should allocate new table (or tables, recursively)
    page_t *zoom_in(node_t *target, size_t this_node_size, size_t sample_size) {
        if ( target == nullptr || target->type() != node_t::type_t::None )
            throw std::logic_error("Page is already occupied");

        const auto new_page_size = this_node_size / sample_size;
        if ( new_page_size > max_page_size )
            ; // recursion
        else {
            // allocate new page, return node pointer to the new page
            const auto new_page_idx = pages.emplace(new_page_size);
            *target = node_t();
        }
    }

    // should allocate new root table and adopt the old one
    void zoom_out();

    const value_t &operator[](address_t address) const { return _root[address]; }
    value_t &operator[](address_t address) { return _root[address]; }

    //value_t &emplace(address_t address, value_t value)

private:
    page_t _root;
    blocks<page_t> pages;
    //blocks<page_data_t> pages; // page_data_t storage that is able to reuse
};


} // namespace paged_map