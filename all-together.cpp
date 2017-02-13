#include <cstddef>
#include <vector>


namespace paged_map {


template<typename value_t, typename address_t>
class page {
public:
    page(size_t size, size_t granularity) : _list(size), _granularity(granularity) { }
    const value_t &operator[](address_t address) const { return _list[idx(address)]; }
    value_t &operator[](address_t address) { return _list[idx(address)]; }

private:
    address_t idx(address_t address) const { return address / _granularity; }

private:
    size_t _granularity;
    std::vector<value_t> _list;
};


template<typename value_t, typename page_link_t, typename address_t>
class node {
public:
    typedef page<value_t, address_t> page_t;

    node() : _nested(nullptr), _value_type(type::NONE) { }
    explicit node(value_t value) : _value(value), _nested(nullptr), _value_type(type::VALUE) { }
    explicit node(page_t *page, page_link_t page_link) : _link(page_link), _nested(page), _value_type(type::PAGE_LINK) { }

    const value_t operator[](size_t address) const {
        switch ( _value_type ) {
            case type::VALUE : return _value;
            case type::PAGE_LINK : return _nested[address];
            case type::NONE:
            default:
                static value_t dummy;
                return dummy;
        }
    }

private:
    union {
        value_t _value;
        page_link_t _link;
    };
    page_t *_nested;
    enum class type { VALUE, PAGE_LINK, NONE } _value_type;
};


template<typename item_t>
class blocks {
public:
    blocks(size_t init_count = 256) : _storage(init_count) { }

private:
    struct block {
        item_t item;
        enum block_state : uint8_t { Allocated, Free } state;
    };
    size_t _last_allocated;
    std::vector<block> _storage;
};


} // namespace paged_map