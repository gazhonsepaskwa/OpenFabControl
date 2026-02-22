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
  Snackbar,
  Stack,
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableHead,
  TableRow,
  TextField,
  Typography,
  Paper,
} from '@mui/material';
import EditIcon from '@mui/icons-material/Edit';
import DeleteIcon from '@mui/icons-material/Delete';
import { useEffect, useState } from 'react';

const API_BASE = '/web-admin-api';

interface User {
  id: string;
  email: string;
  access_key: string;
  password: string;
  first_name: string;
  last_name: string;
  tva: string;
  facturation_address: string;
  facturation_account: string;
  activation_code: string;
  status: string;
  created_at: string;
}

function UsersPanel() {
  const [users, setUsers] = useState<User[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [snackbar, setSnackbar] = useState({ open: false, message: '' });
  const [createDialogOpen, setCreateDialogOpen] = useState(false);
  const [newUserEmail, setNewUserEmail] = useState('');
  const [newUserAccessKey, setNewUserAccessKey] = useState('');
  const [editDialogOpen, setEditDialogOpen] = useState(false);
  const [editingUser, setEditingUser] = useState<User | null>(null);
  const [deleteConfirmDialog, setDeleteConfirmDialog] = useState({ open: false, userId: '' });

  const fetchUsers = async () => {
    setLoading(true);
    setError(null);
    try {
      const res = await fetch(`${API_BASE}/get_user_list`);

      if (!res.ok) {
        throw new Error('Failed to fetch users');
      }

      const data = await res.json();
      setUsers(data || []);
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Unknown error');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchUsers();
  }, []);

  const handleCloseSnackbar = () => {
    setSnackbar({ open: false, message: '' });
  };

  const handleOpenCreateDialog = () => {
    setCreateDialogOpen(true);
  };

  const handleCloseCreateDialog = () => {
    setCreateDialogOpen(false);
    setNewUserEmail('');
    setNewUserAccessKey('');
  };

  const handleCreateUser = async () => {
    if (!newUserEmail.trim()) {
      setSnackbar({ open: true, message: 'Email cannot be empty' });
      return;
    }

    if (!newUserAccessKey.trim()) {
      setSnackbar({ open: true, message: 'Access key cannot be empty' });
      return;
    }

    try {
      const res = await fetch(`${API_BASE}/create_user`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ email: newUserEmail, access_key: newUserAccessKey }),
      });

      if (res.ok) {
        setNewUserEmail('');
        setNewUserAccessKey('');
        setCreateDialogOpen(false);
        await fetchUsers();
      } else {
        throw new Error('API call failed');
      }
    } catch (err) {
      const errorMsg = 'Failed to create user';
      console.error(errorMsg, err);
      setSnackbar({ open: true, message: `${errorMsg}: ${err instanceof Error ? err.message : 'Unknown error'}` });
    }
  };

  const handleOpenEditDialog = (user: User) => {
    setEditingUser({ ...user });
    setEditDialogOpen(true);
  };

  const handleCloseEditDialog = () => {
    setEditDialogOpen(false);
    setEditingUser(null);
  };

  const handleUpdateUser = async () => {
    if (!editingUser) return;

    try {
      const { created_at, password, status, activation_code, ...userToUpdate } = editingUser;
      const res = await fetch(`${API_BASE}/update_user`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ ...userToUpdate, id: String(userToUpdate.id) }),
      });

      if (res.ok) {
        setEditDialogOpen(false);
        setEditingUser(null);
        await fetchUsers();
      } else {
        throw new Error('API call failed');
      }
    } catch (err) {
      const errorMsg = 'Failed to update user';
      console.error(errorMsg, err);
      setSnackbar({ open: true, message: `${errorMsg}: ${err instanceof Error ? err.message : 'Unknown error'}` });
    }
  };

  const handleEditFieldChange = (field: keyof User, value: string) => {
    if (editingUser) {
      setEditingUser({ ...editingUser, [field]: value });
    }
  };

  const handleDeleteUserClick = (userId: string) => {
    setDeleteConfirmDialog({ open: true, userId });
  };

  const handleConfirmDelete = async () => {
    try {
      const res = await fetch(`${API_BASE}/delete_user`, {
        method: 'DELETE',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ user_id: String(deleteConfirmDialog.userId) }),
      });

      if (res.ok) {
        setDeleteConfirmDialog({ open: false, userId: '' });
        await fetchUsers();
      } else {
        throw new Error('API call failed');
      }
    } catch (err) {
      const errorMsg = 'Failed to delete user';
      console.error(errorMsg, err);
      setSnackbar({ open: true, message: `${errorMsg}: ${err instanceof Error ? err.message : 'Unknown error'}` });
      setDeleteConfirmDialog({ open: false, userId: '' });
    }
  };

  const handleCancelDelete = () => {
    setDeleteConfirmDialog({ open: false, userId: '' });
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
        Users
      </Typography>
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 2 }}>
        <Typography variant="h6">Total Users ({users.length})</Typography>
        <Button variant="contained" onClick={handleOpenCreateDialog}>
          Create User
        </Button>
      </Box>

      {users.length === 0 ? (
        <Typography color="text.secondary">No users available</Typography>
      ) : (
        <TableContainer component={Paper}>
          <Table sx={{ minWidth: 650 }} aria-label="users table">
            <TableHead>
              <TableRow sx={{ backgroundColor: 'action.hover' }}>
                <TableCell>
                  <strong>ID</strong>
                </TableCell>
                <TableCell>
                  <strong>Email</strong>
                </TableCell>
                <TableCell>
                  <strong>First Name</strong>
                </TableCell>
                <TableCell>
                  <strong>Last Name</strong>
                </TableCell>
                <TableCell>
                  <strong>Status</strong>
                </TableCell>
                <TableCell>
                  <strong>TVA</strong>
                </TableCell>
                <TableCell>
                  <strong>Facturation Address</strong>
                </TableCell>
                <TableCell>
                  <strong>Facturation Account</strong>
                </TableCell>
                <TableCell>
                  <strong>Access Key</strong>
                </TableCell>
                <TableCell>
                  <strong>Activation Code</strong>
                </TableCell>
                <TableCell>
                  <strong>Created At</strong>
                </TableCell>
                <TableCell>
                  <strong>Actions</strong>
                </TableCell>
              </TableRow>
            </TableHead>
            <TableBody>
              {users.map((user) => (
                <TableRow key={user.id} hover sx={{ '&:last-child td, &:last-child th': { border: 0 } }}>
                  <TableCell>{user.id}</TableCell>
                  <TableCell>{user.email}</TableCell>
                  <TableCell>{user.first_name}</TableCell>
                  <TableCell>{user.last_name}</TableCell>
                  <TableCell>{user.status}</TableCell>
                  <TableCell>{user.tva || 'N/A'}</TableCell>
                  <TableCell>{user.facturation_address || 'N/A'}</TableCell>
                  <TableCell>{user.facturation_account || 'N/A'}</TableCell>
                  <TableCell sx={{ wordBreak: 'break-all', maxWidth: 150 }}>{user.access_key}</TableCell>
                  <TableCell sx={{ wordBreak: 'break-all', maxWidth: 150 }}>{user.activation_code || 'N/A'}</TableCell>
                  <TableCell>{new Date(user.created_at).toLocaleDateString()}</TableCell>
                  <TableCell>
                    <IconButton
                      size="small"
                      onClick={() => handleOpenEditDialog(user)}
                      color="primary"
                    >
                      <EditIcon />
                    </IconButton>
                    <IconButton
                      size="small"
                      onClick={() => handleDeleteUserClick(user.id)}
                      color="error"
                    >
                      <DeleteIcon />
                    </IconButton>
                  </TableCell>
                </TableRow>
              ))}
            </TableBody>
          </Table>
        </TableContainer>
      )}

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

      {/* Create user dialog */}
      <Dialog
        open={createDialogOpen}
        onClose={handleCloseCreateDialog}
        aria-labelledby="create-user-dialog-title"
      >
        <DialogTitle id="create-user-dialog-title">Create New User</DialogTitle>
        <DialogContent>
          <Stack spacing={2} sx={{ mt: 2 }}>
            <TextField
              label="Email"
              value={newUserEmail}
              onChange={(e) => setNewUserEmail(e.target.value)}
              variant="outlined"
              fullWidth
              type="email"
            />
            <TextField
              label="Access Key"
              value={newUserAccessKey}
              onChange={(e) => setNewUserAccessKey(e.target.value)}
              variant="outlined"
              fullWidth
            />
          </Stack>
        </DialogContent>
        <DialogActions>
          <Button onClick={handleCloseCreateDialog}>Cancel</Button>
          <Button onClick={handleCreateUser} variant="contained" color="primary">
            Create User
          </Button>
        </DialogActions>
      </Dialog>

      {/* Edit user dialog */}
      <Dialog
        open={editDialogOpen}
        onClose={handleCloseEditDialog}
        aria-labelledby="edit-user-dialog-title"
        maxWidth="sm"
        fullWidth
      >
        <DialogTitle id="edit-user-dialog-title">Edit User</DialogTitle>
        <DialogContent>
          <Stack spacing={2} sx={{ mt: 2 }}>
            <TextField
              label="ID"
              value={editingUser?.id || ''}
              variant="outlined"
              fullWidth
              disabled
            />
            <TextField
              label="Email"
              value={editingUser?.email || ''}
              onChange={(e) => handleEditFieldChange('email', e.target.value)}
              variant="outlined"
              fullWidth
            />
            <TextField
              label="Access Key"
              value={editingUser?.access_key || ''}
              onChange={(e) => handleEditFieldChange('access_key', e.target.value)}
              variant="outlined"
              fullWidth
            />
            <TextField
              label="First Name"
              value={editingUser?.first_name || ''}
              onChange={(e) => handleEditFieldChange('first_name', e.target.value)}
              variant="outlined"
              fullWidth
            />
            <TextField
              label="Last Name"
              value={editingUser?.last_name || ''}
              onChange={(e) => handleEditFieldChange('last_name', e.target.value)}
              variant="outlined"
              fullWidth
            />
            <TextField
              label="TVA"
              value={editingUser?.tva || ''}
              onChange={(e) => handleEditFieldChange('tva', e.target.value)}
              variant="outlined"
              fullWidth
            />
            <TextField
              label="Facturation Address"
              value={editingUser?.facturation_address || ''}
              onChange={(e) => handleEditFieldChange('facturation_address', e.target.value)}
              variant="outlined"
              fullWidth
              multiline
              rows={2}
            />
            <TextField
              label="Facturation Account"
              value={editingUser?.facturation_account || ''}
              onChange={(e) => handleEditFieldChange('facturation_account', e.target.value)}
              variant="outlined"
              fullWidth
            />
          </Stack>
        </DialogContent>
        <DialogActions>
          <Button onClick={handleCloseEditDialog}>Cancel</Button>
          <Button onClick={handleUpdateUser} variant="contained" color="primary">
            Update User
          </Button>
        </DialogActions>
      </Dialog>

      {/* Delete user confirmation dialog */}
      <Dialog
        open={deleteConfirmDialog.open}
        onClose={handleCancelDelete}
        aria-labelledby="delete-user-dialog-title"
        aria-describedby="delete-user-dialog-description"
      >
        <DialogTitle id="delete-user-dialog-title">Delete User</DialogTitle>
        <DialogContent>
          <Typography id="delete-user-dialog-description">
            Are you sure you want to delete this user? This action cannot be undone.
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

export default UsersPanel;
