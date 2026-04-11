import AdminPanelSettingsIcon from '@mui/icons-material/AdminPanelSettings';
import Brightness4Icon from '@mui/icons-material/Brightness4';
import Brightness7Icon from '@mui/icons-material/Brightness7';
import CalendarMonthIcon from '@mui/icons-material/CalendarMonth';
import DevicesIcon from '@mui/icons-material/Devices';
import LogoutIcon from '@mui/icons-material/Logout';
import PeopleIcon from '@mui/icons-material/People';
import SettingsIcon from '@mui/icons-material/Settings';
import {
  AppBar,
  BottomNavigation,
  BottomNavigationAction,
  Box,
  CssBaseline,
  IconButton,
  Paper,
  Tab,
  Tabs,
  Toolbar,
  Typography,
  useMediaQuery,
} from '@mui/material';
import { ThemeProvider, createTheme } from '@mui/material/styles';
import React, { useEffect, useMemo, useState } from 'react';
import { ADMIN_ROLE_ID, isAdmin } from './common';
import BookingPanel from './components/BookingPanel';
import ConfirmEmailPage from './components/ConfirmEmailPage';
import DevicesPanel from './components/DevicesPanel';
import LoginPage from './components/LoginPage';
import RolesPanel from './components/RolesPanel';
import SettingsPanel from './components/SettingsPanel';
import SubscriptionsPanel from './components/SubscriptionsPanel';
import UsersPanel from './components/UsersPanel';

const NAV_ITEMS = [
  { label: 'Users', index: 0, icon: PeopleIcon },
  { label: 'Roles', index: 1, icon: AdminPanelSettingsIcon },
  // { label: 'Subscriptions', index: 2, icon: SubscriptionsIcon },
  { label: 'Devices', index: 3, icon: DevicesIcon },
  { label: 'Booking', index: 4, icon: CalendarMonthIcon },
  { label: 'Settings', index: 5, icon: SettingsIcon },
];

const DEFAULT_ACCESSIBLE_TABS = [4, 5];

const TAB_ACCESS_BY_ROLE: Record<number, number[]> = {
  [ADMIN_ROLE_ID]: [0, 1, 2, 3, 4, 5],
};

type ThemeMode = 'light' | 'dark';

function getAccessibleTabs(roles: { id: number }[]): number[] {
  const tabs = new Set<number>(DEFAULT_ACCESSIBLE_TABS);
  for (const role of roles) {
    TAB_ACCESS_BY_ROLE[role.id]?.forEach((t) => tabs.add(t));
  }
  return Array.from(tabs).sort((a, b) => a - b);
}

function getConfirmEmailCode(): string | null {
  if (window.location.pathname !== '/confirm-email') return null;
  return new URLSearchParams(window.location.search).get('code');
}

function App() {
  const confirmEmailCode = getConfirmEmailCode();
  const [isLoggedIn, setIsLoggedIn] = useState<boolean>(() => !!sessionStorage.getItem('token'));
  const [tabValue, setTabValue] = useState(4);
  const [accessibleTabs, setAccessibleTabs] = useState<number[]>(() => {
    const userStr = sessionStorage.getItem('user');
    const user = userStr ? JSON.parse(userStr) : null;
    return getAccessibleTabs(Array.isArray(user?.roles) ? user.roles : []);
  });
  const [themeMode, setThemeMode] = useState<ThemeMode>(() => {
    const stored = localStorage.getItem('themeMode');
    return stored === 'light' || stored === 'dark' ? stored : 'light';
  });

  const theme = useMemo(
    () =>
      createTheme({
        palette: {
          mode: themeMode,
          primary: {
            main: '#abc78f',
          },
          secondary: {
            main: '#c60091',
          },
        },
      }),
    [themeMode]
  );

  const isMobile = useMediaQuery(theme.breakpoints.down('sm'));

  useEffect(() => {
    localStorage.setItem('themeMode', themeMode);
  }, [themeMode]);

  const toggleThemeMode = () => {
    setThemeMode((prev) => (prev === 'light' ? 'dark' : 'light'));
  };

  const handleTabChange = (_event: React.SyntheticEvent, newValue: number) => {
    setTabValue(newValue);
  };

  const handleLoginSuccess = () => {
    const userStr = sessionStorage.getItem('user');
    const user = userStr ? JSON.parse(userStr) : null;
    const roles: { id: number }[] = Array.isArray(user?.roles) ? user.roles : [];
    const tabs = getAccessibleTabs(roles);
    setAccessibleTabs(tabs);
    setTabValue(isAdmin() ? 0 : 4);
    setIsLoggedIn(true);
  };

  const handleLogout = () => {
    sessionStorage.clear();
    window.location.reload();
  };

  const handleAccountConfirmation = () => {
    window.location.href = '/';
  };

  return (
    <ThemeProvider theme={theme}>
      <CssBaseline />

      {confirmEmailCode !== null ? (
        <ConfirmEmailPage activationCode={confirmEmailCode} onSetupComplete={handleAccountConfirmation} />
      ) : !isLoggedIn ? (
        <LoginPage onLoginSuccess={handleLoginSuccess} />
      ) : (
        <Box sx={{ display: 'flex', flexDirection: 'column', minHeight: '100vh' }}>
          {/* App bar */}
          <AppBar position="static">
            <Toolbar>
              <Typography variant="h6" component="div" sx={{ flexGrow: 1 }}>
                OpenFabControl
              </Typography>
              <IconButton color="inherit" onClick={toggleThemeMode} aria-label="toggle theme">
                {themeMode === 'dark' ? <Brightness7Icon /> : <Brightness4Icon />}
              </IconButton>
              <IconButton color="inherit" onClick={handleLogout} aria-label="logout">
                <LogoutIcon />
              </IconButton>
            </Toolbar>
          </AppBar>

          {/* Desktop tabs */}
          {!isMobile && (
            <Box sx={{ borderBottom: 1, borderColor: 'divider' }}>
              <Tabs value={tabValue} onChange={handleTabChange} aria-label="admin navigation tabs" centered>
                {NAV_ITEMS.map((item) => {
                  if (!accessibleTabs.includes(item.index)) return null;
                  const Icon = item.icon;
                  return (
                    <Tab
                      key={item.label}
                      value={item.index}
                      icon={<Icon />}
                      iconPosition="start"
                      label={item.label}
                      id={`tab-${item.index}`}
                      aria-controls={`tabpanel-${item.index}`}
                    />
                  );
                })}
              </Tabs>
            </Box>
          )}

          {/* Tab content */}
          <Box
            sx={{ flexGrow: 1, pb: isMobile ? 7 : 0 }}
            role="tabpanel"
            id={`tabpanel-${tabValue}`}
            aria-labelledby={`tab-${tabValue}`}
          >
            {tabValue === 0 && <UsersPanel />}
            {tabValue === 1 && <RolesPanel />}
            {tabValue === 2 && <SubscriptionsPanel />}
            {tabValue === 3 && <DevicesPanel />}
            {tabValue === 4 && <BookingPanel />}
            {tabValue === 5 && <SettingsPanel />}
          </Box>

          {/* Mobile tabs */}
          {isMobile && (
            <Paper sx={{ position: 'fixed', bottom: 0, left: 0, right: 0 }} elevation={3}>
              <BottomNavigation value={tabValue} onChange={handleTabChange} showLabels>
                {NAV_ITEMS.map((item) => {
                  if (!accessibleTabs.includes(item.index)) return null;
                  const Icon = item.icon;
                  return (
                    <BottomNavigationAction key={item.label} value={item.index} label={item.label} icon={<Icon />} />
                  );
                })}
              </BottomNavigation>
            </Paper>
          )}
        </Box>
      )}
    </ThemeProvider>
  );
}

export default App;
