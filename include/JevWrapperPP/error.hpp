#pragma once

#include <string>
#include <string_view>
#include <utility>

namespace jvpp {

class Error {
  public:
    Error(std::string_view category, std::string message)
        : m_category(category), m_message(std::move(message)) {}

    [[nodiscard]] constexpr std::string_view category() const noexcept { return m_category; }
    [[nodiscard]] const std::string& message() const noexcept { return m_message; }

  private:
    std::string_view m_category;
    std::string m_message;
};

} // namespace jvpp
