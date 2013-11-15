#include "pch.hpp"
#include "languages.hpp"

using namespace tez;

utf8string const language_info::FILE_NAME = R"(./data/language.def)";

////////////////////////////////////////////////////////////////////////////////
namespace {

std::once_flag once_flag;

utf8string lang_fallback_string   = {"en"};
utf8string lang_default_string    = lang_fallback_string;
utf8string lang_substitute_string = {"<substitute>"};

language_id lang_fallback_id = 1;
language_id lang_default_id  = 1;

boost::container::flat_map<size_t, language_info::info> lang_info;

void init() {
    using namespace bklib::json;

    static utf8string const FIELD_FILE_ID    = {"file_id"};
    static utf8string const FIELD_DEFAULT    = {"default"};
    static utf8string const FIELD_SUBSTITUTE = {"substitute"};
    static utf8string const FIELD_FALLBACK   = {"fallback"};
    static utf8string const FIELD_LANGUAGE   = {"language"};

    Json::Value  json_root;
    Json::Reader json_reader;
    
    auto json_in = std::ifstream{language_info::FILE_NAME};
    if (!json_in) {
        //failed to open the file
        BK_DEBUG_BREAK();
    }

    if (!json_reader.parse(json_in, json_root)) {
        //failed to parse the file
        BK_DEBUG_BREAK();
        std::cout << json_reader.getFormattedErrorMessages();
    }

    if (required_string(json_root[FIELD_FILE_ID]) != FIELD_LANGUAGE) {
        //wrong file_id
        BK_DEBUG_BREAK();
    }

    cref languages = required_array(json_root[FIELD_LANGUAGE]);
    lang_info.reserve(languages.size());

    string_hasher hasher;
    language_id   id = 1;

    for (cref lang : languages) {
        required_array(lang, 2, 2);
       
        auto string_id = required_string(lang[0]);
        auto name      = required_string(lang[1]);
        auto hash      = hasher(string_id);

        auto const result = lang_info.emplace(
            hash, std::make_tuple(id, std::move(string_id), std::move(name))
        );
        
        if (!result.second) {
            //duplicate
            BK_DEBUG_BREAK();
        }

        if (++id == INVALID_LANG_ID) {
            //wrapped around
            BK_DEBUG_BREAK();
        }
    }

    lang_default_string    = optional_string(json_root[FIELD_DEFAULT], lang_default_string);
    lang_fallback_string   = optional_string(json_root[FIELD_FALLBACK], lang_fallback_string);
    lang_substitute_string = optional_string(json_root[FIELD_SUBSTITUTE], lang_substitute_string);

    auto const end = lang_info.end();
    
    auto it = lang_info.find(hasher(lang_default_string));
    if (it == end) {
        //bad language
        BK_DEBUG_BREAK();
    }
    lang_default_id = std::get<0>(it->second);

    it = lang_info.find(hasher(lang_fallback_string));
    if (it == end) {
        //bad language
        BK_DEBUG_BREAK();
    }
    lang_fallback_id = std::get<0>(it->second);
}

} //namespace

language_info::info const& language_info::get_info(
    hash const lang
) {
    static info const NOT_FOUND {
        0, "", ""
    };

    std::call_once(once_flag, init);

    auto it = lang_info.find(lang);
    if (it != lang_info.end()) {
        return it->second;
    }

    return NOT_FOUND;
}

language_info::info const& language_info::get_info(
    utf8string const& lang
) {
    return get_info(string_hasher{}(lang));
}

bool language_info::is_defined(hash const lang) {
    std::call_once(once_flag, init);

    return lang_info.find(lang) != lang_info.end();
}

language_id language_info::fallback() { return lang_fallback_id; }
language_id language_info::default()  { return lang_default_id; }
utf8string  language_info::substitute()  { return lang_substitute_string; }

////////////////////////////////////////////////////////////////////////////////
language_map::language_map(size_t const size)
{
    if (size) {
        values_.reserve(size);
    }
}

language_map::language_map(Json::Value const& json)
{
    using namespace bklib::json;

    required_array(json);

    for (cref lang : json) {
        required_array(lang, 2, 2);

        auto id_string = required_string(lang[0]);
        auto value     = required_string(lang[1]);

        auto const& info = language_info::get_info(id_string);
        auto const  id   = std::get<0>(info);

        if (id == INVALID_LANG_ID) {
            BK_DEBUG_BREAK();
        }

        insert(id, std::move(value));
    }
}

utf8string const& language_map::operator[](language_id const id) const {
    auto it = values_.find(id);
    if (it != values_.end()) {
        return it->second;
    }

    std::cout << "Warning: requested language string not found.";

    it = values_.find(language_info::fallback());
    if (it != values_.end()) {
        return it->second;
    }

    std::cout << "Warning: fallback language string not found.";

    static auto const substitute = language_info::substitute();
    return substitute;
}

void language_map::insert(language_id const id, utf8string value) {
    auto result = values_.emplace(id, std::move(value));

    if (!result.second) {
        BK_DEBUG_BREAK();
        std::cout << "Warning: duplicate language.";
    }
}
