package session_handler

import (
	"OpenFabControl/database"
	"OpenFabControl/model"
	"OpenFabControl/utils"
	"database/sql"
	"net/http"
)

// Route for a resource to start a session (access_key + resource_uuid)
func Start_session(w http.ResponseWriter, r *http.Request) {

	if utils.Reject_all_methode_exept(r, w, http.MethodPost) != nil {
		return
	}

	var payload struct {
		AccessKey    string `json:"access_key"`
		ResourceUUID string `json:"resource_uuid"`
	}

	if utils.Extract_payload_data(r, w, &payload) != nil {
		return
	}

	if !utils.Validate_payload_no_msg(payload.AccessKey == "", "Unsuported badge type", w) {
		return
	}
	if !utils.Validate_payload(payload.ResourceUUID == "", "resource_uuid cannot be empty", w) {
		return
	}

	// Resolve access_key -> user_id
	var userID int
	err := database.Self.QueryRow("SELECT id FROM users WHERE access_key = $1", payload.AccessKey).Scan(&userID)
	if err == sql.ErrNoRows {
		utils.Respond_error(w, "Badge not registered", http.StatusNotFound)
		return
	}
	if err != nil {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}

	// Find session: planned or progress, for this user+resource, started_at <= NOW()
	query := `SELECT id, user_id, resource_uuid, started_at, ended_at, time_used, status
		FROM sessions
		WHERE user_id = $1 AND resource_uuid = $2
		AND status IN ('planned','progress') AND started_at <= NOW()
		ORDER BY started_at DESC LIMIT 1`
	var session model.Session
	err = database.Self.QueryRow(query, userID, payload.ResourceUUID).
		Scan(&session.ID, &session.UserID, &session.ResourceUUID, &session.StartedAt, &session.EndedAt, &session.TimeUsed, &session.Status)
	if err == sql.ErrNoRows {
		utils.Respond_error(w, "No session found to start", http.StatusNotFound)
		return
	}
	if err != nil {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}

	// Update status to progress
	_, err = database.Self.Exec("UPDATE sessions SET status = 'progress' WHERE id = $1", session.ID)
	if err != nil {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}
	session.Status = "progress"

	utils.Respond_json(w, map[string]any{
		"session": session,
	}, http.StatusOK)
}
