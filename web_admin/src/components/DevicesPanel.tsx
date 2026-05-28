import { SvgIconComponent } from '@mui/icons-material';
import AddIcon from '@mui/icons-material/Add';
import CheckIcon from '@mui/icons-material/Check';
import CircleIcon from '@mui/icons-material/Circle';
import CloseIcon from '@mui/icons-material/Close';
import DevicesOtherIcon from '@mui/icons-material/DevicesOther';
import GridViewIcon from '@mui/icons-material/GridView';
import PrecisionManufacturingIcon from '@mui/icons-material/PrecisionManufacturing';
import ViewListIcon from '@mui/icons-material/ViewList';
import {
  Alert,
  Autocomplete,
  Badge,
  Box,
  Button,
  Card,
  CardActions,
  CardContent,
  CardHeader,
  CircularProgress,
  Dialog,
  DialogActions,
  DialogContent,
  DialogTitle,
  Divider,
  FormControl,
  Grid,
  InputLabel,
  MenuItem,
  Select,
  SelectChangeEvent,
  Snackbar,
  Stack,
  TextField,
  ToggleButton,
  ToggleButtonGroup,
  Typography,
} from '@mui/material';
import React, { useEffect, useMemo, useState } from 'react';
import { apiFetch, extractErrorMessage } from '../apiUtils';
import type { Resource } from '../types';

const API_BASE = '/web-admin-api';

type ViewMode = 'grid' | 'stack';

interface DeviceCardProps {
  device: Resource;
  onApprove: (uuid: string) => void;
  onUnapprove: (uuid: string) => void;
  onEdit: (device: Resource) => void;
  viewMode: ViewMode;
}

const DEVICE_TYPE_ICONS: Record<string, SvgIconComponent> = {
  'fm-bv2': PrecisionManufacturingIcon,
};

function DeviceCard({ device, onApprove, onUnapprove, onEdit, viewMode }: DeviceCardProps) {
  const isApproved = device.approved;
  const TypeIcon = DEVICE_TYPE_ICONS[device.type] || DevicesOtherIcon;

  return (
    <Card
      onClick={() => onEdit(device)}
      sx={{
        minWidth: 275,
        cursor: 'pointer',
        transition: 'transform 0.2s, box-shadow 0.2s',
        '&:hover': {
          transform: 'translateY(-4px)',
          boxShadow: 6,
        },
      }}
    >
      <CardHeader
        avatar={viewMode === 'grid' && <TypeIcon sx={{ color: 'text.secondary' }} />}
        title={device.name}
        slotProps={{ title: { variant: 'h6' } }}
        action={<CircleIcon sx={{ fontSize: 12, mt: 1, mr: 1 }} color={isApproved ? 'success' : 'error'} />}
      />
      <CardContent sx={{ pt: 0 }}>
        <Box sx={{ display: 'flex', gap: 2 }}>
          {viewMode === 'stack' && <TypeIcon sx={{ fontSize: 96, color: 'text.secondary' }} />}
          <Box>
            <Typography color="text.secondary" gutterBottom>
              Type: {device.type}
            </Typography>
            <Typography variant="body2">Zone: {device.zone || 'N/A'}</Typography>
            <Typography variant="body2">UUID: {device.uuid}</Typography>
            <Typography variant="body2">
              Booking: €{device.price_booking_in_eur?.toFixed(2) ?? '0.00'} | Usage: €
              {device.price_usage_in_eur?.toFixed(2) ?? '0.00'}
            </Typography>
          </Box>
        </Box>
      </CardContent>
      <CardActions sx={{ justifyContent: 'flex-end' }}>
        {isApproved ? (
          <Button
            size="small"
            color="error"
            startIcon={<CloseIcon />}
            onClick={(e) => {
              e.stopPropagation();
              onUnapprove(device.uuid);
            }}
          >
            Unapprove
          </Button>
        ) : (
          <Button
            size="small"
            color="success"
            startIcon={<CheckIcon />}
            onClick={(e) => {
              e.stopPropagation();
              onApprove(device.uuid);
            }}
          >
            Approve
          </Button>
        )}
      </CardActions>
    </Card>
  );
}

const BLANK_DEVICE: Resource = {
  uuid: '',
  name: '',
  type: '',
  zone: '',
  manual: '',
  price_booking_in_eur: 0,
  price_usage_in_eur: 0,
};

function DevicesPanel() {
  const [viewMode, setViewMode] = useState<ViewMode>('grid');
  const [notApprovedDevices, setNotApprovedDevices] = useState<Resource[]>([]);
  const [approvedDevices, setApprovedDevices] = useState<Resource[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [snackbar, setSnackbar] = useState({ open: false, message: '' });
  const [zoneFilter, setZoneFilter] = useState('All');
  const [typeFilter, setTypeFilter] = useState('All');

  const [deviceDialogOpen, setDeviceDialogOpen] = useState(false);
  const [isEditMode, setIsEditMode] = useState(false);
  const [editingDevice, setEditingDevice] = useState<Resource | null>(null);

  const allZones = useMemo(() => {
    const zones = new Set<string>();
    [...notApprovedDevices, ...approvedDevices].forEach((device) => {
      if (device.zone) zones.add(device.zone);
    });
    return ['All', ...Array.from(zones).sort()];
  }, [notApprovedDevices, approvedDevices]);

  const allTypes = useMemo(() => {
    const types = new Set<string>();
    [...notApprovedDevices, ...approvedDevices].forEach((device) => {
      if (device.type) types.add(device.type);
    });
    return Array.from(types).sort();
  }, [notApprovedDevices, approvedDevices]);

  const filteredNotApprovedDevices = useMemo(() => {
    return notApprovedDevices.filter(
      (device) =>
        (zoneFilter === 'All' || device.zone === zoneFilter) && (typeFilter === 'All' || device.type === typeFilter)
    );
  }, [notApprovedDevices, zoneFilter, typeFilter]);

  const filteredApprovedDevices = useMemo(() => {
    return approvedDevices.filter(
      (device) =>
        (zoneFilter === 'All' || device.zone === zoneFilter) && (typeFilter === 'All' || device.type === typeFilter)
    );
  }, [approvedDevices, zoneFilter, typeFilter]);

  const fetchDevices = async (showLoading = true) => {
    if (showLoading) setLoading(true);
    setError(null);
    try {
      const [notApprovedRes, approvedRes] = await Promise.all([
        apiFetch(`${API_BASE}/get_resource_list_to_approve`),
        apiFetch(`${API_BASE}/get_resource_list_approved`),
      ]);

      if (!notApprovedRes.ok || !approvedRes.ok) {
        throw new Error('Failed to fetch devices');
      }

      setNotApprovedDevices((await notApprovedRes.json()) || []);
      setApprovedDevices((await approvedRes.json()) || []);
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Unknown error');
    } finally {
      if (showLoading) setLoading(false);
    }
  };

  useEffect(() => {
    fetchDevices();
  }, []);

  const handleCloseSnackbar = () => setSnackbar({ open: false, message: '' });

  const handleOpenCreateDialog = () => {
    setIsEditMode(false);
    setEditingDevice({ ...BLANK_DEVICE });
    setDeviceDialogOpen(true);
  };

  const handleOpenEditDialog = (device: Resource) => {
    setIsEditMode(true);
    setEditingDevice({ ...device });
    setDeviceDialogOpen(true);
  };

  const handleCloseDeviceDialog = () => {
    setDeviceDialogOpen(false);
    setEditingDevice(null);
  };

  const handleDeviceFieldChange = (field: keyof Resource, value: string | number) => {
    if (editingDevice) {
      setEditingDevice({ ...editingDevice, [field]: value });
    }
  };

  const validateDeviceForm = (): string | null => {
    if (!editingDevice) return 'No device data';
    if (!isEditMode && !editingDevice.uuid.trim()) return 'UUID cannot be empty';
    if (!editingDevice.name.trim()) return 'Name cannot be empty';
    if (!editingDevice.type.trim()) return 'Type cannot be empty';
    return null;
  };

  const handleSaveDevice = async () => {
    if (!editingDevice) return;

    const validationError = validateDeviceForm();
    if (validationError) {
      setSnackbar({ open: true, message: validationError });
      return;
    }

    const endpoint = isEditMode ? '/edit_resource' : '/register_resource';
    const errorMsg = isEditMode ? 'Failed to update device' : 'Failed to create device';

    let payload: Record<string, string | number>;
    if (isEditMode) {
      payload = { uuid: editingDevice.uuid };
      if (editingDevice.name.trim()) payload.name = editingDevice.name.trim();
      if (editingDevice.type.trim()) payload.type = editingDevice.type.trim();
      if (editingDevice.zone?.trim()) payload.zone = editingDevice.zone.trim();
      if (editingDevice.manual?.trim()) payload.manual = editingDevice.manual.trim();
      if (editingDevice.price_booking_in_eur != null)
        payload.price_booking_in_eur = String(editingDevice.price_booking_in_eur);
      if (editingDevice.price_usage_in_eur != null)
        payload.price_usage_in_eur = String(editingDevice.price_usage_in_eur);
    } else {
      payload = {
        uuid: editingDevice.uuid.trim(),
        name: editingDevice.name.trim(),
        type: editingDevice.type.trim(),
        zone: editingDevice.zone?.trim() ?? '',
        manual: editingDevice.manual?.trim() ?? '',
        price_booking_in_eur: editingDevice.price_booking_in_eur ?? 0,
        price_usage_in_eur: editingDevice.price_usage_in_eur ?? 0,
      };
    }

    try {
      const res = await apiFetch(`${API_BASE}${endpoint}`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload),
      });

      if (res.ok) {
        setDeviceDialogOpen(false);
        setEditingDevice(null);
        await fetchDevices(false);
      } else {
        const msg = await extractErrorMessage(res, 'API call failed');
        throw new Error(msg);
      }
    } catch (err) {
      console.error(errorMsg, err);
      setSnackbar({ open: true, message: `${errorMsg}: ${err instanceof Error ? err.message : 'Unknown error'}` });
    }
  };

  const handleDeleteDevice = async () => {
    if (!editingDevice) return;
    try {
      const res = await apiFetch(`${API_BASE}/delete_resource`, {
        method: 'DELETE',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ uuid: editingDevice.uuid }),
      });
      if (res.ok) {
        setDeviceDialogOpen(false);
        setEditingDevice(null);
        await fetchDevices(false);
      } else {
        const msg = await extractErrorMessage(res, 'API call failed');
        throw new Error(msg);
      }
    } catch (err) {
      setSnackbar({
        open: true,
        message: `Failed to delete device: ${err instanceof Error ? err.message : 'Unknown error'}`,
      });
    }
  };

  const handleApprove = async (uuid: string) => {
    try {
      const res = await apiFetch(`${API_BASE}/approve_resource`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ uuid }),
      });
      if (res.ok) {
        await fetchDevices(false);
      } else {
        const msg = await extractErrorMessage(res, 'API call failed');
        throw new Error(msg);
      }
    } catch (err) {
      const errorMsg = 'Failed to approve device';
      console.error(errorMsg, err);
      setSnackbar({ open: true, message: `${errorMsg}: ${err instanceof Error ? err.message : 'Unknown error'}` });
    }
  };

  const handleUnapprove = async (uuid: string) => {
    try {
      const res = await apiFetch(`${API_BASE}/unapprove_resource`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ uuid }),
      });
      if (res.ok) {
        await fetchDevices(false);
      } else {
        const msg = await extractErrorMessage(res, 'API call failed');
        throw new Error(msg);
      }
    } catch (err) {
      const errorMsg = 'Failed to unapprove device';
      console.error(errorMsg, err);
      setSnackbar({ open: true, message: `${errorMsg}: ${err instanceof Error ? err.message : 'Unknown error'}` });
    }
  };

  const handleViewModeChange = (_event: React.MouseEvent<HTMLElement>, newMode: ViewMode | null) => {
    if (newMode !== null) setViewMode(newMode);
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
      {/* Header */}
      <Box
        sx={{ display: 'flex', flexWrap: 'wrap', justifyContent: 'space-between', alignItems: 'center', gap: 1, mb: 3 }}
      >
        <Box sx={{ display: 'flex', flexWrap: 'wrap', alignItems: 'center', gap: 1 }}>
          <Typography variant="h4" component="h1" gutterBottom sx={{ mb: 0 }}>
            Devices
          </Typography>
          <Box sx={{ display: 'flex', gap: 1 }}>
            <FormControl size="small" sx={{ minWidth: 100 }}>
              <InputLabel id="zone-filter-label">Zone</InputLabel>
              <Select
                labelId="zone-filter-label"
                value={zoneFilter}
                label="Zone"
                onChange={(e: SelectChangeEvent) => setZoneFilter(e.target.value)}
              >
                {allZones.map((zone) => (
                  <MenuItem key={zone} value={zone}>
                    {zone}
                  </MenuItem>
                ))}
              </Select>
            </FormControl>
            <FormControl size="small" sx={{ minWidth: 100 }}>
              <InputLabel id="type-filter-label">Type</InputLabel>
              <Select
                labelId="type-filter-label"
                value={typeFilter}
                label="Type"
                onChange={(e: SelectChangeEvent) => setTypeFilter(e.target.value)}
              >
                <MenuItem value="All">All</MenuItem>
                {allTypes.map((type) => (
                  <MenuItem key={type} value={type}>
                    {type}
                  </MenuItem>
                ))}
              </Select>
            </FormControl>
          </Box>
        </Box>
        <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
          {/* Create Device buttons hidden — re-enable when needed
          <Button
            variant="contained"
            startIcon={<AddIcon />}
            onClick={handleOpenCreateDialog}
            sx={{ display: { xs: 'none', sm: 'inline-flex' } }}
          >
            Create Device
          </Button>
          <Button
            variant="contained"
            onClick={handleOpenCreateDialog}
            sx={{ display: { xs: 'inline-flex', sm: 'none' }, minWidth: 0, px: 1 }}
            aria-label="Create Device"
          >
            <AddIcon />
          </Button>
          */}
          <ToggleButtonGroup
            value={viewMode}
            exclusive
            onChange={handleViewModeChange}
            aria-label="view mode"
            size="small"
          >
            <ToggleButton value="stack" aria-label="stack view">
              <ViewListIcon />
            </ToggleButton>
            <ToggleButton value="grid" aria-label="grid view">
              <GridViewIcon />
            </ToggleButton>
          </ToggleButtonGroup>
        </Box>
      </Box>

      {/* Pending approval */}
      <Box>
        <Badge badgeContent={filteredNotApprovedDevices.length} color="warning" max={9999} sx={{ mb: 2 }}>
          <Typography variant="h6" sx={{ pr: 2 }}>
            Pending Approval
          </Typography>
        </Badge>
        {filteredNotApprovedDevices.length === 0 ? (
          <Typography color="text.secondary">No devices pending approval</Typography>
        ) : viewMode === 'stack' ? (
          <Stack spacing={2}>
            {filteredNotApprovedDevices.map((device) => (
              <DeviceCard
                key={device.uuid}
                device={device}
                onApprove={handleApprove}
                onUnapprove={handleUnapprove}
                onEdit={handleOpenEditDialog}
                viewMode={viewMode}
              />
            ))}
          </Stack>
        ) : (
          <Grid container spacing={2}>
            {filteredNotApprovedDevices.map((device) => (
              <Grid size={{ xs: 12, sm: 6, md: 4, lg: 3 }} key={device.uuid}>
                <DeviceCard
                  device={device}
                  onApprove={handleApprove}
                  onUnapprove={handleUnapprove}
                  onEdit={handleOpenEditDialog}
                  viewMode={viewMode}
                />
              </Grid>
            ))}
          </Grid>
        )}
      </Box>
      <Divider sx={{ my: 3 }} />

      {/* Approved devices */}
      <Box>
        <Badge badgeContent={filteredApprovedDevices.length} color="success" max={9999} sx={{ mb: 2 }}>
          <Typography variant="h6" sx={{ pr: 2 }}>
            Approved Devices
          </Typography>
        </Badge>
        {filteredApprovedDevices.length === 0 ? (
          <Typography color="text.secondary">No approved devices</Typography>
        ) : viewMode === 'stack' ? (
          <Stack spacing={2}>
            {filteredApprovedDevices.map((device) => (
              <DeviceCard
                key={device.uuid}
                device={device}
                onApprove={handleApprove}
                onUnapprove={handleUnapprove}
                onEdit={handleOpenEditDialog}
                viewMode={viewMode}
              />
            ))}
          </Stack>
        ) : (
          <Grid container spacing={2}>
            {filteredApprovedDevices.map((device) => (
              <Grid size={{ xs: 12, sm: 6, md: 4, lg: 3 }} key={device.uuid}>
                <DeviceCard
                  device={device}
                  onApprove={handleApprove}
                  onUnapprove={handleUnapprove}
                  onEdit={handleOpenEditDialog}
                  viewMode={viewMode}
                />
              </Grid>
            ))}
          </Grid>
        )}
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

      {/* Create / Edit dialog */}
      <Dialog
        open={deviceDialogOpen}
        onClose={handleCloseDeviceDialog}
        aria-labelledby="device-dialog-title"
        maxWidth="sm"
        fullWidth
      >
        <DialogTitle id="device-dialog-title">{isEditMode ? 'Edit Device' : 'Create New Device'}</DialogTitle>
        <DialogContent>
          <Stack spacing={2} sx={{ mt: 2 }}>
            <TextField
              label="UUID"
              value={editingDevice?.uuid || ''}
              onChange={(e) => handleDeviceFieldChange('uuid', e.target.value)}
              variant="outlined"
              fullWidth
              disabled={isEditMode}
            />
            <TextField
              label="Name"
              value={editingDevice?.name || ''}
              onChange={(e) => handleDeviceFieldChange('name', e.target.value)}
              variant="outlined"
              fullWidth
            />
            <Autocomplete
              freeSolo
              options={allTypes}
              value={editingDevice?.type || ''}
              onInputChange={(_e, value) => handleDeviceFieldChange('type', value)}
              renderInput={(params) => <TextField {...params} label="Type" variant="outlined" />}
            />
            <Autocomplete
              freeSolo
              options={allZones.filter((z) => z !== 'All')}
              value={editingDevice?.zone || ''}
              onInputChange={(_e, value) => handleDeviceFieldChange('zone', value)}
              renderInput={(params) => <TextField {...params} label="Zone" variant="outlined" />}
            />
            {/* <TextField
              label="Manual URL"
              value={editingDevice?.manual || ''}
              onChange={(e) => handleDeviceFieldChange('manual', e.target.value)}
              variant="outlined"
              fullWidth
            /> */}
            <TextField
              label="Booking Price (€)"
              value={editingDevice?.price_booking_in_eur ?? ''}
              onChange={(e) => handleDeviceFieldChange('price_booking_in_eur', parseFloat(e.target.value) || 0)}
              variant="outlined"
              type="number"
              inputProps={{ min: 0, step: 0.01 }}
              fullWidth
            />
            <TextField
              label="Usage Price (€)"
              value={editingDevice?.price_usage_in_eur ?? ''}
              onChange={(e) => handleDeviceFieldChange('price_usage_in_eur', parseFloat(e.target.value) || 0)}
              variant="outlined"
              type="number"
              inputProps={{ min: 0, step: 0.01 }}
              fullWidth
            />
          </Stack>
        </DialogContent>
        <DialogActions>
          {isEditMode && (
            <Button onClick={handleDeleteDevice} color="error">
              Delete
            </Button>
          )}
          <Box sx={{ flex: 1 }} />
          <Button onClick={handleCloseDeviceDialog}>Cancel</Button>
          <Button onClick={handleSaveDevice} variant="contained" color="primary">
            {isEditMode ? 'Update Device' : 'Create Device'}
          </Button>
        </DialogActions>
      </Dialog>
    </Box>
  );
}

export default DevicesPanel;
