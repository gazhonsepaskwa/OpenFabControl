import { Box, Typography } from '@mui/material';

export default function BookingPanel() {
  return (
    <Box sx={{ p: 3 }}>
      <Typography variant="h5">Booking</Typography>
      <Typography variant="body1" sx={{ mt: 1, color: 'text.secondary' }}>
        Booking management coming soon.
      </Typography>
    </Box>
  );
}
