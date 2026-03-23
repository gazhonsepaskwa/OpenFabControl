import AdminPanelSettingsIcon from '@mui/icons-material/AdminPanelSettings';
import Brightness4Icon from '@mui/icons-material/Brightness4';
import Brightness7Icon from '@mui/icons-material/Brightness7';
import DevicesIcon from '@mui/icons-material/Devices';
import PeopleIcon from '@mui/icons-material/People';
import SubscriptionsIcon from '@mui/icons-material/Subscriptions';
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
import DevicesPanel from './components/DevicesPanel';
import RolesPanel from './components/RolesPanel';
import SubscriptionsPanel from './components/SubscriptionsPanel';
import UsersPanel from './components/UsersPanel';

const NAV_ITEMS = [
  { label: 'Users', icon: PeopleIcon },
  { label: 'Roles', icon: AdminPanelSettingsIcon },
  { label: 'Subscriptions', icon: SubscriptionsIcon },
  { label: 'Devices', icon: DevicesIcon },
];

type ThemeMode = 'light' | 'dark';

function App() {
  const [tabValue, setTabValue] = useState(0);
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

  return (
    <ThemeProvider theme={theme}>
      <CssBaseline />
      <Box sx={{ display: 'flex', flexDirection: 'column', minHeight: '100vh' }}>
        {/* App bar */}
        <AppBar position="static">
          <Toolbar>
            <Typography variant="h6" component="div" sx={{ flexGrow: 1 }}>
              OpenFabControl Admin
            </Typography>
            <IconButton color="inherit" onClick={toggleThemeMode} aria-label="toggle theme">
              {themeMode === 'dark' ? <Brightness7Icon /> : <Brightness4Icon />}
            </IconButton>
          </Toolbar>
        </AppBar>

        {/* Desktop tabs */}
        {!isMobile && (
          <Box sx={{ borderBottom: 1, borderColor: 'divider' }}>
            <Tabs value={tabValue} onChange={handleTabChange} aria-label="admin navigation tabs" centered>
              {NAV_ITEMS.map((item, index) => {
                const Icon = item.icon;
                return (
                  <Tab
                    key={item.label}
                    icon={<Icon />}
                    iconPosition="start"
                    label={item.label}
                    id={`tab-${index}`}
                    aria-controls={`tabpanel-${index}`}
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
        </Box>

        {/* Mobile tabs */}
        {isMobile && (
          <Paper sx={{ position: 'fixed', bottom: 0, left: 0, right: 0 }} elevation={3}>
            <BottomNavigation value={tabValue} onChange={handleTabChange} showLabels>
              {NAV_ITEMS.map((item) => {
                const Icon = item.icon;
                return <BottomNavigationAction key={item.label} label={item.label} icon={<Icon />} />;
              })}
            </BottomNavigation>
          </Paper>
        )}
      </Box>
    </ThemeProvider>
  );
}

export default App;
