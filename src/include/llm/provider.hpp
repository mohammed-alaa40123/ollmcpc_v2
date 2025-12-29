#pragma once

#include <string>
#include <vector>
#include <map>

struct Tool {
    std::string name;
    std::string description;
    std::string parameters;
};

class LLMProvider {
protected:
    std::vector<Tool> tools;
    
    // Helper: Check if tool already exists
    bool hasToolNamed(const std::string& name) const {
        for (const auto& t : tools) {
            if (t.name == name) return true;
        }
        return false;
    }

public:
    virtual ~LLMProvider() = default;
    
    virtual std::string chat(const std::string& user_message, 
                             const std::vector<std::map<std::string, std::string>>& history = {}) = 0;
    
    virtual void addTool(const std::string& name, const std::string& description, 
                         const std::string& parameters) = 0;
    
    virtual std::string name() const = 0;
    
    virtual void clearTools() { tools.clear(); }
    virtual std::vector<Tool> getTools() const { return tools; }
};
