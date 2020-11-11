//
// Created by faitc on 10/29/2020.
//

#ifndef IOT_SMART_GATEWAY_SESSION_MANAGER_H
#define IOT_SMART_GATEWAY_SESSION_MANAGER_H

#include <vector>
#include <algorithm>
#include <unordered_map>
#include <boost/random.hpp>

namespace Simple {
    constexpr const size_t default_max_sessions = 1000;

    template<typename Session>
    class SessionManager {
        std::unordered_map<size_t, Session> sessions;
        const size_t max_sessions;

        [[nodiscard]] typename std::unordered_map<size_t, Session>::iterator find_session_or_throw(const size_t & sess_handle) {
            if (sessions.empty()) throw std::exception();// SessionInvalid("Manager is empty");
            auto p_sess = sessions.find(sess_handle);
            if (p_sess == sessions.end()) throw std::exception();// SessionInvalid("Session does not exist");
            return p_sess;
        }

        [[nodiscard]] static size_t gen_handle(const size_t &upper_limit) {
            static boost::mt19937 randomizer(time(nullptr));
            static boost::uniform_int<> allowed_keys(0, upper_limit);
            static boost::variate_generator<boost::mt19937, boost::uniform_int<>> keygen(randomizer, allowed_keys);
            return keygen();
        }

    public:
        class SessionInvalid : std::exception {
            std::string e;
        public:
            SessionInvalid() = delete;

            SessionInvalid(SessionInvalid &&) = delete;

            SessionInvalid(const SessionInvalid &) = delete;

            explicit SessionInvalid(std::string reason)
                    : e(std::move(reason)) {
            };

            [[nodiscard]] const char *what() const noexcept override {
                return e.c_str();
            }
        };

        explicit SessionManager(size_t max_sessions = default_max_sessions) : sessions(), max_sessions(max_sessions) {};
        SessionManager(const SessionManager &) = delete;
        SessionManager(SessionManager &&)      = delete;
        SessionManager& operator=(const SessionManager &) = delete;
        SessionManager& operator=(SessionManager &&)      = delete;

        template<typename ...Params>
        [[nodiscard]] size_t create(Params &&... params) {
            if (sessions.size() >= max_sessions) throw std::exception();// SessionInvalid("Manager is full");
            size_t sess_handle;// = gen_unique_key(gen_secret(max_sessions));
            do sess_handle = gen_handle(max_sessions); while (sessions.find(sess_handle) != sessions.end());
            sessions[sess_handle] = std::move(Session(std::forward<Params>(params)...));
            return sess_handle;
        }

        [[nodiscard]] Session &get(const size_t &sess_handle) {
            return std::forward<Session &>(find_session_or_throw(sess_handle)->second);
        }

        [[nodiscard]] std::vector<std::pair<Session, size_t>> collect() const {
            std::vector<std::pair<Session, size_t>> sessions_vec;
            std::transform(sessions.begin(), sessions.end(), std::back_inserter(sessions_vec), [](auto session) { return std::make_pair(session.second, session.first); });
            return sessions_vec;
        }

        void remove(const size_t &sess_handle) {
            sessions.erase(find_session_or_throw(sess_handle)->first);
        }

        [[nodiscard]] size_t count() const { return sessions.size(); }
        [[nodiscard]] bool empty() const { return sessions.empty(); }
    };
}

#endif //IOT_SMART_GATEWAY_SESSION_MANAGER_H
