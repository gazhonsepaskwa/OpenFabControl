import basicSsl from '@vitejs/plugin-basic-ssl';
import react from '@vitejs/plugin-react';
import { defineConfig } from 'vite';

// https://vite.dev/config/
export default defineConfig({
  plugins: [react(), basicSsl()],
  server: {
    proxy: {
      '/web-admin-api': {
        target: 'https://localhost:4080',
        changeOrigin: true,
        secure: false,
      },
    },
  },
});
