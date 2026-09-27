#pragma once

#include <JevWrapperPP/error.hpp>
#include <JevWrapperPP/jev_response_result.hpp>

#include <expected>
#include <functional>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

namespace jvpp {

template <class... Results> class JevResponse;
class TSJevClient;

namespace detail {

template <class Result> struct JevProcessor {
    template <class Process>
    JevProcessor(std::string name, Process&& process)
        : m_name(std::move(name)), m_process(std::forward<Process>(process)) {}

    std::string m_name;
    std::move_only_function<Result()> m_process;
};

} // namespace detail

template <> class JevResponse<> {
  public:
    [[nodiscard]] const std::string& payload() const { return m_payload; }

    template <class Process>
    [[nodiscard]] auto processor(
        std::string name,
        Process&& process) && -> JevResponse<std::remove_cvref_t<std::invoke_result_t<Process&>>> {
        using Result = std::remove_cvref_t<std::invoke_result_t<Process&>>;
        static_assert(!std::is_void_v<Result>, "A processor must return a value.");

        return JevResponse<Result>(
            detail::JevProcessor<Result>(std::move(name), std::forward<Process>(process)));
    }

  private:
    friend class TSJevClient;

    explicit JevResponse(std::string payload) : m_payload(std::move(payload)) {}

    std::string m_payload;
};

template <class... Results> class JevResponse {
  public:
    using Result = JevResponseResult<Results...>;

    explicit JevResponse(detail::JevProcessor<Results>... processors)
        : m_processors(std::move(processors)...) {}

    template <class Process>
    [[nodiscard]] auto processor(std::string name, Process&& process) && -> JevResponse<
        Results..., std::remove_cvref_t<std::invoke_result_t<Process&>>> {
        using NextResult = std::remove_cvref_t<std::invoke_result_t<Process&>>;
        static_assert(!std::is_void_v<NextResult>, "A processor must return a value.");

        return std::apply(
            [name = std::move(name),
             process = std::forward<Process>(process)](auto&&... processors) mutable {
                return JevResponse<Results..., NextResult>(
                    std::move(processors)...,
                    detail::JevProcessor<NextResult>(std::move(name), std::move(process)));
            },
            std::move(m_processors));
    }

    [[nodiscard]] std::expected<Result, Error> operator()() && {
        auto values = typename Result::Values();

        std::apply(
            [&values](auto&... processors) {
                (values.insert_or_assign(processors.m_name,
                                         std::variant<Results...>(processors.m_process())),
                 ...);
            },
            m_processors);

        return Result(std::move(values));
    }

  private:
    std::tuple<detail::JevProcessor<Results>...> m_processors;
};

} // namespace jvpp
