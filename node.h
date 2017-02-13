// paged-map, a.ksenzhuk, 2017

#pragma once

#include "page.h"


namespace paged_map {


template<typename address_t, typename value_t, typename page_link_t>
class node {
public:
    typedef page<address_t, value_t> page_t;
    enum class type_t : uint8_t { Value, Page, None };

    node() : _type(type_t::None) { }
    explicit node(value_t value) : _value(value), _type(type_t::Value) { }
    explicit node(page_t &&page) : _page(page)  , _type(type_t::Page)  { }

    type_t type() const { return _type; }

    const value_t &operator[](address_t address) const {
        switch (_type) {
            case type_t::Value : return _value;
            case type_t::Page  : return _nested[address];
            case type_t::None  : /* fall-through */
            default            : return _dummy;
        }
    }

    value_t &operator[](address_t address) {
        switch (_type) {
            case type_t::Value : return _value;
            case type_t::Page  : return _nested[address];
            case type_t::None  : /* fall-through */
            default            : return _dummy;
        }
    }

private:
    static value_t _dummy;
    union {
        value_t _value;
        page_t  _page;
    };
    type_t _type;
};


} // namespace paged_map