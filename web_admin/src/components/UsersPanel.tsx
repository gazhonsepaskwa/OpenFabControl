import { Box, Typography } from '@mui/material';

function UsersPanel() {
  return (
    <Box sx={{ p: 3 }}>
      <Typography variant="h4" component="h1" gutterBottom>
        Users
      </Typography>
      <Typography variant="body1">Manage users in your system. Add, edit, or remove user accounts.</Typography>
    </Box>
  );
}

export default UsersPanel;
