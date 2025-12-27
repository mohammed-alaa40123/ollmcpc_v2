# Deployment Guide

## GitHub Pages Deployment

OLLMCPC documentation is automatically deployed to GitHub Pages using GitHub Actions.

### Setup Instructions

1. **Enable GitHub Pages** in your repository settings:
   - Go to **Settings** → **Pages**
   - Under **Source**, select **Deploy from a branch**
   - Select the `gh-pages` branch
   - Click **Save**

2. **Push to trigger deployment**:
   ```bash
   git add .
   git commit -m "Add documentation"
   git push origin main
   ```

3. **Access your docs**:
   Your documentation will be available at:
   `https://mohammed-alaa40123.github.io/ollmcpc_v2/`

### How it Works

The GitHub Actions workflow (`.github/workflows/docs.yml`) automatically:
1. Triggers on every push to `main` or `master` branch
2. Sets up Python and installs MkDocs Material
3. Builds the documentation
4. Deploys to the `gh-pages` branch

### Manual Deployment

You can also deploy manually from your local machine:

```bash
# Install dependencies first
pip install mkdocs-material

# Deploy
mkdocs gh-deploy
```

This will build the docs and push them to the `gh-pages` branch.

### Custom Domain (Optional)

To use a custom domain:
1. Add a `CNAME` file to the `docs/` directory with your domain name
2. Configure your DNS provider to point to GitHub Pages
3. Enable custom domain in repository settings
