#pragma once

#include "llm/provider.hpp"
#include <string>
#include <vector>

class ManualProvider : public LLMProvider {
private:
    std::vector<Tool> tools;

public:
    std::string chat(const std::string& user_message, 
                     const std::vector<std::map<std::string, std::string>>& history = {}) override;
    
    void addTool(const std::string& name, const std::string& description, 
                 const std::string& parameters) override;
    std::string name() const override { return "manual"; }
    void clearTools() override { tools.clear(); }
    std::vector<Tool> getTools() const override { return tools; }
};
