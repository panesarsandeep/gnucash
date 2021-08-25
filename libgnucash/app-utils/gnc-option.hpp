/********************************************************************\
 * gnc-option.hpp -- Application options system                     *
 * Copyright (C) 2020 John Ralls <jralls@ceridwen.us>               *
 *                                                                  *
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 51 Franklin Street, Fifth Floor    Fax:    +1-617-542-2652       *
 * Boston, MA  02110-1301,  USA       gnu@gnu.org                   *
 *                                                                  *
\********************************************************************/

#ifndef GNC_OPTION_HPP_
#define GNC_OPTION_HPP_

#include <glib.h>
#include <libguile.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <variant>
#include <memory>
#include "gnc-option-ui.hpp"
#include "gnc-option-date.hpp"

struct OptionClassifier;
class GncOptionUIItem;
using GncOptionUIItemPtr = std::unique_ptr<GncOptionUIItem>;
#ifndef SWIG //SWIG pulls in GncOwner from swig-engine.
struct _gncOwner;
using GncOwner = _gncOwner;
#endif
struct _QofQuery;
using QofQuery = _QofQuery;
struct QofInstance_s;
using QofInstance = QofInstance_s;
template <typename ValueType> class GncOptionValue;
class GncOptionAccountListValue;
class GncOptionAccountSelValue;
class GncOptionMultichoiceValue;
template <typename ValueType> class GncOptionRangeValue;
template <typename ValueType> class GncOptionValidatedValue;
class GncOptionDateValue;

template <typename T>
struct is_OptionClassifier
{
    static constexpr bool value =
        std::is_base_of_v<OptionClassifier, std::decay_t<T>>;
};

template <typename T> inline constexpr bool
is_OptionClassifier_v = is_OptionClassifier<T>::value;

template <typename T, typename U>
struct is_same_decayed
{
    static constexpr bool value = std::is_same_v<std::decay_t<T>,
                                                 std::decay_t<U>>;
};

template <typename T, typename U> inline constexpr bool
is_same_decayed_v = is_same_decayed<T, U>::value;

template <typename T>
struct is_RangeValue
{
    static constexpr bool value =
         (is_same_decayed_v<T, GncOptionRangeValue<int>> ||
          is_same_decayed_v<T, GncOptionRangeValue<double>>);
};

template <typename T> inline constexpr bool
is_RangeValue_v = is_RangeValue<T>::value;


using GncOptionVariant = std::variant<GncOptionValue<std::string>,
                                      GncOptionValue<bool>,
                                      GncOptionValue<int64_t>,
                                      GncOptionValue<const QofInstance*>,
                                      GncOptionValue<const QofQuery*>,
                                      GncOptionValue<const GncOwner*>,
                                      GncOptionValue<SCM>,
                                      GncOptionAccountListValue,
                                      GncOptionAccountSelValue,
                                      GncOptionMultichoiceValue,
                                      GncOptionRangeValue<int>,
                                      GncOptionRangeValue<double>,
                                      GncOptionValidatedValue<const QofInstance*>,
                                      GncOptionValidatedValue<const QofQuery*>,
                                      GncOptionDateValue>;

using GncOptionVariantPtr = std::unique_ptr<GncOptionVariant>;

enum class GncOptionMultichoiceKeyType
{
    SYMBOL,
    STRING,
    NUMBER,
};

class GncOption
{
public:
    template <typename OptionType,
              typename std::enable_if_t<is_OptionClassifier_v<OptionType>,
                               int>  = 0>

    GncOption(OptionType option) :
        m_option{std::make_unique<GncOptionVariant>(option)} {}
    template <typename ValueType,
              typename std::enable_if_t<!is_OptionClassifier_v<ValueType>,
                               int>  = 0>
    GncOption(const char* section, const char* name,
              const char* key, const char* doc_string,
              ValueType value,
              GncOptionUIType ui_type = GncOptionUIType::INTERNAL);
    template <typename ValueType> void set_value(ValueType value);
    template <typename ValueType> void set_default_value(ValueType value);
    template <typename ValueType> ValueType get_default_value() const;
    template <typename ValueType> ValueType get_value() const;
    void reset_default_value();

    const std::string& get_section() const;
    const std::string& get_name() const;
    const std::string& get_key() const;
    const std::string& get_docstring() const;
    void set_ui_item(GncOptionUIItemPtr&& ui_elem);
    const GncOptionUIType get_ui_type() const;
    void set_ui_item_selectable(bool) const noexcept;
    GncOptionUIItem* const get_ui_item() const;
    void set_ui_item_from_option();
    void set_option_from_ui_item();
    void make_internal();
    bool is_changed() const noexcept;
    bool is_multiselect() const noexcept;
    template <typename ValueType> void get_limits(ValueType&, ValueType&,
                                                  ValueType&) const noexcept;
    template <typename ValueType> bool validate(ValueType value) const;
    std::size_t num_permissible_values() const;
    std::size_t permissible_value_index(const char* value) const;
    const char* permissible_value(std::size_t index) const;
    const char* permissible_value_name(std::size_t index) const;
    GList* account_type_list() const noexcept;
    bool is_alternate() const noexcept;
    void set_alternate(bool) noexcept;
    std::ostream& out_stream(std::ostream& oss) const;
    std::istream& in_stream(std::istream& iss);
    std::ostream& to_scheme(std::ostream& oss) const;
    std::istream& from_scheme(std::istream& iss);


    friend GncOptionVariant& swig_get_option(GncOption*);

private:
    inline static const std::string c_empty_string{""};
    GncOptionVariantPtr m_option;
    GncOptionUIItemPtr m_ui_item{nullptr};
};

inline bool
operator<(const GncOption& right, const GncOption& left)
{
    return right.get_key() < left.get_key();
}

inline std::ostream&
operator<<(std::ostream& oss, const GncOption& opt)
{
    return opt.out_stream(oss);
}

inline std::istream&
operator>>(std::istream& iss, GncOption& opt)
{
    return opt.in_stream(iss);
}

inline std::ostream&
output_color_value(std::ostream& oss, const std::string& value)
{
    oss << "'(";
    oss << std::fixed << std::showpoint << std::setprecision(1);
    auto len{value.length() > 8 ? 8 : value.length()};
    for (size_t i{}; i < len; i += 2)
    {
        oss << static_cast<float>(stoi(value.substr(i, 2), nullptr, 16));
        if (i < 6)
            oss << " ";
    }
    if (len < 8)
        oss << 256.0;
    oss << ")";
    return oss;
}

template<typename ValueType> GncOption*
gnc_make_option(const char* section, const char* name,
                const char* key, const char* doc_string,
                ValueType value, GncOptionUIType ui_type)
{
    return new GncOption(section, name, key, doc_string, value, ui_type);
}

/* To work around SWIG_Guile's typedef of SCM to unsigned long: */
GncOption* gnc_make_SCM_option(const char* section, const char* name,
                               const char* key, const char* doc_string,
                               SCM value, GncOptionUIType ui_type);

#endif //GNC_OPTION_HPP_
