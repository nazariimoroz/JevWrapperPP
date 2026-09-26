#pragma once

#include <JevWrapperPP/error.hpp>
#include <JevWrapperPP/jev_response.hpp>

#include <expected>
#include <string>
#include <utility>

namespace jvpp {

class JevRequest;

class TSJevClient {
  public:
    TSJevClient() = default;
    explicit TSJevClient(std::string api_key, std::string model = "jev-latest")
        : m_api_key(std::move(api_key)), m_model(std::move(model)) {}

    [[nodiscard]] static TSJevClient Create(std::string api_key, std::string model = "jev-latest") {
        return TSJevClient(std::move(api_key), std::move(model));
    }

    [[nodiscard]] std::expected<JevResponse<>, Error> operator()(const JevRequest& request) const;
    [[nodiscard]] bool check_connection() const;

  private:
    std::string m_api_key;
    std::string m_model{"jev-latest"};
};

} // namespace jvpp
