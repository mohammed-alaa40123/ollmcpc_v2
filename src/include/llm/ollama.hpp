#pragma once

#include "llm/provider.hpp"
#include <string>
#include <vector>

class OllamaProvider : public LLMProvider {
private:
    std::string model_name;
    std::string ollama_url;
    std::vector<std::string> tools_json;  // JSON representation for API

public:
    OllamaProvider(const std::string& model);
    
    std::string chat(const std::string& user_message, 
                     const std::vector<std::map<std::string, std::string>>& history = {}) override;
    
    void addTool(const std::string& name, const std::string& description, 
                 const std::string& parameters) override;
    
    std::string name() const override { return "ollama"; }
    void clearTools() override { tools.clear(); tools_json.clear(); }
    // getTools() inherited from base class
};
