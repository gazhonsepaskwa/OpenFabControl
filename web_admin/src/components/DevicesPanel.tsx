import { SvgIconComponent } from '@mui/icons-material';
import CheckIcon from '@mui/icons-material/Check';
import CircleIcon from '@mui/icons-material/Circle';
import CloseIcon from '@mui/icons-material/Close';
import DevicesOtherIcon from '@mui/icons-material/DevicesOther';
import GridViewIcon from '@mui/icons-material/GridView';
import PrecisionManufacturingIcon from '@mui/icons-material/PrecisionManufacturing';
import ViewListIcon from '@mui/icons-material/ViewList';
import {
  Alert,
  Box,
  Button,
  Card,
  CardActions,
  CardContent,
  CardHeader,
  CircularProgress,
  Divider,
  FormControl,
  Grid,
  InputLabel,
  MenuItem,
  Select,
  SelectChangeEvent,
  Snackbar,
  Stack,
  ToggleButton,
  ToggleButtonGroup,
  Typography,
} from '@mui/material';
import React, { useEffect, useMemo, useState } from 'react';

const API_BASE = '/web-admin-api';

interface Device {
  approved: boolean;
  name: string;
  type: string;
  zone: string;
  uuid: string;
  price_booking_in_eur?: number;
  price_usage_in_eur?: number;
}

interface DeviceCardProps {
  device: Device;
  onApprove: (uuid: string) => void;
  onUnapprove: (uuid: string) => void;
  viewMode: ViewMode;
}

type ViewMode = 'grid' | 'stack';

const DEVICE_TYPE_ICONS: Record<string, SvgIconComponent> = {
  'fm-bv2': PrecisionManufacturingIcon,
  // "device-type-1": AnyIcon,
  // "device-type-2": AnyIcon,
};

function DeviceCard({ device, onApprove, onUnapprove, viewMode }: DeviceCardProps) {
  const isApproved = device.approved;
  const TypeIcon = DEVICE_TYPE_ICONS[device.type] || DevicesOtherIcon;

  return (
    <Card
      sx={{
        minWidth: 275,
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
          <Button size="small" color="error" startIcon={<CloseIcon />} onClick={() => onUnapprove(device.uuid)}>
            Unapprove
          </Button>
        ) : (
          <Button size="small" color="success" startIcon={<CheckIcon />} onClick={() => onApprove(device.uuid)}>
            Approve
          </Button>
        )}
      </CardActions>
    </Card>
  );
}

function DevicesPanel() {
  const [viewMode, setViewMode] = useState<ViewMode>('grid');
  const [notApprovedDevices, setNotApprovedDevices] = useState<Device[]>([]);
  const [approvedDevices, setApprovedDevices] = useState<Device[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [snackbar, setSnackbar] = useState({ open: false, message: '' });
  const [zoneFilter, setZoneFilter] = useState('All');

  const allZones = useMemo(() => {
    const zones = new Set<string>();
    [...notApprovedDevices, ...approvedDevices].forEach((device) => {
      if (device.zone) {
        zones.add(device.zone);
      }
    });
    return ['All', ...Array.from(zones).sort()];
  }, [notApprovedDevices, approvedDevices]);

  const filteredNotApprovedDevices = useMemo(() => {
    if (zoneFilter === 'All') return notApprovedDevices;
    return notApprovedDevices.filter((device) => device.zone === zoneFilter);
  }, [notApprovedDevices, zoneFilter]);

  const filteredApprovedDevices = useMemo(() => {
    if (zoneFilter === 'All') return approvedDevices;
    return approvedDevices.filter((device) => device.zone === zoneFilter);
  }, [approvedDevices, zoneFilter]);

  const fetchDevices = async (showLoading = true) => {
    if (showLoading) {
      setLoading(true);
    }
    setError(null);
    try {
      const [notApprovedRes, approvedRes] = await Promise.all([
        fetch(`${API_BASE}/get_machine_controler_list_to_approve`),
        fetch(`${API_BASE}/get_machine_controler_list_approved`),
      ]);

      if (!notApprovedRes.ok || !approvedRes.ok) {
        throw new Error('Failed to fetch devices');
      }

      const notApprovedData = await notApprovedRes.json();
      const approvedData = await approvedRes.json();

      setNotApprovedDevices(notApprovedData || []);
      setApprovedDevices(approvedData || []);
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Unknown error');
    } finally {
      if (showLoading) {
        setLoading(false);
      }
    }
  };

  useEffect(() => {
    fetchDevices();
  }, []);

  const handleViewModeChange = (_event: React.MouseEvent<HTMLElement>, newMode: ViewMode | null) => {
    if (newMode !== null) {
      setViewMode(newMode);
    }
  };

  const handleApprove = async (uuid: string) => {
    try {
      const res = await fetch(`${API_BASE}/approve_machine_controler`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ uuid }),
      });
      if (res.ok) {
        await fetchDevices(false);
      } else {
        throw new Error('API call failed');
      }
    } catch (err) {
      const errorMsg = 'Failed to approve device';
      console.error(errorMsg, err);
      setSnackbar({ open: true, message: `${errorMsg}: ${err instanceof Error ? err.message : 'Unknown error'}` });
    }
  };

  const handleUnapprove = async (uuid: string) => {
    try {
      const res = await fetch(`${API_BASE}/unapprove_machine_controler`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ uuid }),
      });
      if (res.ok) {
        await fetchDevices(false);
      } else {
        throw new Error('API call failed');
      }
    } catch (err) {
      const errorMsg = 'Failed to unapprove device';
      console.error(errorMsg, err);
      setSnackbar({ open: true, message: `${errorMsg}: ${err instanceof Error ? err.message : 'Unknown error'}` });
    }
  };

  const handleCloseSnackbar = () => {
    setSnackbar({ open: false, message: '' });
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
        sx={{
          display: 'flex',
          justifyContent: 'space-between',
          alignItems: 'center',
          mb: 3,
        }}
      >
        <Box sx={{ display: 'flex', alignItems: 'center', gap: 2 }}>
          <Typography variant="h4" component="h1">
            Devices
          </Typography>
          <FormControl size="small" sx={{ minWidth: 120 }}>
            <InputLabel id="zone-filter-label">Zone</InputLabel>
            <Select
              labelId="zone-filter-label"
              id="zone-filter"
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
        </Box>
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

      {/* Unapproved devices */}
      <Box>
        <Typography variant="h6" gutterBottom>
          Pending Approval ({filteredNotApprovedDevices.length})
        </Typography>
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
        <Typography variant="h6" gutterBottom>
          Approved Devices ({filteredApprovedDevices.length})
        </Typography>
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
                  viewMode={viewMode}
                />
              </Grid>
            ))}
          </Grid>
        )}
      </Box>

      {/* Snackbar for toast notifications */}
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
    </Box>
  );
}

export default DevicesPanel;
