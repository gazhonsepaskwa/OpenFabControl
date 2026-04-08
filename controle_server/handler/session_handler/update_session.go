package session_handler

import (
	"OpenFabControl/database"
	"OpenFabControl/model"
	"OpenFabControl/utils"
	"database/sql"
	"net/http"
	"strings"
	"time"
)

// Route to update a planned session.
// Admin: can change resource_uuid, started_at, ended_at and user_id.
// User:  can change resource_uuid, started_at and ended_at for their own sessions only.
func Update_session(w http.ResponseWriter, r *http.Request) {
	if utils.Reject_all_methode_exept(r, w, http.MethodPost) != nil {
		return
	}

	var payload struct {
		ID           int    `json:"id"`
		UserID       *int   `json:"user_id"`
		ResourceUUID string `json:"resource_uuid"`
		StartedAt    string `json:"started_at"`
		EndedAt      string `json:"ended_at"`
	}

	if utils.Extract_payload_data(r, w, &payload) != nil {
		return
	}

	if !utils.Validate_payload(payload.ID == 0, "id cannot be empty", w) {
		return
	}
	if !utils.Validate_payload(payload.ResourceUUID == "", "resource_uuid cannot be empty", w) {
		return
	}
	if !utils.Validate_payload(payload.StartedAt == "", "started_at cannot be empty", w) {
		return
	}
	if !utils.Validate_payload(payload.EndedAt == "", "ended_at cannot be empty", w) {
		return
	}

	// Parse time fields
	startedAt, err := time.Parse(time.RFC3339, payload.StartedAt)
	if err != nil {
		utils.Respond_error(w, "invalid payload: started_at must be RFC3339 format", http.StatusBadRequest)
		return
	}
	endedAt, err := time.Parse(time.RFC3339, payload.EndedAt)
	if err != nil {
		utils.Respond_error(w, "invalid payload: ended_at must be RFC3339 format", http.StatusBadRequest)
		return
	}
	if !endedAt.After(startedAt) {
		utils.Respond_error(w, "ended_at must be after started_at", http.StatusBadRequest)
		return
	}

	// Load existing session
	var session model.Session
	err = database.Self.QueryRow(
		`SELECT id, user_id, resource_uuid, started_at, ended_at, time_used, status FROM sessions WHERE id = $1`,
		payload.ID,
	).Scan(&session.ID, &session.UserID, &session.ResourceUUID, &session.StartedAt, &session.EndedAt, &session.TimeUsed, &session.Status)
	if err == sql.ErrNoRows {
		utils.Respond_error(w, "Session not found", http.StatusNotFound)
		return
	}
	if err != nil {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}

	// Only planned sessions can be edited
	if session.Status != "planned" {
		utils.Respond_error(w, "Only planned sessions can be edited", http.StatusBadRequest)
		return
	}

	// For user route: enforce ownership
	isAdminRoute := strings.HasPrefix(r.URL.Path, "/web-admin-api/")
	if !isAdminRoute {
		jwtUserID, err := getUserIDFromToken(r)
		if err != nil || jwtUserID != session.UserID {
			utils.Respond_error(w, "Forbidden", http.StatusForbidden)
			return
		}
	}

	// Resolve target user_id (admin may override, otherwise keep existing)
	targetUserID := session.UserID
	if isAdminRoute && payload.UserID != nil {
		targetUserID = *payload.UserID
		// Check the new user exists
		var exists int
		err = database.Self.QueryRow("SELECT 1 FROM users WHERE id = $1", targetUserID).Scan(&exists)
		if err == sql.ErrNoRows {
			utils.Respond_error(w, "User not found", http.StatusNotFound)
			return
		}
		if err != nil {
			utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
			return
		}
	}

	// Check overlap, excluding the session being updated
	var overlapping int
	err = database.Self.QueryRow(
		`SELECT 1 FROM sessions
		 WHERE resource_uuid = $1 AND id != $2 AND started_at < $3 AND ended_at > $4`,
		payload.ResourceUUID, payload.ID, endedAt, startedAt,
	).Scan(&overlapping)
	if err == nil {
		utils.Respond_error(w, "A session already exists for this resource in the requested time slot", http.StatusBadRequest)
		return
	}
	if err != sql.ErrNoRows {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}

	// Apply update
	err = database.Self.QueryRow(
		`UPDATE sessions
		 SET user_id = $1, resource_uuid = $2, started_at = $3, ended_at = $4
		 WHERE id = $5
		 RETURNING id, user_id, resource_uuid, started_at, ended_at, time_used, status`,
		targetUserID, payload.ResourceUUID, startedAt, endedAt, payload.ID,
	).Scan(&session.ID, &session.UserID, &session.ResourceUUID, &session.StartedAt, &session.EndedAt, &session.TimeUsed, &session.Status)
	if err != nil {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}

	utils.Respond_json(w, map[string]any{
		"session": session,
	}, http.StatusOK)
}
