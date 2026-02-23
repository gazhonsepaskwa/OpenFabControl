package session_handler

import (
	"OpenFabControl/database"
	"OpenFabControl/model"
	"OpenFabControl/utils"
	"database/sql"
	"net/http"
)

// Route for a resource to stop the current session (resource_uuid only)
func Stop_session(w http.ResponseWriter, r *http.Request) {

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

	// Find session in progress for this resource
	query := `SELECT id, user_id, resource_uuid, started_at, ended_at, time_used, status
		FROM sessions
		WHERE resource_uuid = $1 AND status = 'progress'`
	var session model.Session
	err := database.Self.QueryRow(query, payload.ResourceUUID).
		Scan(&session.ID, &session.UserID, &session.ResourceUUID, &session.StartedAt, &session.EndedAt, &session.TimeUsed, &session.Status)
	if err == sql.ErrNoRows {
		utils.Respond_error(w, "No session in progress for this resource", http.StatusNotFound)
		return
	}
	if err != nil {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}

	// Update status to done
	_, err = database.Self.Exec("UPDATE sessions SET status = 'done' WHERE id = $1", session.ID)
	if err != nil {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}
	session.Status = "done"

	utils.Respond_json(w, map[string]any{
		"session": session,
	}, http.StatusOK)
}
