#include <JevWrapperPP/TypeSafe_Jev/ts_jev_client.hpp>

#include <JevWrapperPP/jev_request.hpp>

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <expected>
#include <memory>
#include <string>
#include <utility>

namespace {

constexpr auto kEndpoint = "https://api.typesafe.ai/v1/systemone";

auto append_response(char* contents, std::size_t size, std::size_t count, void* output)
    -> std::size_t {
    const auto length = size * count;
    static_cast<std::string*>(output)->append(contents, length);
    return length;
}

} // namespace

namespace jvpp {

auto TSJevClient::operator()(const JevRequest& request) const
    -> std::expected<JevResponse<>, Error> {
    if (m_api_key.empty()) {
        return std::unexpected{Error{"authentication", "An API key is required."}};
    }

    nlohmann::json payload = {
        {"model", m_model},
        {"state", request.m_state.value()},
        {"questions", nlohmann::json::object()},
    };

    for (const auto& noul : request.m_nouls) {
        payload["questions"][noul.name] = {
            {"type", "noul"},
            {"instructions", noul.instruction},
        };
    }

    for (const auto& choise : request.m_choises) {
        auto criteria = nlohmann::json::object();
        for (const auto& value : choise.values) {
            criteria[value] = value;
        }

        payload["questions"][choise.name] = {
            {"type", "choice"},
            {"instructions", choise.instruction},
            {"criteria", std::move(criteria)},
        };
    }

    for (const auto& score : request.m_scores) {
        payload["questions"][score.name] = {
            {"type", "score"},
            {"instructions", score.instruction},
            {"criteria", {std::to_string(score.from), std::to_string(score.to)}},
        };
    }

    static const auto curl_initialized = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (curl_initialized != CURLE_OK) {
        return std::unexpected{Error{"curl", curl_easy_strerror(curl_initialized)}};
    }

    auto curl =
        std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>(curl_easy_init(), curl_easy_cleanup);
    if (!curl) {
        return std::unexpected{Error{"curl", "Unable to create a curl handle."}};
    }

    auto headers =
        std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)>(nullptr, curl_slist_free_all);
    const auto append_header = [&headers](const std::string& value) -> std::expected<void, Error> {
        const auto updated = curl_slist_append(headers.get(), value.c_str());
        if (updated == nullptr) {
            return std::unexpected{Error{"curl", "Unable to create request headers."}};
        }

        headers.reset(updated);
        return {};
    };

    if (const auto result = append_header("Content-Type: application/json"); !result) {
        return std::unexpected{result.error()};
    }
    if (const auto result = append_header("Authorization: Bearer " + m_api_key); !result) {
        return std::unexpected{result.error()};
    }

    const auto body = payload.dump();
    std::string response_body;

    const auto set_option = [handle = curl.get()](CURLoption option,
                                                  auto value) -> std::expected<void, Error> {
        const auto result = curl_easy_setopt(handle, option, value);
        if (result != CURLE_OK) {
            return std::unexpected{Error{"curl", curl_easy_strerror(result)}};
        }

        return {};
    };

    if (const auto result = set_option(CURLOPT_URL, kEndpoint); !result) {
        return std::unexpected{result.error()};
    }
    if (const auto result = set_option(CURLOPT_POST, 1L); !result) {
        return std::unexpected{result.error()};
    }
    if (const auto result = set_option(CURLOPT_POSTFIELDS, body.c_str()); !result) {
        return std::unexpected{result.error()};
    }
    if (const auto result =
            set_option(CURLOPT_POSTFIELDSIZE_LARGE, static_cast<curl_off_t>(body.size()));
        !result) {
        return std::unexpected{result.error()};
    }
    if (const auto result = set_option(CURLOPT_HTTPHEADER, headers.get()); !result) {
        return std::unexpected{result.error()};
    }
    if (const auto result = set_option(CURLOPT_WRITEFUNCTION, append_response); !result) {
        return std::unexpected{result.error()};
    }
    if (const auto result = set_option(CURLOPT_WRITEDATA, &response_body); !result) {
        return std::unexpected{result.error()};
    }
    if (const auto result = set_option(CURLOPT_TIMEOUT_MS, 30000L); !result) {
        return std::unexpected{result.error()};
    }

    if (const auto result = curl_easy_perform(curl.get()); result != CURLE_OK) {
        return std::unexpected{Error{"curl", curl_easy_strerror(result)}};
    }

    long status_code = 0;
    if (const auto result = curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &status_code);
        result != CURLE_OK) {
        return std::unexpected{Error{"curl", curl_easy_strerror(result)}};
    }

    if (status_code < 200 || status_code >= 300) {
        return std::unexpected{
            Error{"http", "The Jev API returned HTTP " + std::to_string(status_code) + "."}};
    }

    const auto response = nlohmann::json::parse(response_body, nullptr, false);
    if (response.is_discarded() || !response.contains("answers") ||
        !response["answers"].is_object()) {
        return std::unexpected{Error{"response", "The Jev API returned an invalid response body."}};
    }

    return JevResponse<>(response_body);
}

auto TSJevClient::check_connection() const -> bool {
    static const auto curl_initialized = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (curl_initialized != CURLE_OK) {
        return false;
    }

    auto curl =
        std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>(curl_easy_init(), curl_easy_cleanup);
    if (!curl) {
        return false;
    }

    if (curl_easy_setopt(curl.get(), CURLOPT_URL, kEndpoint) != CURLE_OK ||
        curl_easy_setopt(curl.get(), CURLOPT_NOBODY, 1L) != CURLE_OK ||
        curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT_MS, 5000L) != CURLE_OK) {
        return false;
    }

    return curl_easy_perform(curl.get()) == CURLE_OK;
}

} // namespace jvpp
