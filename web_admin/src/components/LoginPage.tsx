import LockOutlinedIcon from '@mui/icons-material/LockOutlined';
import { Alert, Avatar, Box, Button, CircularProgress, TextField, Typography } from '@mui/material';
import { useState } from 'react';

interface Role {
  id: number;
  name: string;
  created_at: string;
}

interface User {
  id: number;
  email: string;
  access_key: string;
  first_name: string;
  last_name: string;
  tva: string;
  facturation_address: string;
  facturation_account: string;
  status: string;
  created_at: string;
  roles: Role[];
}

interface LoginPageProps {
  onLoginSuccess: () => void;
}

export default function LoginPage({ onLoginSuccess }: LoginPageProps) {
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const handleSubmit = async (e: { preventDefault(): void }) => {
    e.preventDefault();
    setError(null);
    setLoading(true);

    try {
      const response = await fetch(`https://${window.location.hostname}:4080/web-user-api/login`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ email, password }),
      });

      const data = await response.json();

      if (!response.ok) {
        setError(data.error ?? 'Login failed');
        return;
      }

      const token: string = data.token;
      const user: User = data.user;

      sessionStorage.setItem('token', token);
      sessionStorage.setItem('user', JSON.stringify(user));

      onLoginSuccess();
    } catch (err) {
      setError('An error occurred. Please try again.');
      console.error('Login error:', err);
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
        <LockOutlinedIcon />
      </Avatar>

      <Typography component="h1" variant="h5" gutterBottom>
        OpenFabControl
      </Typography>

      <Box component="form" onSubmit={handleSubmit} sx={{ mt: 1, width: '100%', maxWidth: 360 }}>
        <TextField
          margin="normal"
          required
          fullWidth
          label="Email address"
          type="email"
          autoComplete="email"
          autoFocus
          value={email}
          onChange={(e) => setEmail(e.target.value)}
          disabled={loading}
        />

        <TextField
          margin="normal"
          required
          fullWidth
          label="Password"
          type="password"
          autoComplete="current-password"
          value={password}
          onChange={(e) => setPassword(e.target.value)}
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
          {loading ? 'Signing in…' : 'Sign in'}
        </Button>
      </Box>
    </Box>
  );
}
