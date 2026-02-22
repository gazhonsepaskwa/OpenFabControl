import {
  Alert,
  Box,
  Button,
  CircularProgress,
  Dialog,
  DialogActions,
  DialogContent,
  DialogTitle,
  IconButton,
  List,
  ListItem,
  ListItemText,
  Snackbar,
  Stack,
  TextField,
  Typography,
} from '@mui/material';
import DeleteIcon from '@mui/icons-material/Delete';
import { useEffect, useState } from 'react';

const API_BASE = '/web-admin-api';

interface Role {
  name: string;
}

function RolesPanel() {
  const [roles, setRoles] = useState<Role[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [newRoleName, setNewRoleName] = useState('');
  const [snackbar, setSnackbar] = useState({ open: false, message: '' });
  const [deleteConfirmDialog, setDeleteConfirmDialog] = useState({ open: false, roleName: '' });

  const fetchRoles = async () => {
    setLoading(true);
    setError(null);
    try {
      const res = await fetch(`${API_BASE}/get_role_list`);

      if (!res.ok) {
        throw new Error('Failed to fetch roles');
      }

      const data = await res.json();
      setRoles(data || []);
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Unknown error');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchRoles();
  }, []);

  const handleCreateRole = async () => {
    if (!newRoleName.trim()) {
      setSnackbar({ open: true, message: 'Role name cannot be empty' });
      return;
    }

    try {
      const res = await fetch(`${API_BASE}/create_role`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ role_name: newRoleName }),
      });

      if (res.ok) {
        setNewRoleName('');
        await fetchRoles();
      } else {
        throw new Error('API call failed');
      }
    } catch (err) {
      const errorMsg = 'Failed to create role';
      console.error(errorMsg, err);
      setSnackbar({ open: true, message: `${errorMsg}: ${err instanceof Error ? err.message : 'Unknown error'}` });
    }
  };

  const handleCloseSnackbar = () => {
    setSnackbar({ open: false, message: '' });
  };

  const handleDeleteRoleClick = (roleName: string) => {
    setDeleteConfirmDialog({ open: true, roleName });
  };

  const handleConfirmDelete = async () => {
    try {
      const res = await fetch(`${API_BASE}/delete_role`, {
        method: 'DELETE',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ role_name: deleteConfirmDialog.roleName }),
      });

      if (res.ok) {
        setDeleteConfirmDialog({ open: false, roleName: '' });
        await fetchRoles();
      } else {
        throw new Error('API call failed');
      }
    } catch (err) {
      const errorMsg = 'Failed to delete role';
      console.error(errorMsg, err);
      setSnackbar({ open: true, message: `${errorMsg}: ${err instanceof Error ? err.message : 'Unknown error'}` });
      setDeleteConfirmDialog({ open: false, roleName: '' });
    }
  };

  const handleCancelDelete = () => {
    setDeleteConfirmDialog({ open: false, roleName: '' });
  };

  if (loading) {
    return (
      <Box sx={{ p: 3, display: 'flex', justifyContent: 'center' }}>
        <CircularProgress />
      </Box>
    );
  }

  if (error) {
    return (
      <Box sx={{ p: 3 }}>
        <Typography color="error">Error: {error}</Typography>
      </Box>
    );
  }

  return (
    <Box sx={{ p: 3 }}>
      <Typography variant="h4" component="h1" gutterBottom>
        Roles
      </Typography>
      <Typography variant="h6" gutterBottom>
        Available Roles ({roles.length})
      </Typography>
      {roles.length === 0 ? (
        <Typography color="text.secondary">No roles available</Typography>
      ) : (
        <List sx={{ width: 'fit-content' }}>
          {roles.map((role) => (
            <ListItem
              key={role.name}
              secondaryAction={
                <IconButton
                  edge="end"
                  aria-label="delete"
                  onClick={() => handleDeleteRoleClick(role.name)}
                  color="error"
                >
                  <DeleteIcon />
                </IconButton>
              }
            >
              <ListItemText primary={role.name} />
            </ListItem>
          ))}
        </List>
      )}

      <Box sx={{ mt: 4, pt: 3, borderTop: '1px solid', borderColor: 'divider' }}>
        <Typography variant="h6" gutterBottom>
          Create New Role
        </Typography>
        <Stack direction="row" spacing={2} sx={{ mt: 2 }}>
          <TextField
            label="Role Name"
            value={newRoleName}
            onChange={(e) => setNewRoleName(e.target.value)}
            variant="outlined"
            size="small"
          />
          <Button variant="contained" onClick={handleCreateRole}>
            Create Role
          </Button>
        </Stack>
      </Box>

      <Snackbar
        open={snackbar.open}
        autoHideDuration={6000}
        onClose={handleCloseSnackbar}
        anchorOrigin={{ vertical: 'bottom', horizontal: 'center' }}
      >
        <Alert onClose={handleCloseSnackbar} severity="error" sx={{ width: '100%' }}>
          {snackbar.message}
        </Alert>
      </Snackbar>

      {/* Delete confirmation dialog */}
      <Dialog
        open={deleteConfirmDialog.open}
        onClose={handleCancelDelete}
        aria-labelledby="alert-dialog-title"
        aria-describedby="alert-dialog-description"
      >
        <DialogTitle id="alert-dialog-title">Delete Role</DialogTitle>
        <DialogContent>
          <Typography id="alert-dialog-description">
            Are you sure you want to delete this role? This action cannot be undone.
          </Typography>
        </DialogContent>
        <DialogActions>
          <Button onClick={handleCancelDelete}>Cancel</Button>
          <Button onClick={handleConfirmDelete} color="error" variant="contained">
            Delete
          </Button>
        </DialogActions>
      </Dialog>
    </Box>
  );
}

export default RolesPanel;
