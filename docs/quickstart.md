# Quick Start Guide

Get up and running with OLLMCPC in 60 seconds.

## 1. Run the Setup Script
This handles dependencies and builds the project.
```bash
chmod +x scripts/setup.sh
./scripts/setup.sh
```

## 2. Configure Your Models
Run the wizard to set your preferred provider.
```bash
./build/ollmcpc config
```

## 3. Pull a Model (If using Ollama)
```bash
ollama pull functiongemma
```

## 4. Start Chatting!
```bash
./build/ollmcpc serve
```

## 5. Try a Tool
Ask something like:
> "What is my current CPU usage and what are the top 5 largest files in my home directory?"

Accept the HIL prompts and watch your AI interact with your system!
