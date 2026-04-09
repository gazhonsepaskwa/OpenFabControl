import { EventClickArg, EventInput } from '@fullcalendar/core';
import dayGridPlugin from '@fullcalendar/daygrid';
import interactionPlugin, { DateClickArg } from '@fullcalendar/interaction';
import FullCalendar from '@fullcalendar/react';
import {
  Alert,
  Box,
  Button,
  Dialog,
  DialogActions,
  DialogContent,
  DialogTitle,
  FormControl,
  InputLabel,
  MenuItem,
  Select,
  Snackbar,
  Stack,
  TextField,
  Typography,
} from '@mui/material';
import { useCallback, useEffect, useRef, useState } from 'react';
import { apiFetch, getCurrentUserId, isAdmin } from '../common';
import type { Resource, Session, User } from '../types';

interface BookingForm {
  resource_uuid: string;
  started_at: string;
  ended_at: string;
  user_id: string;
}

interface SnackbarState {
  open: boolean;
  message: string;
  severity: 'success' | 'error';
}

function statusColor(status: string): string {
  switch (status) {
    case 'planned':
      return '#1976d2';
    case 'progress':
      return '#ed6c02';
    case 'done':
      return '#388e3c';
    default:
      return '#757575';
  }
}

export default function BookingPanel() {
  const userIsAdmin = isAdmin();

  const [resources, setResources] = useState<Resource[]>([]);
  const [users, setUsers] = useState<User[]>([]);
  const [dialogOpen, setDialogOpen] = useState(false);
  const [editingSessionId, setEditingSessionId] = useState<number | null>(null);
  const [form, setForm] = useState<BookingForm>({ resource_uuid: '', started_at: '', ended_at: '', user_id: '' });
  const [submitting, setSubmitting] = useState(false);
  const [snackbar, setSnackbar] = useState<SnackbarState>({ open: false, message: '', severity: 'success' });
  const calendarRef = useRef<FullCalendar>(null);

  /* Fetch approved resources once (admin only – used for dropdown + event titles) */
  useEffect(() => {
    if (!userIsAdmin) return;
    apiFetch('/web-admin-api/get_resource_list_approved')
      .then((r) => r.json())
      .then((data: Resource[]) => setResources(Array.isArray(data) ? data : []))
      .catch(console.error);
  }, [userIsAdmin]);

  /* Fetch all users once (admin only – used for user select) */
  useEffect(() => {
    if (!userIsAdmin) return;
    apiFetch('/web-admin-api/get_user_list')
      .then((r) => r.json())
      .then((data: User[]) => setUsers(Array.isArray(data) ? data : []))
      .catch(console.error);
  }, [userIsAdmin]);

  /* Build a human-readable event title from a session */
  const getTitle = useCallback(
    (session: Session): string => {
      const resource = resources.find((r) => r.uuid === session.resource_uuid);
      const resourceLabel = resource?.name ?? `${session.resource_uuid.slice(0, 8)}…`;
      return userIsAdmin ? `${resourceLabel} · #${session.user_id}` : resourceLabel;
    },
    [userIsAdmin, resources]
  );

  /* FullCalendar event-source function – called on every view/navigation change */
  const fetchEvents = useCallback(
    (
      fetchInfo: { startStr: string; endStr: string },
      successCallback: (events: EventInput[]) => void,
      failureCallback: (error: Error) => void
    ) => {
      const endpoint = userIsAdmin ? '/web-admin-api/fetch_booking' : '/web-user-api/fetch_my_booking';
      apiFetch(endpoint, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ started_at: fetchInfo.startStr, ended_at: fetchInfo.endStr }),
      })
        .then((r) => {
          if (!r.ok) throw new Error('Failed to fetch bookings');
          return r.json();
        })
        .then((data: { sessions: Session[] }) => {
          successCallback(
            (data.sessions ?? []).map((s) => ({
              id: String(s.id),
              title: getTitle(s),
              start: s.started_at,
              end: s.ended_at,
              backgroundColor: statusColor(s.status),
              borderColor: statusColor(s.status),
              extendedProps: { session: s },
            }))
          );
        })
        .catch(failureCallback);
    },
    [userIsAdmin, getTitle]
  );

  /* Open "New Booking" dialog pre-filled with the clicked date */
  const handleDateClick = useCallback(
    (info: DateClickArg) => {
      const d = info.dateStr; // YYYY-MM-DD
      const currentUserId = getCurrentUserId();
      setEditingSessionId(null);
      setForm({
        resource_uuid: resources[0]?.uuid ?? '',
        started_at: `${d}T09:00`,
        ended_at: `${d}T10:00`,
        user_id: currentUserId !== null ? String(currentUserId) : '',
      });
      setDialogOpen(true);
    },
    [resources]
  );

  /* Open dialog pre-filled with an existing session's data for editing */
  const handleEventClick = useCallback((info: EventClickArg) => {
    const session: Session = info.event.extendedProps.session;
    if (!session) return;
    const toLocal = (iso: string) => {
      // Convert ISO string to "YYYY-MM-DDTHH:mm" for datetime-local input
      const d = new Date(iso);
      const pad = (n: number) => String(n).padStart(2, '0');
      return `${d.getFullYear()}-${pad(d.getMonth() + 1)}-${pad(d.getDate())}T${pad(d.getHours())}:${pad(d.getMinutes())}`;
    };
    setEditingSessionId(session.id);
    setForm({
      resource_uuid: session.resource_uuid,
      started_at: toLocal(session.started_at),
      ended_at: toLocal(session.ended_at),
      user_id: String(session.user_id),
    });
    setDialogOpen(true);
  }, []);

  const handleSubmit = async () => {
    if (!form.resource_uuid || !form.started_at || !form.ended_at) {
      setSnackbar({ open: true, message: 'All fields are required.', severity: 'error' });
      return;
    }
    if (userIsAdmin && !form.user_id) {
      setSnackbar({ open: true, message: 'User is required.', severity: 'error' });
      return;
    }

    setSubmitting(true);

    const isEditing = editingSessionId !== null;
    const endpoint = isEditing
      ? userIsAdmin
        ? '/web-admin-api/update_session'
        : '/web-user-api/update_session'
      : userIsAdmin
        ? '/web-admin-api/create_session'
        : '/web-user-api/create_session';

    const payload: Record<string, unknown> = {
      ...(isEditing && { id: editingSessionId }),
      resource_uuid: form.resource_uuid,
      started_at: new Date(form.started_at).toISOString(),
      ended_at: new Date(form.ended_at).toISOString(),
      ...(userIsAdmin && { user_id: parseInt(form.user_id, 10) }),
    };

    try {
      const res = await apiFetch(endpoint, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload),
      });
      const data = await res.json();
      if (!res.ok) throw new Error(data.error ?? (isEditing ? 'Failed to update booking' : 'Failed to create booking'));
      setDialogOpen(false);
      setSnackbar({
        open: true,
        message: isEditing ? 'Booking updated successfully!' : 'Booking created successfully!',
        severity: 'success',
      });
      calendarRef.current?.getApi().refetchEvents();
    } catch (err) {
      setSnackbar({
        open: true,
        message:
          err instanceof Error ? err.message : isEditing ? 'Failed to update booking' : 'Failed to create booking',
        severity: 'error',
      });
    } finally {
      setSubmitting(false);
    }
  };

  const closeSnackbar = () => setSnackbar((s) => ({ ...s, open: false }));

  return (
    <Box sx={{ p: 3 }}>
      <Typography variant="h5" gutterBottom>
        Booking
      </Typography>

      <FullCalendar
        ref={calendarRef}
        plugins={[dayGridPlugin, interactionPlugin]}
        initialView="dayGridMonth"
        events={fetchEvents}
        dateClick={handleDateClick}
        eventClick={handleEventClick}
        headerToolbar={{ left: 'prev,next today', center: 'title', right: '' }}
        height="auto"
        buttonText={{ today: 'Today' }}
      />

      {/* ── New / Edit Booking dialog ── */}
      <Dialog open={dialogOpen} onClose={() => setDialogOpen(false)} maxWidth="sm" fullWidth>
        <DialogTitle>{editingSessionId !== null ? 'Edit Booking' : 'New Booking'}</DialogTitle>
        <DialogContent>
          <Stack spacing={2} sx={{ mt: 1 }}>
            {userIsAdmin && (
              <FormControl fullWidth required>
                <InputLabel>User</InputLabel>
                <Select
                  label="User"
                  value={form.user_id}
                  onChange={(e) => setForm((f) => ({ ...f, user_id: e.target.value }))}
                >
                  {users.map((u) => (
                    <MenuItem key={u.id} value={String(u.id)}>
                      {u.first_name} {u.last_name} — {u.email}
                    </MenuItem>
                  ))}
                </Select>
              </FormControl>
            )}

            {resources.length > 0 ? (
              <FormControl fullWidth required>
                <InputLabel>Machine</InputLabel>
                <Select
                  label="Machine"
                  value={form.resource_uuid}
                  onChange={(e) => setForm((f) => ({ ...f, resource_uuid: e.target.value }))}
                >
                  {resources.map((r) => (
                    <MenuItem key={r.uuid} value={r.uuid}>
                      {r.name} — {r.zone}
                    </MenuItem>
                  ))}
                </Select>
              </FormControl>
            ) : (
              <TextField
                label="Machine UUID"
                value={form.resource_uuid}
                onChange={(e) => setForm((f) => ({ ...f, resource_uuid: e.target.value }))}
                fullWidth
                required
              />
            )}

            <TextField
              label="Start"
              type="datetime-local"
              value={form.started_at}
              onChange={(e) => setForm((f) => ({ ...f, started_at: e.target.value }))}
              fullWidth
              slotProps={{ inputLabel: { shrink: true } }}
            />
            <TextField
              label="End"
              type="datetime-local"
              value={form.ended_at}
              onChange={(e) => setForm((f) => ({ ...f, ended_at: e.target.value }))}
              fullWidth
              slotProps={{ inputLabel: { shrink: true } }}
            />
          </Stack>
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setDialogOpen(false)}>Cancel</Button>
          <Button onClick={handleSubmit} variant="contained" disabled={submitting}>
            {submitting
              ? editingSessionId !== null
                ? 'Saving…'
                : 'Creating…'
              : editingSessionId !== null
                ? 'Save'
                : 'Create'}
          </Button>
        </DialogActions>
      </Dialog>

      {/* ── Snackbar ── */}
      <Snackbar
        open={snackbar.open}
        autoHideDuration={5000}
        onClose={closeSnackbar}
        anchorOrigin={{ vertical: 'bottom', horizontal: 'center' }}
      >
        <Alert severity={snackbar.severity} onClose={closeSnackbar} sx={{ width: '100%' }}>
          {snackbar.message}
        </Alert>
      </Snackbar>
    </Box>
  );
}
