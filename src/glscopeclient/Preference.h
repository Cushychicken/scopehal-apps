/***********************************************************************************************************************
*                                                                                                                      *
* ANTIKERNEL v0.1                                                                                                      *
*                                                                                                                      *
* Copyright (c) 2012-2020 Andrew D. Zonenberg                                                                          *
* All rights reserved.                                                                                                 *
*                                                                                                                      *
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the     *
* following conditions are met:                                                                                        *
*                                                                                                                      *
*    * Redistributions of source code must retain the above copyright notice, this list of conditions, and the         *
*      following disclaimer.                                                                                           *
*                                                                                                                      *
*    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the       *
*      following disclaimer in the documentation and/or other materials provided with the distribution.                *
*                                                                                                                      *
*    * Neither the name of the author nor the names of any contributors may be used to endorse or promote products     *
*      derived from this software without specific prior written permission.                                           *
*                                                                                                                      *
* THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED   *
* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL *
* THE AUTHORS BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES        *
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR       *
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE       *
* POSSIBILITY OF SUCH DAMAGE.                                                                                          *
*                                                                                                                      *
***********************************************************************************************************************/

/**
	@file
	@author Katharina B.
	@brief  Basic preference class and auxilliary types
 */

#ifndef Preference_h
#define Preference_h

#include <string>
#include <type_traits>
#include <utility>
#include <cstdint>

#include <giomm.h>
#include <gtkmm.h>

#include "Unit.h"

constexpr std::size_t max(std::size_t a, std::size_t b)
{
    return (a > b) ? a : b;
}

enum class PreferenceType
{
    Boolean,
    String,
    Real,
    Color,
    None // Only for moved-from values
};

namespace impl
{
    class PreferenceBuilder;

    struct Color
    {
        Color(std::uint16_t r, std::uint16_t g, std::uint16_t b)
            : m_r{r}, m_g{g}, m_b{b}
        {

        }

        std::uint16_t m_r, m_g, m_b;
    };
}

class Preference
{
    friend class impl::PreferenceBuilder;

private:
    using PreferenceValue = typename std::aligned_union<
        max(sizeof(impl::Color), max(max(sizeof(bool), sizeof(double)), sizeof(std::string))),
        bool, std::string, double, impl::Color
    >::type;

public:
    // Taking string as value and then moving is intended
    Preference(std::string identifier, std::string label, std::string description, bool defaultValue)
        :   m_identifier{std::move(identifier)}, m_label{std::move(label)}, m_description{std::move(description)},
            m_type{PreferenceType::Boolean}
    {
        new (&m_value) bool(defaultValue);
    }
    
    Preference(std::string identifier, std::string label, std::string description, std::string defaultValue)
        :   m_identifier{std::move(identifier)}, m_label{std::move(label)}, m_description{std::move(description)},
            m_type{PreferenceType::String}
    {
        new (&m_value) std::string(std::move(defaultValue));
    }
    
    Preference(std::string identifier, std::string label, std::string description, const char* defaultValue)
        :   m_identifier{std::move(identifier)}, m_label{std::move(label)}, m_description{std::move(description)},
            m_type{PreferenceType::String}
    {
        new (&m_value) std::string(defaultValue);
    }
    
    Preference(std::string identifier, std::string label, std::string description, double defaultValue)
        :   m_identifier{std::move(identifier)}, m_label{std::move(label)}, m_description{std::move(description)},
            m_type{PreferenceType::Real}
    {
        new (&m_value) double(defaultValue);
    }

    Preference(std::string identifier, std::string label, std::string description, const Gdk::Color& defaultValue)
        :   m_identifier{std::move(identifier)}, m_label{std::move(label)}, m_description{std::move(description)},
            m_type{PreferenceType::Color}
    {
        new (&m_value) impl::Color(defaultValue.get_red(), defaultValue.get_green(), defaultValue.get_blue());
    }
    
    ~Preference()
    {
        CleanUp();
    }

public:
    static impl::PreferenceBuilder New(std::string identifier, std::string label, std::string description, bool defaultValue);
    static impl::PreferenceBuilder New(std::string identifier, std::string label, std::string description, double defaultValue);
    static impl::PreferenceBuilder New(std::string identifier, std::string label, std::string description, const char* defaultValue);
    static impl::PreferenceBuilder New(std::string identifier, std::string label, std::string description, std::string defaultValue);
    static impl::PreferenceBuilder New(std::string identifier, std::string label, std::string description, const Gdk::Color& defaultValue);

public:
    Preference(const Preference&) = delete;
    Preference(Preference&& other)
    {
        MoveFrom(other);
    }
    
    Preference& operator=(const Preference&) = delete;
    Preference& operator=(Preference&& other)
    {
        CleanUp();
        MoveFrom(other);
        return *this;
    }
    
public:
    const std::string& GetIdentifier() const;
    const std::string& GetLabel() const;
    const std::string& GetDescription() const;
    PreferenceType GetType() const;
    bool GetBool() const;
    double GetReal() const;
    const std::string& GetString() const;
    std::string ToString() const;
    bool GetIsVisible() const;
    Gdk::Color GetColor() const;
    const impl::Color& GetColorRaw() const;

    void SetBool(bool value);
    void SetReal(double value);
    void SetString(std::string value);
    void SetColor(const Gdk::Color& value);
    void SetColorRaw(const impl::Color& value);

    bool HasUnit();
    Unit& GetUnit();
    
private:
    template<typename T>
    const T& GetValueRaw() const
    {
        return *reinterpret_cast<const T*>(&m_value);
    }
    
    template<typename T>
    T& GetValueRaw()
    {
        return *reinterpret_cast<T*>(&m_value);
    }
    
    void CleanUp();
    
    template<typename T>
    void Construct(T value)
    {
        new (&m_value) T(std::move(value));
    }
    
    void MoveFrom(Preference& other);
    
private:
    std::string m_identifier;
    std::string m_label;
    std::string m_description;
    PreferenceType m_type;
    PreferenceValue m_value;
    bool m_isVisible{true};
    Unit m_unit{Unit::UNIT_COUNTS};
};

namespace impl
{
    class PreferenceBuilder
    {
        public:
            PreferenceBuilder(Preference&& pref);

        public:
            PreferenceBuilder&& IsVisible(bool isVisible) &&;
            PreferenceBuilder&& WithUnit(Unit::UnitType type) &&;
            Preference&& Build() &&;

        protected:
            Preference m_pref;
    };
}

#endif // Preference_h
