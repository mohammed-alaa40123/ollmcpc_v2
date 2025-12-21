#pragma once

#include "llm/provider.hpp"
#include "llm/ollama.hpp"
#include "llm/gemini.hpp"
#include "llm/manual.hpp"
#include "app/config.hpp"
#include <memory>

inline std::unique_ptr<LLMProvider> createProvider(const Config& config) {
    if (config.default_provider == "gemini") {
        return std::make_unique<GeminiProvider>(config.gemini_model, config.gemini_api_key);
    } else if (config.default_provider == "manual" || config.default_provider == "none") {
        return std::make_unique<ManualProvider>();
    } else {
        return std::make_unique<OllamaProvider>(config.ollama_model);
    }
}
