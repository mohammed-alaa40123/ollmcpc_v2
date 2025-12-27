#!/bin/bash
# serve_docs.sh - Set up a venv and serve the documentation website

set -e

DOCS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VENV_PATH="$DOCS_DIR/.venv_docs"

echo "📂 Setting up documentation environment in $VENV_PATH..."

if [ ! -d "$VENV_PATH" ]; then
    python3 -m venv "$VENV_PATH"
    echo "✅ Virtual environment created."
fi

source "$VENV_PATH/bin/activate"

echo "⬇️  Installing/Updating dependencies (MkDocs, Material Theme)..."
pip install -q mkdocs mkdocs-material

echo "🚀 Starting MkDocs server..."
echo "🔗 Your documentation will be available at: http://127.0.0.1:8000"
echo "💡 Press Ctrl+C to stop the server."

mkdocs serve
