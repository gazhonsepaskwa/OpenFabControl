import { Box, Typography } from '@mui/material';

export default function SettingsPanel() {
  return (
    <Box sx={{ p: 3 }}>
      <Typography variant="h4" component="h1" gutterBottom>Settings</Typography>
      <Typography variant="body1" sx={{ mt: 1, color: 'text.secondary' }}>
        Application settings coming soon.
      </Typography>
    </Box>
  );
}
