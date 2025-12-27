#pragma once

#include "llm/provider.hpp"
#include <string>
#include <vector>

class OllamaProvider : public LLMProvider {
private:
    std::string model_name;
    std::string ollama_url;
    std::vector<std::string> tools_json;
    std::vector<std::string> registered_tool_names;

public:
    OllamaProvider(const std::string& model);
    
    std::string chat(const std::string& user_message, 
                     const std::vector<std::map<std::string, std::string>>& history = {}) override;
    
    void addTool(const std::string& name, const std::string& description, 
                 const std::string& parameters) override;
    std::string name() const override { return "ollama"; }
    void clearTools() override { tools_json.clear(); registered_tool_names.clear(); tools.clear(); }
    std::vector<Tool> getTools() const override { return tools; }

private:
    std::vector<Tool> tools;
};
