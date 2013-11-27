#include "pch.hpp"
#include "json.hpp"
#include "util.hpp"

namespace json   = bklib::json;
namespace error  = bklib::json::error;
namespace detail = bklib::json::detail;

using bklib::utf8string;

using json::cref;
namespace error = ::bklib::json::error;

namespace {
    utf8string const type_strings[] = {
        {"null"},   {"int"}
      , {"uint"},   {"float"}
      , {"string"}, {"bool"}
      , {"array"},  {"object"}
    };

    utf8string const& get_type_string(Json::ValueType const type) {
        return type_strings[static_cast<size_t>(type)];
    }

    error::bad_type make_bad_type(
        Json::ValueType const expected
      , Json::ValueType const actual
    ) {
        error::bad_type e;

        e << error::info_expected_type{get_type_string(expected)}
          << error::info_actual_type{get_type_string(actual)};

        return e;
    }
} //namespace anon

cref json::require_array(cref json) {
    if (!json.isArray()) {
        BOOST_THROW_EXCEPTION(
            make_bad_type(Json::arrayValue, json.type())
        );
    }

    return json;
}

cref json::require_object(cref json) {
    if (!json.isObject()) {
        BOOST_THROW_EXCEPTION(
            make_bad_type(Json::objectValue, json.type())
        );
    }

    return json;
}

cref json::require_key(cref json, size_t index) {
    BK_ASSERT(json.isArray());

    if (json.size() < index) {
        BOOST_THROW_EXCEPTION(
            error::bad_index{} << error::info_index{index}
        );
    }

    return json[index];
}

cref json::require_key(cref json, utf8string const& index) {
    BK_ASSERT(json.isObject());

    if (!json.isMember(index)) {
        BOOST_THROW_EXCEPTION(
            error::bad_index{} << error::info_index{index}
        );
    }

    return json[index];
}

utf8string json::require_string(cref json) {
    if (!json.isString()) {
        BOOST_THROW_EXCEPTION(
            make_bad_type(Json::stringValue, json.type())
        );
    }

    return json.asString();
}



std::ostream& error::operator<<(std::ostream& out, base const& e) {
    out << "json exception (" << typeid(e).name() << ")";

    if (auto const ptr = boost::get_error_info<info_expected_type>(e)) {
        out << "\n  expected type = " << *ptr;
    }
    if (auto const ptr = boost::get_error_info<info_actual_type>(e)) {
        out << "\n  actual type   = " << *ptr;
    }
    if (auto const ptr = boost::get_error_info<info_expected_size>(e)) {
        out << "\n  expected size = " << *ptr;
    }
    if (auto const ptr = boost::get_error_info<info_actual_size>(e)) {
        out << "\n  actual size   = " << *ptr;
    }
    if (auto const ptr = boost::get_error_info<info_index>(e)) {
        out << "\n  index         = " << *ptr;
    }
    if (auto const ptr = boost::get_error_info<info_location>(e)) {
        out << "\n  location      = " << *ptr;
    }

    out << std::endl;
    return out;
}

void json::detail::for_each_element_skip_on_fail_on_fail_(
    error::base const& e
  , size_t      const  index
) {
    BOOST_LOG_TRIVIAL(warning)
     << "parsing failed:"
     << "  element = " << index
     << "  reason  = " << e
    ;
}

/////////////////////////

//
//using bklib::json::input_stack;
//
//namespace {
//
//struct visitor : public boost::static_visitor<input_stack::pointer_t> {
//    using pointer_t = input_stack::pointer_t;
//
//    visitor(pointer_t ptr) : ptr{ptr} {}
//
//    void check(size_t i) const {
//        if (!ptr->isArray()) {
//            BOOST_THROW_EXCEPTION(
//                error::make_type_info(Json::arrayValue, ptr->type())
//            );
//        } else if (i >= ptr->size()) {
//            BOOST_THROW_EXCEPTION(error::bad_index{});
//        }
//    }
//
//    void check(utf8string const& i) const {
//        if (!ptr->isObject()) {
//            BOOST_THROW_EXCEPTION(
//                error::make_type_info(Json::objectValue, ptr->type())
//            );
//        } else if (!ptr->isMember(i)) {
//            BOOST_THROW_EXCEPTION(error::bad_index{});
//        }
//    }
//
//    template <typename T>
//    pointer_t step_into(T const& i) const {
//        try {
//            check(i);
//        } catch (error::base& e) {
//            e << error::info_index{i};
//            throw;
//        }
//
//        return &(*ptr)[i];
//    }
//
//    pointer_t operator()(utf8string const& i) const { return step_into(i); }
//    pointer_t operator()(size_t i) const { return step_into(i); }
//
//    pointer_t ptr;
//};
//
//} //namespace
//
//input_stack::input_stack(std::istream& in)
//  : current_{nullptr}
//{
//    if (!in) {
//        BK_DEBUG_BREAK(); //TODO
//    }
//
//    Json::Reader json_reader;
//    if (!json_reader.parse(in, root_)) {
//        BK_DEBUG_BREAK(); //TODO
//    }
//
//    current_ = &root_;
//
//    stack_.reserve(10);
//    stack_.emplace_back(record_t{"root", current_});
//}
//
//input_stack& input_stack::step_into(json::index_t const& index) {
//    auto const ptr = get_index_(index);
//
//    stack_.emplace_back(record_t{index, ptr});
//    current_ = ptr;
//
//    BOOST_LOG_TRIVIAL(trace) << "json: location = " << *this;
//
//    return *this;
//}
//
//input_stack& input_stack::step_out() {
//    if (stack_.size() == 1) {
//        BOOST_LOG_TRIVIAL(error) << "json: at root";
//        BOOST_THROW_EXCEPTION(error::bad_type()); //TODO
//    }
//
//    stack_.pop_back();
//    current_ = stack_.back().second;
//
//    BOOST_LOG_TRIVIAL(trace) << "json: location = " << *this;
//
//    return *this;
//}
//
//utf8string input_stack::require_string(json::index_t const& index) const {
//    auto const ptr = get_index_(index);
//    if (!ptr->isString()) {
//        BOOST_THROW_EXCEPTION(
//            error::make_type_info(Json::stringValue, ptr->type())
//        );        
//    }
//
//    return ptr->asString();
//}
//
//input_stack::pointer_t input_stack::get_index_(json::index_t const& index) const {
//    try {
//        return boost::apply_visitor(visitor{current_}, index);
//    } catch (error::base& e) {
//        std::stringstream location;
//        location << *this;
//        e << error::info_location(location.str());     
//        throw;
//    }
//}
//
//std::ostream& json::operator<<(
//    std::ostream& lhs
//  , input_stack const& rhs
//) {
//    struct output_visitor : public boost::static_visitor<void> {
//        BK_DELETE_ALL(output_visitor);
//
//        output_visitor(std::ostream& out) : out{out} {}
//
//        void operator()(utf8string const& i) const { out << "{" << i << "}"; }
//        void operator()(size_t i) const { out << "[" << i << "]"; }
//
//        std::ostream& out;
//    };
//
//    for (auto const& record : rhs.stack_) {
//        boost::apply_visitor(output_visitor{lhs}, record.first);
//    }
//
//    return lhs;
//}
