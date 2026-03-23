import { Box, Typography } from '@mui/material';

function RolesPanel() {
  return (
    <Box sx={{ p: 3 }}>
      <Typography variant="h4" component="h1" gutterBottom>
        Roles
      </Typography>
      <Typography variant="body1">
        Manage roles and permissions. Define access levels for different user groups.
      </Typography>
    </Box>
  );
}

export default RolesPanel;
