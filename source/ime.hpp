#pragma once

#include <vector>
#include <string>

#include "callback.hpp"

namespace bklib {

template <typename T>
using non_owning_ptr = T*;

class ime_manager;
class ime_text_store;
class ime_composition;
class ime_candidate_list;

//------------------------------------------------------------------------------
class ime_manager {
public:
    ime_text_store create_text_store();
private:
    class impl_t_;
    std::unique_ptr<impl_t_> impl_;
};
//------------------------------------------------------------------------------
class ime_composition {
public:
    enum class line_style {
        none, solid, dot, dash, squiggle
    };

    enum class attribute {
        input, converted, input_error, fixed_converted, other,
        target_converted, target_non_converted,
    };

    struct info {
        line_style style;
        attribute  attr;

        platform_string::const_iterator begin;
        platform_string::const_iterator end;
    };

    using ranges_t = std::vector<info>;

    ranges_t::const_iterator cbegin() const {
        return std::cbegin(ranges_);
    }

    ranges_t::const_iterator cend() const {
        return std::cend(ranges_);
    }
private:
    class impl_t_;
    std::unique_ptr<impl_t_> impl_;

    ranges_t ranges_;
};
//------------------------------------------------------------------------------
class ime_text_store {
public:
private:
    class impl_t_;
    std::unique_ptr<impl_t_> impl_;
};
//------------------------------------------------------------------------------
class ime_candidate_list {
public:
    class impl_t_;
    friend impl_t_;

    using strings_t = std::vector<platform_string>;

    enum class update_type {
        strings, selection,
    };

    ~ime_candidate_list();

    ime_candidate_list(ime_candidate_list const&) = default;
    ime_candidate_list& operator=(ime_candidate_list const&) = default;

    //strings_t::const_iterator begin() const { return cbegin(); }
    //strings_t::const_iterator end() const { return cend(); }

    //strings_t::const_iterator cbegin() const;
    //strings_t::const_iterator cend() const;

    //strings_t::const_iterator page_begin() const;
    //strings_t::const_iterator page_end() const;

    //size_t size() const;
    //size_t selection() const;

    //size_t page_selection() const;

    //void lock();
    void unlock();

    BK_DECLARE_EVENT(on_begin,  void ());
    BK_DECLARE_EVENT(on_update, void (ime_candidate_list, update_type));
    BK_DECLARE_EVENT(on_end,    void ());
private:
    explicit ime_candidate_list(non_owning_ptr<impl_t_> impl);
    non_owning_ptr<impl_t_> impl_;
};

} //namespace bklib
