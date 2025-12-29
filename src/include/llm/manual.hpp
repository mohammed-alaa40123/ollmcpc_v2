#pragma once

#include "llm/provider.hpp"

class ManualProvider : public LLMProvider {
public:
    std::string chat(const std::string& user_message, 
                     const std::vector<std::map<std::string, std::string>>& history = {}) override;
    
    void addTool(const std::string& name, const std::string& description, 
                 const std::string& parameters) override;
    
    std::string name() const override { return "manual"; }
    // clearTools() and getTools() inherited from base class
};
