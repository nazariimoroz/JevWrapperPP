#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace jvpp {

class JevRequestBuilder;
class TSJevClient;

class JevRequest {
  public:
    struct State {
      public:
        constexpr explicit State(std::string_view value) noexcept : m_value(value) {}

        [[nodiscard]] constexpr std::string_view value() const noexcept { return m_value; }

      private:
        std::string_view m_value;
    };

    struct Noul {
        std::string name;
        std::string instruction;
    };

    struct Choise {
        std::string name;
        std::string instruction;
        std::vector<std::string> values;
    };

    struct Score {
        std::string name;
        std::string instruction;
        float from;
        float to;
    };

  private:
    friend class JevRequestBuilder;
    friend class TSJevClient;

    JevRequest(State state, std::vector<Noul> nouls, std::vector<Choise> choises,
               std::vector<Score> scores)
        : m_state(state), m_nouls(std::move(nouls)), m_choises(std::move(choises)),
          m_scores(std::move(scores)) {}

    State m_state;
    std::vector<Noul> m_nouls;
    std::vector<Choise> m_choises;
    std::vector<Score> m_scores;
};

} // namespace jvpp
