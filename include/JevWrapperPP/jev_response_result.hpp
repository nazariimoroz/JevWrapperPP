#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>

namespace jvpp {

template <class... Results> class JevResponseResult {
  public:
    using Values = std::unordered_map<std::string, std::variant<Results...>>;

    explicit JevResponseResult(Values values) : m_values(std::move(values)) {}

    template <class T> [[nodiscard]] std::optional<T> get(std::string_view name) const {
        const auto value = m_values.find(std::string(name));
        if (value == m_values.end()) {
            return std::nullopt;
        }

        if (const auto result = std::get_if<T>(&value->second)) {
            return *result;
        }

        return std::nullopt;
    }

  private:
    Values m_values;
};

} // namespace jvpp
