import CheckCircleOutlineIcon from '@mui/icons-material/CheckCircleOutline';
import { Alert, Avatar, Box, Button, CircularProgress, TextField, Typography } from '@mui/material';
import { useState } from 'react';

interface ConfirmEmailPageProps {
  /** The activation code extracted from the ?code= query parameter. */
  activationCode: string;
  /** Called when the setup is complete so the parent can navigate to login. */
  onSetupComplete: () => void;
}

export default function ConfirmEmailPage({ activationCode, onSetupComplete }: ConfirmEmailPageProps) {
  const [fields, setFields] = useState({
    activation_code: activationCode,
    first_name: '',
    last_name: '',
    password: '',
    tva: '',
    facturation_address: '',
    facturation_account: '',
  });
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const handleChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    setFields((prev) => ({ ...prev, [e.target.name]: e.target.value }));
  };

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    setError(null);
    setLoading(true);

    try {
      const response = await fetch(`https://${window.location.hostname}:4080/web-user-api/user_one_time_setup`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(fields),
      });

      const data = await response.json().catch(() => ({}));

      if (!response.ok) {
        setError((data as { error?: string }).error ?? 'Setup failed. Please try again.');
        return;
      }

      onSetupComplete();
    } catch (err) {
      setError('An error occurred. Please try again.');
      console.error('Confirm email error:', err);
    } finally {
      setLoading(false);
    }
  };

  return (
    <Box
      sx={{
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        justifyContent: 'center',
        minHeight: '80vh',
        px: 2,
      }}
    >
      <Avatar sx={{ mb: 2, bgcolor: 'primary.main' }}>
        <CheckCircleOutlineIcon />
      </Avatar>

      <Typography component="h1" variant="h5" gutterBottom>
        Confirm your account
      </Typography>

      <Typography variant="body2" color="text.secondary" sx={{ mb: 2, textAlign: 'center', maxWidth: 400 }}>
        Fill in the details below to activate your account.
      </Typography>

      <Box component="form" onSubmit={handleSubmit} sx={{ mt: 1, width: '100%', maxWidth: 480 }}>
        <TextField
          margin="normal"
          required
          fullWidth
          label="Activation code"
          name="activation_code"
          value={fields.activation_code}
          onChange={handleChange}
          disabled={loading}
        />
        <TextField
          margin="normal"
          required
          fullWidth
          label="First name"
          name="first_name"
          value={fields.first_name}
          onChange={handleChange}
          disabled={loading}
        />
        <TextField
          margin="normal"
          required
          fullWidth
          label="Last name"
          name="last_name"
          value={fields.last_name}
          onChange={handleChange}
          disabled={loading}
        />
        <TextField
          margin="normal"
          required
          fullWidth
          label="Password"
          name="password"
          type="password"
          autoComplete="new-password"
          value={fields.password}
          onChange={handleChange}
          disabled={loading}
        />
        <TextField
          margin="normal"
          fullWidth
          label="VAT number (TVA)"
          name="tva"
          value={fields.tva}
          onChange={handleChange}
          disabled={loading}
        />
        <TextField
          margin="normal"
          fullWidth
          label="Billing address"
          name="facturation_address"
          value={fields.facturation_address}
          onChange={handleChange}
          disabled={loading}
        />
        <TextField
          margin="normal"
          fullWidth
          label="Billing account"
          name="facturation_account"
          value={fields.facturation_account}
          onChange={handleChange}
          disabled={loading}
        />

        {error && (
          <Alert severity="error" sx={{ mt: 2 }}>
            {error}
          </Alert>
        )}

        <Button
          type="submit"
          fullWidth
          variant="contained"
          sx={{ mt: 3, mb: 2 }}
          disabled={loading}
          startIcon={loading ? <CircularProgress size={18} color="inherit" /> : null}
        >
          {loading ? 'Activating…' : 'Activate account'}
        </Button>
      </Box>
    </Box>
  );
}
