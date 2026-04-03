package session_handler

import (
	"OpenFabControl/database"
	"OpenFabControl/model"
	"OpenFabControl/utils"
	"database/sql"
	"net/http"
)

// Route for a resource to update the time used on the active session (resource_uuid only)
// time_used_seconds is monotone: server keeps the maximum value (never decreases).
func Update_time_used(w http.ResponseWriter, r *http.Request) {
	if utils.Reject_all_methode_exept(r, w, http.MethodPost) != nil {
		return
	}

	var payload struct {
		ResourceUUID    string `json:"resource_uuid"`
		TimeUsedSeconds int    `json:"time_used_seconds"`
	}

	if utils.Extract_payload_data(r, w, &payload) != nil {
		return
	}

	if !utils.Validate_payload(payload.ResourceUUID == "", "resource_uuid cannot be empty", w) {
		return
	}
	if payload.TimeUsedSeconds < 0 {
		utils.Respond_error(w, "invalid payload: time_used_seconds must be >= 0", http.StatusBadRequest)
		return
	}

	// Find active session: status=progress and NOW() between started_at and ended_at
	query := `SELECT id, user_id, resource_uuid, started_at, ended_at, time_used, status
		FROM sessions
		WHERE resource_uuid = $1
		  AND status = 'progress'
		  AND started_at <= NOW()
		  AND ended_at > NOW()
		ORDER BY started_at DESC
		LIMIT 1`

	var session model.Session
	err := database.Self.QueryRow(query, payload.ResourceUUID).
		Scan(&session.ID, &session.UserID, &session.ResourceUUID, &session.StartedAt, &session.EndedAt, &session.TimeUsed, &session.Status)
	if err == sql.ErrNoRows {
		utils.Respond_error(w, "No active session in progress for this resource", http.StatusNotFound)
		return
	}
	if err != nil {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}

	// Monotone update: keep the maximum value.
	update := `UPDATE sessions
		SET time_used = GREATEST(time_used, $2)
		WHERE id = $1
		RETURNING id, user_id, resource_uuid, started_at, ended_at, time_used, status`
	err = database.Self.QueryRow(update, session.ID, payload.TimeUsedSeconds).
		Scan(&session.ID, &session.UserID, &session.ResourceUUID, &session.StartedAt, &session.EndedAt, &session.TimeUsed, &session.Status)
	if err != nil {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}

	utils.Respond_json(w, map[string]any{
		"session": session,
	}, http.StatusOK)
}

