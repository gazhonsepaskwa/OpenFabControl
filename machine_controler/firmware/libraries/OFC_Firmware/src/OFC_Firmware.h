#pragma once

// Firmware version used for OTA comparison.
// Keep it in simple dotted numeric form: "MAJOR.MINOR[.PATCH]".
static constexpr const char* OFC_FIRMWARE_VERSION = "0.1.3";

// Compare semantic-like versions (digits + dots), e.g. "0.1" < "0.1.1" < "1.0".
// - Ignores leading 'v' or 'V'
// - Missing parts are treated as 0: "0.1" == "0.1.0"
// Returns:
//   -1 if a < b
//    0 if a == b
//   +1 if a > b
int ofc_compare_versions(const char* a, const char* b);

