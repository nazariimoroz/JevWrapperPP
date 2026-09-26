#include <JevWrapperPP/TypeSafe_Jev/ts_jev_client.hpp>
#include <JevWrapperPP/jev_request_builder.hpp>
#include <JevWrapperPP/jev_response_result.hpp>

#include <array>
#include <optional>
#include <span>
#include <string>
#include <utility>

int main() {
    using namespace jvpp;

    const auto client = TSJevClient();
    const auto configured_client = TSJevClient::Create("test key");
    const auto check_connection = &TSJevClient::check_connection;
    const auto state = JevRequest::State("test");
    auto choices = std::array<std::string, 2>();
    choices[0] = "42";
    choices[1] = "7";
    auto request = JevRequestBuilder::Create(state)
                       .noul("answer", "What is the answer?")
                       .noul("explanation", "Explain your answer.")
                       .choise("answer", "Select an answer.", {choices})
                       .choise("alternative", "Select an alternative.", {choices})
                       .score("confidence", "Score confidence.")
                       .score("quality", "Score quality.", 0.2F, 0.8F);
    const auto copied_request = request.build();
    const auto moved_request = std::move(request).build();

    static_cast<void>(client);
    static_cast<void>(configured_client);
    static_cast<void>(check_connection);
    static_cast<void>(copied_request);
    static_cast<void>(moved_request);

    const auto response = client(copied_request);
    if (response || response.error().category() != "authentication") {
        return 1;
    }

    auto values = JevResponseResult<int, std::string>::Values();
    values.emplace("number", 42);
    values.emplace("text", std::string("done"));
    const auto result = JevResponseResult<int, std::string>(std::move(values));

    if (result.get<int>("number") != std::optional<int>(42)) {
        return 1;
    }
    if (result.get<std::string>("text") != std::optional<std::string>("done")) {
        return 1;
    }
    if (result.get<int>("text") || result.get<int>("missing")) {
        return 1;
    }

    return 0;
}
