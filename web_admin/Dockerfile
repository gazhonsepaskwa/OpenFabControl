FROM node:18-alpine

WORKDIR /app

# Copy package files
COPY package*.json ./

# Install dependencies
RUN npm ci

# Copy source files
COPY . .

# Build the application
RUN npm run build && echo "✓ Web admin built successfully"

# Copy files from /app/dist to /web_admin_dist so they persists after the container exits
CMD ["sh", "-c", "cp -rf /app/dist/* /web_admin_dist/ && echo '✓ Web admin files ready in /web_admin_dist volume'"]

