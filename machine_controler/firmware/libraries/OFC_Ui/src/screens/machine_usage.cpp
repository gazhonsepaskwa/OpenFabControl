#include <OFC_Ui.h>
#include <OFC_Hardware.h>
#include <OFC_Network.h>
#include <OFC_NetworkConfig.h>

extern Session      g_current_session;
extern OFC_Hardware g_hardware;
extern OFC_Network  g_network;

void OFC_Ui::reset_session_active_usage_tracking() {
    _session_active_used_ms      = 0;
    _session_active_last_tick_ms = millis();
    _last_update_time_used_api_ms = millis();
}

unsigned int OFC_Ui::get_session_active_used_seconds() const {
    return (unsigned int)(_session_active_used_ms / 1000);
}

void OFC_Ui::update_machine_usage_times() {
    unsigned long now = millis();
    if (_session_active_last_tick_ms == 0) {
        _session_active_last_tick_ms = now;
    } else {
        unsigned long dt = now - _session_active_last_tick_ms;
        if (g_hardware.mcp2.digitalRead(MCP2_GPA1) == LOW) {
            _session_active_used_ms += dt;
        }
        _session_active_last_tick_ms = now;
    }

    if ((now - _last_machine_usage_time_update_ms) >= 1000) {
        draw_machine_usage_times_inner();
        _last_machine_usage_time_update_ms = now;
    }

    if (g_network.api && g_network.isWifiConnected()
        && (now - _last_update_time_used_api_ms) >= OFC_UPDATE_TIME_USED_INTERVAL_MS) {
        int sec = (int)get_session_active_used_seconds();
        char errbuf[64] = {0};
        if (g_network.api->update_time_used(sec, &g_current_session, errbuf, sizeof(errbuf))) {
            // g_current_session.time_used synced from server (monotone max)
        } else {
            // keep UI; retry on next interval
        }
        _last_update_time_used_api_ms = now;
    }
}

void OFC_Ui::draw_machine_usage_times_inner() {
    time_t now_sec = time(nullptr);
    int time_left = (int)(g_current_session.ended_at_unix - now_sec);
    int time_used_val = (int)(_session_active_used_ms / 1000);
    if (time_left < 0) time_left = 0;
    if (time_used_val < 0) time_used_val = 0;

    char buf[16];
    format_hms(time_left, buf, sizeof(buf));
    _tft->fillRect(0, 95, 160, 45, ILI9341_BLACK);
    printTFTcentered(buf, _tft->color565(255, 255, 255), 3, 0, 95, 160, 45);

    format_hms(time_used_val, buf, sizeof(buf));
    _tft->fillRect(160, 95, 160, 45, ILI9341_BLACK);
    printTFTcentered(buf, _tft->color565(255, 255, 255), 3, 160, 95, 160, 45);
}

void OFC_Ui::draw_machine_usage() {
    clear_screen();
    draw_title((char*)_machine_name.c_str());
    draw_button_left("Add time");
    draw_button_right("Finish", 255, 100, 100);
    printTFTcentered("Time Left:", _tft->color565(255, 255, 255), 2, 0, 70, 160, 30);
    printTFTcentered("Time used:", _tft->color565(255, 255, 255), 2, 160, 70, 160, 30);
    draw_machine_usage_times_inner();
    _menu = MACHINE_USAGE;
    // Avoid counting time spent on other menus (e.g. ADD_TIME) as active usage.
    _session_active_last_tick_ms = millis();
}
