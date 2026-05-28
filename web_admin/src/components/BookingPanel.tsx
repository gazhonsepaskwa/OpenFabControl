import { EventClickArg, EventInput } from '@fullcalendar/core';
import dayGridPlugin from '@fullcalendar/daygrid';
import interactionPlugin from '@fullcalendar/interaction';
import FullCalendar from '@fullcalendar/react';
import AddIcon from '@mui/icons-material/Add';
import {
  Alert,
  Autocomplete,
  Box,
  Button,
  Dialog,
  DialogActions,
  DialogContent,
  DialogTitle,
  FormControl,
  GlobalStyles,
  InputLabel,
  MenuItem,
  Select,
  Snackbar,
  Stack,
  TextField,
  Typography,
} from '@mui/material';
import React, { useCallback, useEffect, useMemo, useRef, useState } from 'react';
import { apiFetch, extractErrorMessage } from '../apiUtils';
import { getCurrentUserId, isAdmin } from '../common';
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

const USER_PAGE_SIZE = 20;

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
  const [userSearch, setUserSearch] = useState('');
  const [userPageSize, setUserPageSize] = useState(USER_PAGE_SIZE);

  useEffect(() => {
    if (dialogOpen) {
      setUserSearch('');
      setUserPageSize(USER_PAGE_SIZE);
    }
  }, [dialogOpen]);

  const filteredUsers = useMemo(() => {
    const q = userSearch.toLowerCase().trim();
    const matched = q
      ? users.filter(
          (u) => `${u.first_name} ${u.last_name}`.toLowerCase().includes(q) || u.email.toLowerCase().includes(q)
        )
      : users;
    const page = matched.slice(0, userPageSize);
    if (form.user_id) {
      const selected = users.find((u) => String(u.id) === form.user_id);
      if (selected && !page.some((u) => u.id === selected.id)) {
        return [selected, ...page];
      }
    }
    return page;
  }, [users, userSearch, userPageSize, form.user_id]);

  /* Fetch resources once */
  useEffect(() => {
    const endpoint = userIsAdmin ? '/web-admin-api/get_resource_list_approved' : '/web-user-api/get_resource_list';
    apiFetch(endpoint)
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
      if (!userIsAdmin) return resourceLabel;
      const user = users.find((u) => u.id === session.user_id);
      const userLabel = user ? `${user.first_name} ${user.last_name}` : `#${session.user_id}`;
      return `${resourceLabel} · ${userLabel}`;
    },
    [userIsAdmin, resources, users]
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
        .then(async (r) => {
          if (!r.ok) throw new Error(await extractErrorMessage(r, 'Failed to fetch bookings'));
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

  /* Today's date as YYYY-MM-DD (recomputed once per render, stable enough) */
  const todayStr = new Date().toISOString().slice(0, 10);

  /* Open "New Booking" dialog, optionally pre-filled with a specific date */
  const openNewBookingDialog = useCallback(
    (dateStr?: string) => {
      if (dateStr && !userIsAdmin && dateStr < todayStr) return;
      const d = dateStr ?? new Date().toISOString().slice(0, 10);
      const now = new Date();
      const pad = (n: number) => String(n).padStart(2, '0');
      const startHour = (now.getHours() + 1) % 24;
      const endHour = (now.getHours() + 2) % 24;
      const currentUserId = getCurrentUserId();
      setEditingSessionId(null);
      setForm({
        resource_uuid: resources[0]?.uuid ?? '',
        started_at: `${d}T${pad(startHour)}:00`,
        ended_at: `${d}T${pad(endHour)}:00`,
        user_id: currentUserId !== null ? String(currentUserId) : '',
      });
      setDialogOpen(true);
    },
    [resources, userIsAdmin, todayStr]
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

  const handleStartDateChange = (newStart: string) => {
    const endDate = newStart ? new Date(new Date(newStart).getTime() + 60 * 60 * 1000) : null;
    const pad = (n: number) => String(n).padStart(2, '0');
    const newEnd = endDate
      ? `${endDate.getFullYear()}-${pad(endDate.getMonth() + 1)}-${pad(endDate.getDate())}T${pad(endDate.getHours())}:${pad(endDate.getMinutes())}`
      : '';
    setForm((f) => ({ ...f, started_at: newStart, ended_at: newEnd }));
  };

  const handleDelete = async () => {
    if (editingSessionId === null) return;
    setSubmitting(true);
    const endpoint = userIsAdmin ? '/web-admin-api/delete_session' : '/web-user-api/delete_session';
    try {
      const res = await apiFetch(endpoint, {
        method: 'DELETE',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ session_id: editingSessionId }),
      });
      const data = await res.json();
      if (!res.ok) throw new Error(data.error ?? 'Failed to delete booking');
      setDialogOpen(false);
      setSnackbar({ open: true, message: 'Booking deleted successfully!', severity: 'success' });
      calendarRef.current?.getApi().refetchEvents();
    } catch (err) {
      setSnackbar({
        open: true,
        message: err instanceof Error ? err.message : 'Failed to delete booking',
        severity: 'error',
      });
    } finally {
      setSubmitting(false);
    }
  };

  const closeSnackbar = () => setSnackbar((s) => ({ ...s, open: false }));

  /* True when a non-admin is editing a booking whose start is in the past */
  const isPastBooking =
    !userIsAdmin && editingSessionId !== null && !!form.started_at && new Date(form.started_at) < new Date();

  return (
    <Box sx={{ p: 3, display: 'flex', flexDirection: 'column', height: '100%' }}>
      <Box sx={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', mb: 2 }}>
        <Typography variant="h4" component="h1">
          Booking
        </Typography>
        <Button
          variant="contained"
          startIcon={<AddIcon />}
          onClick={() => openNewBookingDialog()}
          sx={{ display: { xs: 'none', sm: 'inline-flex' } }}
        >
          Book device
        </Button>
        <Button
          variant="contained"
          onClick={() => openNewBookingDialog()}
          sx={{ display: { xs: 'inline-flex', sm: 'none' }, minWidth: 0, px: 1 }}
          aria-label="Book device"
        >
          <AddIcon />
        </Button>
      </Box>

      {!userIsAdmin && (
        <GlobalStyles
          styles={{
            '.fc-day-past': { backgroundColor: 'rgba(0,0,0,0.04)', cursor: 'default' },
            '.fc-day-past .fc-daygrid-day-number': { color: 'rgba(0,0,0,0.38)', pointerEvents: 'none' },
            '.fc-day-past .fc-daygrid-event': { opacity: 0.45 },
          }}
        />
      )}
      <Box sx={{ flex: 1, minHeight: 0 }}>
        <FullCalendar
          ref={calendarRef}
          plugins={[dayGridPlugin, interactionPlugin]}
          initialView="dayGridMonth"
          events={fetchEvents}
          dateClick={(info) => openNewBookingDialog(info.dateStr)}
          eventClick={handleEventClick}
          eventDidMount={(info) => {
            info.el.title = info.event.title;
          }}
          headerToolbar={{ left: 'prev,next today', center: 'title', right: 'dayGridDay,dayGridMonth' }}
          height="100%"
          buttonText={{ today: 'Today', dayGridDay: 'Day', dayGridMonth: 'Month' }}
        />
      </Box>

      {/* ── New / Edit Booking dialog ── */}
      <Dialog open={dialogOpen} onClose={() => setDialogOpen(false)} maxWidth="sm" fullWidth>
        <DialogTitle>{editingSessionId !== null ? 'Edit Booking' : 'New Booking'}</DialogTitle>
        <DialogContent>
          <Stack spacing={2} sx={{ mt: 1 }}>
            {userIsAdmin && (
              <Autocomplete
                options={filteredUsers}
                getOptionLabel={(u) => `${u.first_name} ${u.last_name} — ${u.email}`}
                filterOptions={(x) => x}
                value={users.find((u) => String(u.id) === form.user_id) ?? null}
                onChange={(_, u) => setForm((f) => ({ ...f, user_id: u ? String(u.id) : '' }))}
                onInputChange={(_, val, reason) => {
                  if (reason === 'input') {
                    setUserSearch(val);
                    setUserPageSize(USER_PAGE_SIZE);
                  }
                }}
                isOptionEqualToValue={(option, value) => option.id === value.id}
                ListboxProps={{
                  onScroll: (e: React.UIEvent<HTMLUListElement>) => {
                    const el = e.currentTarget;
                    if (el.scrollHeight - el.scrollTop - el.clientHeight < 50) {
                      setUserPageSize((p: number) => p + USER_PAGE_SIZE);
                    }
                  },
                }}
                renderOption={(props, u) => (
                  <li {...props} key={u.id}>
                    {u.first_name} {u.last_name} — {u.email}
                  </li>
                )}
                renderInput={(params) => <TextField {...params} label="User" required />}
              />
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
              onChange={(e) => handleStartDateChange(e.target.value)}
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
          {editingSessionId !== null && (
            <Button onClick={handleDelete} color="error" disabled={submitting}>
              Delete
            </Button>
          )}
          <Box sx={{ flex: 1 }} />
          <Button onClick={() => setDialogOpen(false)}>Cancel</Button>
          <Button onClick={handleSubmit} variant="contained" disabled={submitting || isPastBooking}>
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
