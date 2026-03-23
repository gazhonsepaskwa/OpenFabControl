import DeleteIcon from '@mui/icons-material/Delete';
import EditIcon from '@mui/icons-material/Edit';
import SecurityIcon from '@mui/icons-material/Security';
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
  MenuItem,
  Paper,
  Select,
  SelectChangeEvent,
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
} from '@mui/material';
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
  status: string;
  created_at: string;
}

interface Role {
  id: number;
  name: string;
}

function UsersPanel() {
  const [users, setUsers] = useState<User[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [snackbar, setSnackbar] = useState({ open: false, message: '' });
  const [userDialogOpen, setUserDialogOpen] = useState(false);
  const [isEditMode, setIsEditMode] = useState(false);
  const [editingUser, setEditingUser] = useState<User | null>(null);
  const [deleteConfirmDialog, setDeleteConfirmDialog] = useState({ open: false, userId: '' });
  const [rolesDialogOpen, setRolesDialogOpen] = useState(false);
  const [userRoles, setUserRoles] = useState<Role[]>([]);
  const [rolesLoading, setRolesLoading] = useState(false);
  const [selectedUserForRoles, setSelectedUserForRoles] = useState<string | null>(null);
  const [availableRoles, setAvailableRoles] = useState<Role[]>([]);
  const [selectedRoleId, setSelectedRoleId] = useState<string>('');

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
    setIsEditMode(false);
    setEditingUser({
      id: '',
      email: '',
      access_key: '',
      password: '',
      first_name: '',
      last_name: '',
      tva: '',
      facturation_address: '',
      facturation_account: '',
      status: '',
      created_at: '',
    });
    setUserDialogOpen(true);
  };

  const handleOpenEditDialog = (user: User) => {
    setIsEditMode(true);
    setEditingUser({ ...user });
    setUserDialogOpen(true);
  };

  const handleCloseUserDialog = () => {
    setUserDialogOpen(false);
    setEditingUser(null);
  };

  const handleUserFieldChange = (field: keyof User, value: string) => {
    if (editingUser) {
      setEditingUser({ ...editingUser, [field]: value });
    }
  };

  const validateUserForm = (): string | null => {
    if (!editingUser) return 'No user data';

    if (!editingUser.email.trim()) {
      return 'Email cannot be empty';
    }

    if (!editingUser.access_key.trim()) {
      return 'Access key cannot be empty';
    }

    return null;
  };

  const handleSaveUser = async () => {
    if (!editingUser) return;

    const validationError = validateUserForm();
    if (validationError) {
      setSnackbar({ open: true, message: validationError });
      return;
    }

    const endpoint = isEditMode ? '/update_user' : '/create_user';
    const errorMsg = isEditMode ? 'Failed to update user' : 'Failed to create user';

    let payload;
    if (isEditMode) {
      const { created_at: _created_at, password: _password, status: _status, ...userToUpdate } = editingUser;
      payload = userToUpdate;
    } else {
      payload = { email: editingUser.email, access_key: editingUser.access_key };
    }

    try {
      const res = await fetch(`${API_BASE}${endpoint}`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload),
      });

      if (res.ok) {
        setUserDialogOpen(false);
        setEditingUser(null);
        await fetchUsers();
      } else {
        throw new Error('API call failed');
      }
    } catch (err) {
      console.error(errorMsg, err);
      setSnackbar({ open: true, message: `${errorMsg}: ${err instanceof Error ? err.message : 'Unknown error'}` });
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
        body: JSON.stringify({ user_id: deleteConfirmDialog.userId }),
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

  const handleOpenRolesDialog = async (userId: string) => {
    setSelectedUserForRoles(userId);
    setRolesLoading(true);
    setUserRoles([]);
    setSelectedRoleId('');
    setRolesDialogOpen(true);

    try {
      const [rolesRes, availableRes] = await Promise.all([
        fetch(`${API_BASE}/get_user_roles`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ user_id: userId }),
        }),
        fetch(`${API_BASE}/get_role_list`),
      ]);

      if (rolesRes.ok) {
        const data = await rolesRes.json();
        setUserRoles(data || []);
      } else {
        throw new Error('Failed to fetch user roles');
      }

      if (availableRes.ok) {
        const availableData = await availableRes.json();
        setAvailableRoles(availableData || []);
      } else {
        throw new Error('Failed to fetch available roles');
      }
    } catch (err) {
      const errorMsg = 'Failed to fetch roles';
      console.error(errorMsg, err);
      setSnackbar({ open: true, message: `${errorMsg}: ${err instanceof Error ? err.message : 'Unknown error'}` });
    } finally {
      setRolesLoading(false);
    }
  };

  const handleCloseRolesDialog = () => {
    setRolesDialogOpen(false);
    setUserRoles([]);
    setSelectedUserForRoles(null);
    setAvailableRoles([]);
    setSelectedRoleId('');
  };

  const handleAssignRole = async () => {
    if (!selectedUserForRoles || !selectedRoleId) {
      setSnackbar({ open: true, message: 'Please select a role' });
      return;
    }

    try {
      const res = await fetch(`${API_BASE}/assign_role_to_user`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          user_id: selectedUserForRoles,
          role_id: selectedRoleId,
        }),
      });

      if (res.ok) {
        setSelectedRoleId('');
        // Refresh user roles
        const rolesRes = await fetch(`${API_BASE}/get_user_roles`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ user_id: selectedUserForRoles }),
        });
        if (rolesRes.ok) {
          const data = await rolesRes.json();
          setUserRoles(data || []);
        }
      } else {
        throw new Error('API call failed');
      }
    } catch (err) {
      const errorMsg = 'Failed to assign role';
      console.error(errorMsg, err);
      setSnackbar({ open: true, message: `${errorMsg}: ${err instanceof Error ? err.message : 'Unknown error'}` });
    }
  };

  const handleRemoveRole = async (roleId: number) => {
    if (!selectedUserForRoles) return;

    try {
      const res = await fetch(`${API_BASE}/remove_role_from_user`, {
        method: 'DELETE',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          user_id: selectedUserForRoles,
          role_id: roleId,
        }),
      });

      if (res.ok) {
        // Refresh user roles
        const rolesRes = await fetch(`${API_BASE}/get_user_roles`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ user_id: selectedUserForRoles }),
        });
        if (rolesRes.ok) {
          const data = await rolesRes.json();
          setUserRoles(data || []);
        }
      } else {
        throw new Error('API call failed');
      }
    } catch (err) {
      const errorMsg = 'Failed to remove role';
      console.error(errorMsg, err);
      setSnackbar({ open: true, message: `${errorMsg}: ${err instanceof Error ? err.message : 'Unknown error'}` });
    }
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
                  <TableCell>{new Date(user.created_at).toLocaleDateString()}</TableCell>
                  <TableCell>
                    <IconButton size="small" onClick={() => handleOpenEditDialog(user)} color="primary">
                      <EditIcon />
                    </IconButton>
                    <IconButton size="small" onClick={() => handleOpenRolesDialog(user.id)} color="info">
                      <SecurityIcon />
                    </IconButton>
                    <IconButton size="small" onClick={() => handleDeleteUserClick(user.id)} color="error">
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

      {/* User dialog (Create/Edit) */}
      <Dialog
        open={userDialogOpen}
        onClose={handleCloseUserDialog}
        aria-labelledby="user-dialog-title"
        maxWidth="sm"
        fullWidth
      >
        <DialogTitle id="user-dialog-title">{isEditMode ? 'Edit User' : 'Create New User'}</DialogTitle>
        <DialogContent>
          <Stack spacing={2} sx={{ mt: 2 }}>
            {isEditMode && <TextField label="ID" value={editingUser?.id || ''} variant="outlined" fullWidth disabled />}
            <TextField
              label="Email"
              value={editingUser?.email || ''}
              onChange={(e) => handleUserFieldChange('email', e.target.value)}
              variant="outlined"
              fullWidth
              type="email"
            />
            <TextField
              label="Access Key"
              value={editingUser?.access_key || ''}
              onChange={(e) => handleUserFieldChange('access_key', e.target.value)}
              variant="outlined"
              fullWidth
            />
            {isEditMode && (
              <>
                <TextField
                  label="First Name"
                  value={editingUser?.first_name || ''}
                  onChange={(e) => handleUserFieldChange('first_name', e.target.value)}
                  variant="outlined"
                  fullWidth
                />
                <TextField
                  label="Last Name"
                  value={editingUser?.last_name || ''}
                  onChange={(e) => handleUserFieldChange('last_name', e.target.value)}
                  variant="outlined"
                  fullWidth
                />
                <TextField
                  label="TVA"
                  value={editingUser?.tva || ''}
                  onChange={(e) => handleUserFieldChange('tva', e.target.value)}
                  variant="outlined"
                  fullWidth
                />
                <TextField
                  label="Facturation Address"
                  value={editingUser?.facturation_address || ''}
                  onChange={(e) => handleUserFieldChange('facturation_address', e.target.value)}
                  variant="outlined"
                  fullWidth
                  multiline
                  rows={2}
                />
                <TextField
                  label="Facturation Account"
                  value={editingUser?.facturation_account || ''}
                  onChange={(e) => handleUserFieldChange('facturation_account', e.target.value)}
                  variant="outlined"
                  fullWidth
                />
              </>
            )}
          </Stack>
        </DialogContent>
        <DialogActions>
          <Button onClick={handleCloseUserDialog}>Cancel</Button>
          <Button onClick={handleSaveUser} variant="contained" color="primary">
            {isEditMode ? 'Update User' : 'Create User'}
          </Button>
        </DialogActions>
      </Dialog>

      {/* User roles dialog */}
      <Dialog
        open={rolesDialogOpen}
        onClose={handleCloseRolesDialog}
        aria-labelledby="roles-dialog-title"
        maxWidth="sm"
        fullWidth
      >
        <DialogTitle id="roles-dialog-title">Assigned Roles</DialogTitle>
        <DialogContent>
          {rolesLoading ? (
            <Box sx={{ display: 'flex', justifyContent: 'center', pt: 3 }}>
              <CircularProgress />
            </Box>
          ) : userRoles.length === 0 ? (
            <Typography sx={{ mt: 2 }}>No roles assigned to this user</Typography>
          ) : (
            <List sx={{ mt: 2 }}>
              {userRoles.map((role) => (
                <ListItem
                  key={role.id}
                  secondaryAction={
                    <IconButton
                      edge="end"
                      aria-label="delete"
                      onClick={() => handleRemoveRole(role.id)}
                      color="error"
                      size="small"
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
          <Stack spacing={2} sx={{ mt: 4 }}>
            <Typography variant="subtitle2">Assign a new role</Typography>
            <Stack direction="row" spacing={2}>
              <Select
                value={selectedRoleId}
                onChange={(e: SelectChangeEvent) => setSelectedRoleId(e.target.value)}
                displayEmpty
                fullWidth
                size="small"
              >
                <MenuItem value="">
                  <em>Select a role</em>
                </MenuItem>
                {availableRoles.map((role) => (
                  <MenuItem key={role.id} value={role.id}>
                    {role.name}
                  </MenuItem>
                ))}
              </Select>
              <Button variant="contained" color="success" onClick={handleAssignRole}>
                Add
              </Button>
            </Stack>
          </Stack>
        </DialogContent>
        <DialogActions>
          <Button onClick={handleCloseRolesDialog}>Close</Button>
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
