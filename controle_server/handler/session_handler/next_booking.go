package session_handler

import (
	"OpenFabControl/database"
	"OpenFabControl/utils"
	"database/sql"
	"net/http"
	"strings"
	"time"
)

const maxUserNameLen = 32

type NextBookingResponse struct {
	StartAt  string `json:"start_at"`
	EndAt    string `json:"end_at"`
	UserName string `json:"user_name"`
}

// Route for a resource to get next or current booking (resource_uuid only).
func Next_booking(w http.ResponseWriter, r *http.Request) {
	if utils.Reject_all_methode_exept(r, w, http.MethodPost) != nil {
		return
	}

	var payload struct {
		ResourceUUID string `json:"resource_uuid"`
	}
	if utils.Extract_payload_data(r, w, &payload) != nil {
		return
	}
	if !utils.Validate_payload(payload.ResourceUUID == "", "resource_uuid cannot be empty", w) {
		return
	}

	query := `
		SELECT s.started_at, s.ended_at, u.first_name, u.last_name
		FROM sessions s
		JOIN users u ON u.id = s.user_id
		WHERE s.resource_uuid = $1 AND s.status IN ('planned','progress') AND s.ended_at > NOW()
		ORDER BY s.started_at ASC
		LIMIT 1`
	var startedAt, endedAt time.Time
	var firstName, lastName string
	err := database.Self.QueryRow(query, payload.ResourceUUID).
		Scan(&startedAt, &endedAt, &firstName, &lastName)
	if err == sql.ErrNoRows {
		utils.Respond_json(w, map[string]any{"next_booking": nil}, http.StatusOK)
		return
	}
	if err != nil {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}

	userName := strings.TrimSpace(firstName + " " + lastName)
	if len(userName) > maxUserNameLen {
		userName = userName[:maxUserNameLen]
	}
	if userName == "" {
		userName = "—"
	}

	nb := NextBookingResponse{
		StartAt:  startedAt.UTC().Format(time.RFC3339),
		EndAt:    endedAt.UTC().Format(time.RFC3339),
		UserName: userName,
	}
	utils.Respond_json(w, map[string]any{"next_booking": nb}, http.StatusOK)
}
