#include <json11/json11.hpp>

namespace rmp
{   

typedef json11::Json json;

template<typename T> T parse_json(const std::string& data)
{
    return T (json(data));
}

template<typename T>
std::pair<bool, T> get_json_primitive(const json & json_data,
        const std::string & keyword, const bool throwExceptionWhenMissing)
{
    json11::Json obj = json_data[keyword];

    std::pair<bool, T> ret(false, T());

    if (!obj.is_null())
    {
        ret = std::make_pair(true, std::move(parse_json<T>(obj)));
    }
    else
    {
        if (throwExceptionWhenMissing)
        {
            throw std::runtime_error(
                    "Parse error.  Missing \'" + keyword + "\'");
        }
    }
    return std::move(ret);
}


class JSON
{
public:
    JSON() {}
    virtual ~JSON() {}

    virtual std::string serialize(void ) const
    {
        return ((json)get_data()).dump();
    }

    virtual json to_json() const
    {
        const auto & d = get_data();
        json j = d;
        return j;
    }

    virtual bool empty(void) const = 0;
    virtual void clear(void) = 0;

protected:
    template<typename T>
    T getValue(const std::string & kw) const
        {
            T v;
            std::tie(std::ignore, v) =
                get_json_primitive<T>(get_data(), kw, true);
            return v;
        }

    template <typename T>
    bool hasValue(const std::string & kw) const
        {
            bool ret;
            std::tie(ret, std::ignore) =
                get_json_primitive<T>(get_data(), kw, false);
            return ret;
        }

    virtual const json::object & get_data() const = 0;
    virtual void set_data(const json & json) = 0;
    virtual void remove_member(const std::string & kw) = 0;

    virtual void parse(const std::string & str) {
        std::string err;
        json o = json::parse(str, err);
        json::object x = o.object_items();
        set_data(x);
    }
};


#define BEGIN_JSON_OBJECT(OBJECT_NAME)                                  \
    class OBJECT_NAME : public ::rmp::JSON {                            \
    private:                                                            \
        json::object m_data;                                    \
    protected:                                                          \
        virtual const json::object & get_data() const override {\
            return m_data;                                              \
        }                                                               \
        virtual void set_data(const json & json) override {     \
            m_data = json.object_items();                               \
        }                                                               \
        virtual void remove_member(const std::string & kw) override {   \
            m_data.erase(kw);                                           \
        }                                                               \
    public:                                                             \
        OBJECT_NAME (const std::string & str) : JSON() { parse(str);}   \
        OBJECT_NAME (const json & json) : JSON()                \
        {                                                               \
            set_data(json);                                             \
        }                                                               \
        OBJECT_NAME () : JSON() {}                                      \
        bool empty(void) const override                                 \
        {                                                               \
            return m_data.empty();                                      \
        }                                                               \
        void clear(void) override                                       \
        {                                                               \
            m_data.clear();                                             \
        }

#define JSON_PRIMITIVE(NAME, STRING, TYPE)                              \
    private:                                                            \
        static constexpr const char * const NAME##_word = STRING ;      \
        typedef TYPE NAME##_t;                                          \
    public:                                                             \
        bool has_##NAME() const                                         \
        {                                                               \
            return hasValue<NAME##_t>(NAME##_word);                     \
        }                                                               \
        NAME##_t get_##NAME() const                                     \
        {                                                               \
            return getValue<NAME##_t>(NAME##_word);                     \
        }                                                               \
        void set_##NAME(const NAME##_t & value)                         \
        {                                                               \
            m_data[NAME##_word] = value;                                \
        };                                                              \
        void set_##NAME(NAME##_t && value)                              \
        {                                                               \
            m_data[NAME##_word] = value;                                \
        };                                                              \
        virtual void remove_##NAME(void)                                \
        {                                                               \
            return remove_member(NAME##_word);                            \
        };

#define JSON_KEY(NAME)                          \
    public:                                     \
        json key() const                \
        {                                       \
            return get_##NAME();                \
        }                                       \
        static std::string key_name()           \
        {                                       \
            return std::string(NAME##_word);    \
        }

#define JSON_ARRAY(NAME, STRING)                                        \
    private:                                                            \
        static constexpr const char * const NAME##_word = STRING;       \
        typedef json::array NAME##_t;                           \
    public:                                                             \
        bool has_##NAME() const                                         \
        {                                                               \
            return hasValue<NAME##_t>(NAME##_word);                     \
        }                                                               \
        NAME##_t get_##NAME() const                                     \
        {                                                               \
            return getValue<NAME##_t>(NAME##_word);                     \
        }                                                               \
        void set_##NAME(const NAME##_t & value)                         \
        {                                                               \
            m_data[NAME##_word] = value;                                \
        }                                                               \
        void set_##NAME(NAME##_t && value)                              \
        {                                                               \
            m_data[NAME##_word] = value;                                \
        }                                                               \
        void add_to_##NAME(const json & v)                      \
        {                                                               \
            NAME##_t arr = has_##NAME() ? get_##NAME() : NAME##_t();    \
            NAME##_t::iterator itr = arr.begin();                       \
            while (itr != arr.end()) {                                  \
                if (*itr == v) {                                        \
                    arr.erase(itr);                                     \
                    break;                                              \
                }                                                       \
                itr++;                                                  \
            }                                                           \
            arr.push_back(v);                                           \
            set_##NAME(std::move(arr));                                 \
        }                                                               \
                                                                        \
        void remove_from_##NAME(const json & v) {               \
            NAME##_t arr = has_##NAME() ? get_##NAME() : NAME##_t();    \
            NAME##_t::iterator itr = arr.begin();                       \
            while (itr != arr.end()) {                                  \
                if (*itr == v) {                                        \
                    arr.erase(itr);                                     \
                    break;                                              \
                }                                                       \
                itr++;                                                  \
            }                                                           \
            set_##NAME(std::move(arr));                                 \
        }                                                               \
        bool contains_##NAME(const json & v) const {            \
            const NAME##_t arr =                                        \
                has_##NAME() ? get_##NAME() : NAME##_t();               \
            NAME##_t::const_iterator itr = arr.begin();                 \
            while (itr != arr.end()) {                                  \
                if (*itr == v) {                                        \
                    return true;                                        \
                }                                                       \
                itr++;                                                  \
            }                                                           \
            return false;                                               \
        }                                                               \
    private:

#define JSON_ARRAY_ITEMS(NAME, STRING, TYPE)                            \
    private:                                                            \
        static constexpr const char * const NAME##_word = STRING ;      \
        typedef TYPE NAME##_t;                                          \
    public:                                                             \
        bool has_##NAME() const                                         \
        {                                                               \
            return hasValue<json::array>(NAME##_word);          \
        }                                                               \
        json::array get_##NAME() const                          \
        {                                                               \
            return getValue<json::array>(NAME##_word);          \
        }                                                               \
        void set_##NAME(const json::array & value)              \
        {                                                               \
            m_data[NAME##_word] = value;                                \
        }                                                               \
        void set_##NAME(json::array && value)                   \
        {                                                               \
            m_data[NAME##_word] = value;                                \
        }                                                               \
        void add_to_##NAME(const NAME##_t & v)                          \
        {                                                               \
            json::array arr =                                   \
                has_##NAME() ? get_##NAME() : json::array();    \
            json::array::iterator itr = arr.begin();            \
            const auto & key = v.key();                                 \
            while (itr != arr.end()) {                                  \
                const auto & x = (*itr).object_items();                 \
                const json & value = x.at(TYPE::key_name());    \
                if (value == key) {                                     \
                    arr.erase(itr);                                     \
                    break;                                              \
                }                                                       \
                itr++;                                                  \
            }                                                           \
            arr.push_back(v.to_json());                                 \
            set_##NAME(std::move(arr));                                 \
        }                                                               \
                                                                        \
        void remove_from_##NAME(const NAME##_t & v) {                   \
            json::array arr =                                   \
                has_##NAME() ? get_##NAME() : json::array();    \
            json::array::iterator itr = arr.begin();            \
            const auto & key = v.key();                                 \
            while (itr != arr.end()) {                                  \
                const auto & x = (*itr).object_items();                 \
                const json & value = x.at(TYPE::key_name());    \
                if (value == key) {                                     \
                    arr.erase(itr);                                     \
                    break;                                              \
                }                                                       \
                itr++;                                                  \
            }                                                           \
            set_##NAME(std::move(arr));                                 \
        }                                                               \
        bool contains_##NAME(const NAME##_t & v) const {                \
            return contains_##NAME(v.key());                            \
        }                                                               \
        bool contains_##NAME(const json & key) const {          \
            const json::array arr =                             \
                has_##NAME() ? get_##NAME() : json::array();    \
            json::array::const_iterator itr = arr.begin();      \
            while (itr != arr.end()) {                                  \
                const auto & x = (*itr).object_items();                 \
                const json & value = x.at(TYPE::key_name());    \
                if (value == key) {                                     \
                    return true;                                        \
                }                                                       \
                itr++;                                                  \
            }                                                           \
            return false;                                               \
        }                                                               \
        NAME##_t get_##NAME(const NAME##_t & v) const {                 \
            return get_##NAME(v.key());                                 \
        }                                                               \
        NAME##_t get_##NAME(const json & key) const {           \
            const json::array arr =                             \
                has_##NAME() ? get_##NAME() : json::array();    \
            json::array::const_iterator itr = arr.begin();      \
            while (itr != arr.end()) {                                  \
                const auto & x = (*itr).object_items();                 \
                const json & value = x.at(TYPE::key_name());    \
                if (value == key) {                                     \
                    return NAME##_t(x);                                 \
                }                                                       \
                itr++;                                                  \
            }                                                           \
            return NAME##_t(std::string(""));                           \
        }                                                               \
    private:


#define JSON_NESTED_OBJECT(NAME, STRING, TYPE)                          \
    private:                                                            \
        static constexpr const char * const NAME##_word = STRING;       \
        typedef TYPE NAME##_t;                                          \
    public:                                                             \
        void set_##NAME(const NAME##_t & value)                         \
        {                                                               \
            m_data[NAME##_word] = value.to_json();                      \
        }                                                               \
        void set_##NAME(NAME##_t && value)                              \
        {                                                               \
            m_data[NAME##_word] = value.to_json();                      \
        }                                                               \
        bool has_##NAME() const                                         \
        {                                                               \
            return hasValue<Json::object>(NAME##_word);         \
        }                                                               \
        NAME##_t get_##NAME() const                                     \
        {                                                               \
            return NAME##_t(                                            \
                getValue<Json::object>(                         \
                    NAME##_word));                                      \
        }                                                               \
        void remove_##NAME(void)                                        \
        {                                                               \
            return remove_member(NAME##_word);                            \
        }                                                               \
    private:

#define END_JSON_OBJECT };

}