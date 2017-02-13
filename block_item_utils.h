// paged-map, a.ksenzhuk, 2017

#pragma once

#include <cstdint>
#include <type_traits>


namespace paged_map
{


constexpr size_t next_power_of_2_impl( size_t i ) { return i == 0 ? 1 : next_power_of_2_impl( i >> 1 ) << 1; }
constexpr size_t next_power_of_2( size_t i ) { return i == 0 ? 0 : next_power_of_2_impl( i - 1 ); }


template<typename value_t>
class block_item
{
public:
    operator const value_t&( ) const { return reinterpret_cast<const value_t &>( _data ); }
    operator value_t&( ) { return reinterpret_cast<value_t &>( _data ); }

    template<typename ...param_ts>
    void construct( param_ts &&...params )
    {
        new ( reinterpret_cast<void *>( &_data ) ) value_t( std::forward<param_ts>( params )... );
    }

    void destruct()
    {
        reinterpret_cast<value_t *>( &_data )->~value_t();
        std::fill_n( _data, sizeof( _data ) / sizeof( _data[0] ), 0xDA );
    }

private:
    typedef uint8_t data_buffer_t[next_power_of_2( sizeof( value_t ) )];
    alignas(value_t) data_buffer_t _data;
};



template<
    typename value_t,
    typename = typename std::enable_if<std::is_same<value_t, std::vector<typename value_t::value_type>>::value>::type>
class block_vector_item
{
public:
    block_vector_item() : _constructed( false ) { }

    operator const value_t&() const { return reinterpret_cast<const value_t &>(_data); }
    operator value_t&() { return reinterpret_cast<value_t &>(_data); }

    template<typename ...param_ts>
    void construct(param_ts &&...params)
    {
        if ( !_constructed ) {
            new ( reinterpret_cast<void *>( &_data ) ) value_t( std::forward<param_ts>( params )... );
            _constructed = true;
        }
        else {
            const auto new_size = get_size( std::forward<param_ts>( params )... );
            const auto &vect = static_cast<const value_t &>( *this );

            if ( vect.size() != new_size )
                vect.resize( new_size );
        }
    }

    void destruct() {
        if (_constructed) {
            // call d-tors for all vector elements
        }
    }

private:
    template<typename first_t, typename ...rest_ts>
    static size_t get_size( first_t &&first, rest_ts &&... ) {
        return first;
    }

private:
    typedef uint8_t data_buffer_t[next_power_of_2( sizeof( value_t ) )];
    alignas(value_t) data_buffer_t _data;
    bool _constructed;
};



} // namespace paged_map
