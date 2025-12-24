#pragma once

#include <algorithm>
#include <array>
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <windows.h>

#include <ModUtils/Patterns.h>
#include <log.h>

namespace re4t::patterns::detail
{
    struct SectionRange
    {
        uintptr_t begin = 0;
        uintptr_t end = 0;
    };

    inline std::optional<SectionRange> find_section_range(HMODULE module, std::string_view sectionName)
    {
        const auto baseAddress = reinterpret_cast<uintptr_t>(module);
        const auto dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(baseAddress);
        const auto ntHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(baseAddress + dosHeader->e_lfanew);

        auto* section = IMAGE_FIRST_SECTION(ntHeader);
        for (WORD index = 0; index < ntHeader->FileHeader.NumberOfSections; ++index, ++section)
        {
            size_t nameLength = 0;
            while (nameLength < IMAGE_SIZEOF_SHORT_NAME && section->Name[nameLength] != '\0')
            {
                ++nameLength;
            }

            const std::string_view currentName(reinterpret_cast<const char*>(section->Name), nameLength);
            if (currentName != sectionName)
            {
                continue;
            }

            const DWORD sectionSize = std::max(section->Misc.VirtualSize, section->SizeOfRawData);
            if (sectionSize == 0)
            {
                return std::nullopt;
            }

            const uintptr_t begin = baseAddress + section->VirtualAddress;
            const uintptr_t end = begin + sectionSize;
            return SectionRange{ begin, end };
        }

        return std::nullopt;
    }

    inline void log_section_hit(std::string_view patternString, std::string_view sectionName)
    {
        spd::log()->warn(R"(re4t::pattern("{0}") matched in {1})", patternString, sectionName);
    }

    inline void log_section_miss(std::string_view patternString, const std::string& sections)
    {
        spd::log()->warn(R"(re4t::pattern("{0}") not found in .text or sections [{1}])", patternString, sections);
    }

    template <typename Range>
    inline std::string join_sections(const Range& sections)
    {
        std::string joined;
        bool first = true;
        for (const auto sectionName : sections)
        {
            if (!first)
            {
                joined.append(", ");
            }

            joined.append(sectionName.data(), sectionName.size());
            first = false;
        }

        return joined;
    }

    inline constexpr std::array<std::string_view, 4> kDefaultSections =
    {
        std::string_view{ "PSFD00" },
        std::string_view{ ".rdata" },
        std::string_view{ ".data" },
        std::string_view{ ".idata" }
    };

    template <typename Range>
    inline std::optional<hook::pattern> try_sections(std::string_view patternString, const Range& sections, HMODULE module)
    {
        for (const auto sectionName : sections)
        {
            const auto range = find_section_range(module, sectionName);
            auto sectionPattern = hook::make_range_pattern(range->begin, range->end, patternString);
            if (!sectionPattern.empty())
            {
                log_section_hit(patternString, sectionName);
                return sectionPattern;
            }
        }

        return std::nullopt;
    }
}

namespace re4t
{
	// Wraps hook::pattern to search in specified sections if not found in .text
    inline hook::pattern pattern(std::string_view patternString, std::initializer_list<std::string_view> sections)
    {
        const auto module = GetModuleHandle(nullptr);
		const auto hasSections = sections.size();

        if (const auto fallback = hasSections
            ? patterns::detail::try_sections(patternString, sections, module)
            : patterns::detail::try_sections(patternString, patterns::detail::kDefaultSections, module))
        {
            return *fallback;
        }

        patterns::detail::log_section_miss(patternString, hasSections
            ? patterns::detail::join_sections(sections)
            : patterns::detail::join_sections(patterns::detail::kDefaultSections));

        return hook::pattern(patternString);
    }

    inline hook::pattern pattern(std::string_view patternString)
    {
        hook::pattern textPattern(patternString);
        if (!textPattern.empty())
        {
            return textPattern;
        }
        return pattern(patternString, {});
    }
}
