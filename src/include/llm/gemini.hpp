#pragma once

#include "llm/provider.hpp"
#include <string>
#include <vector>

class GeminiProvider : public LLMProvider {
private:
    std::string api_key;
    std::string model_name;
    std::vector<std::string> tools_json;  // JSON representation for API

public:
    GeminiProvider(const std::string& model, const std::string& key);
    
    std::string chat(const std::string& user_message, 
                     const std::vector<std::map<std::string, std::string>>& history = {}) override;
    
    void addTool(const std::string& name, const std::string& description, 
                 const std::string& parameters) override;
    
    std::string name() const override { return "gemini"; }
    void clearTools() override { tools.clear(); tools_json.clear(); }
    // getTools() inherited from base class
};
