import { Box, Typography } from '@mui/material';

function SubscriptionsPanel() {
  return (
    <Box sx={{ p: 3 }}>
      <Typography variant="h4" component="h1" gutterBottom>
        Subscriptions
      </Typography>
      <Typography variant="body1">Manage subscriptions and billing. View active plans and payment history.</Typography>
    </Box>
  );
}

export default SubscriptionsPanel;
