#pragma once

#include <JevWrapperPP/jev_request.hpp>

#include <span>
#include <string>
#include <utility>
#include <vector>

namespace jvpp {

class JevRequestBuilder {
  public:
    [[nodiscard]] static JevRequestBuilder Create(JevRequest::State state) {
        return JevRequestBuilder(state);
    }

  protected:
    JevRequestBuilder(JevRequest::State state) : m_state(state) {}

  public:
    [[nodiscard]] JevRequestBuilder&& noul(std::string name, std::string instruction) && {
        m_nouls.push_back(JevRequest::Noul(std::move(name), std::move(instruction)));
        return std::move(*this);
    }

    [[nodiscard]] JevRequestBuilder&& choise(std::string name, std::string instruction,
                                             const std::span<const std::string>& choises) && {
        m_choises.push_back(
            JevRequest::Choise(std::move(name), std::move(instruction),
                               std::vector<std::string>(choises.begin(), choises.end())));
        return std::move(*this);
    }

    [[nodiscard]] JevRequestBuilder&& score(std::string name, std::string instruction,
                                            float from = 0.0F, float to = 1.0F) && {
        m_scores.push_back(JevRequest::Score(std::move(name), std::move(instruction), from, to));
        return std::move(*this);
    }

    [[nodiscard]] JevRequest build() && {
        return JevRequest(m_state, std::move(m_nouls), std::move(m_choises), std::move(m_scores));
    }

    [[nodiscard]] JevRequest build() const& {
        return JevRequest(m_state, m_nouls, m_choises, m_scores);
    }

  private:
    JevRequest::State m_state;
    std::vector<JevRequest::Noul> m_nouls;
    std::vector<JevRequest::Choise> m_choises;
    std::vector<JevRequest::Score> m_scores;
};

} // namespace jvpp
