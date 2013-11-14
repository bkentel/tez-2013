#include "pch.hpp"
#include "languages.hpp"

using namespace tez;

utf8string const language_info::FILE_NAME = R"(./data/language.def)";

////////////////////////////////////////////////////////////////////////////////
namespace {

std::once_flag once_flag;

utf8string lang_fallback_string   = {"en"};
utf8string lang_default_string    = {"en"};
utf8string lang_substitute_string = {"<substitute>"};

language_id lang_fallback_id = 1;
language_id lang_default_id  = 1;

boost::container::flat_map<size_t, language_info::info> lang_info;

void init() {
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
        
    if (json_root[FIELD_FILE_ID].asString() != FIELD_LANGUAGE) {
        //wrong file_id
        BK_DEBUG_BREAK();
    }

    auto const& languages = json_root[FIELD_LANGUAGE];
    if (!languages.isArray()) {
        BK_DEBUG_BREAK();
    }

    lang_info.reserve(languages.size());
    string_hasher hasher;
    language_id   id = 1;

    for (auto const& lang : languages) {
        if (lang.size() != 2) {
            BK_DEBUG_BREAK();
        } else if (!lang[0].isString()) {
            BK_DEBUG_BREAK();
        } else if (!lang[1].isString()) {
            BK_DEBUG_BREAK();
        }
       
        auto string_id = lang[0].asString();
        auto name      = lang[1].asString();
        auto hash      = hasher(string_id);

        auto const result = lang_info.emplace(
            hash, std::make_tuple(id, std::move(string_id), std::move(name))
        );
        
        if (!result.second) {
            BK_DEBUG_BREAK();
        }

        id++;
    }

    auto const find_lang = [&](utf8string const& field) {
        auto const& languages = lang_info;

        auto const& value = json_root[field];
        if (!value.isString()) {
            return languages.end();
        }

        auto       string = value.asString();
        auto const hash   = hasher(string);
        
        return languages.find(hash);
    };

    auto const end = lang_info.end();

    auto it = find_lang(FIELD_DEFAULT);
    if (it != end) {
        lang_default_id     = std::get<0>(it->second);
        lang_default_string = std::get<1>(it->second);
    }

    it = find_lang(FIELD_FALLBACK);
    if (it != end) {
        lang_fallback_id     = std::get<0>(it->second);
        lang_fallback_string = std::get<1>(it->second);
    }

    auto const& substitute = json_root[FIELD_SUBSTITUTE];
    if (substitute.isString()) {
        lang_substitute_string = std::move(substitute.asString());
    }
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
  : language_map(json.isArray() ? json.size() : 0)
{
    if (!json.isArray()) {
        BK_DEBUG_BREAK();
    }

    for (auto const& lang : json) {
        if (!lang.isArray()) {
            BK_DEBUG_BREAK();
        } else if (lang.size() != 2) {
            BK_DEBUG_BREAK();
        } else if (!lang[0].isString()) {
            BK_DEBUG_BREAK();
        } else if (!lang[1].isString()) {
            BK_DEBUG_BREAK();
        }

        auto id_string = lang[0].asString();
        auto value     = lang[1].asString();

        auto const& info = language_info::get_info(id_string);
        auto const  id   = std::get<0>(info);

        if (id == 0) {
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
