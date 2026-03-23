# Web Admin

Admin panel for OpenFabControl.

## Prerequisites

- Node.js 18+
- npm

## Development

```bash
# Install dependencies
npm install

# Start dev server
npm run dev
```

Access at `http://localhost:5173`

## Build

```bash
# Build for production
npm run build
```

## Linting

```bash
npm run lint
```

## Production Run

The web_admin is automatically built as part of the docker-compose stack triggered by the `make` command before being served by nginx.

```bash
# From project root
make
```

Access at `https://localhost:4080`

## API Endpoints

API requests are proxied by nginx:

- `/web-admin-api/*` → Backend admin routes
- `/web-user-api/*` → Backend user routes
- `/machine-api/*` → Backend machine routes
