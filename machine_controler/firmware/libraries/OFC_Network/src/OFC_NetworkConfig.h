#pragma once

// Network / API configuration constants.

// HTTP client timeout (ms) for API calls.
static constexpr unsigned long OFC_MACHINE_API_TIMEOUT_MS = 30000UL;

// WiFi check and reconnect intervals (ms).
static constexpr unsigned long OFC_WIFI_CHECK_INTERVAL_MS = 2000UL;
static constexpr unsigned long OFC_WIFI_RECONNECT_INTERVAL_MS = 5000UL;

// Timezone for NTP and display (POSIX TZ string)
// Examples: "CET-1CEST,M3.5.0,M10.5.0/3" (Europe/Paris), "UTC0", "EST5EDT,M3.2.0,M11.1.0"
static constexpr const char* OFC_TZ_STRING = "CET-1CEST,M3.5.0,M10.5.0/3";

// Refresh interval for fetching next booking info.
static constexpr unsigned long OFC_NEXT_BOOKING_REFRESH_MINUTES = 1UL;
static constexpr unsigned long OFC_NEXT_BOOKING_REFRESH_INTERVAL_MS =
    (OFC_NEXT_BOOKING_REFRESH_MINUTES * 60UL * 1000UL);

// API validation: minimum duration (minutes) when creating a session.
static constexpr int OFC_BOOK_SESSION_MIN_MINUTES = 10;

// POST /machine-api/update_time_used while on MACHINE_USAGE (active time from GPA1).
static constexpr unsigned long OFC_UPDATE_TIME_USED_INTERVAL_MS = 15000UL;
