#include <a2a/client/card_resolver.hpp>
#include <a2a/core/exception.hpp>

namespace a2a {

// PIMPL implementation
class A2ACardResolver::Impl {
public:
    Impl(const std::string& base_url, const std::string& agent_card_path)
        : base_url_(base_url)
        , agent_card_path_(agent_card_path)
        , http_client_() {
        
        // Construct full URL
        if (base_url_.back() == '/') {
            base_url_.pop_back();
        }
        
        if (agent_card_path_.front() != '/') {
            agent_card_url_ = base_url_ + "/" + agent_card_path_;
        } else {
            agent_card_url_ = base_url_ + agent_card_path_;
        }
    }
    
    std::string base_url_;
    std::string agent_card_path_;
    std::string agent_card_url_;
    HttpClient http_client_;
};

A2ACardResolver::A2ACardResolver(const std::string& base_url,
                                 const std::string& agent_card_path)
    : impl_(std::make_unique<Impl>(base_url, agent_card_path)) {}

A2ACardResolver::~A2ACardResolver() = default;

A2ACardResolver::A2ACardResolver(A2ACardResolver&&) noexcept = default;
A2ACardResolver& A2ACardResolver::operator=(A2ACardResolver&&) noexcept = default;

AgentCard A2ACardResolver::get_agent_card() {
    try {
        // Perform GET request
        auto response = impl_->http_client_.get(impl_->agent_card_url_);
        
        // Check response status
        if (!response.is_success()) {
            throw A2AException(
                "Failed to fetch agent card: HTTP " + std::to_string(response.status_code),
                ErrorCode::InternalError
            );
        }
        
        // Parse JSON response
        AgentCard card = AgentCard::from_json(response.body);
        
        return card;
        
    } catch (const A2AException&) {
        throw;
    } catch (const std::exception& e) {
        throw A2AException(
            std::string("Failed to get agent card: ") + e.what(),
            ErrorCode::InternalError
        );
    }
}

std::string A2ACardResolver::get_agent_card_url() const {
    return impl_->agent_card_url_;
}

} // namespace a2a
