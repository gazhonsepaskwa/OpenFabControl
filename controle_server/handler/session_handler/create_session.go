package session_handler

import (
	"OpenFabControl/database"
	"OpenFabControl/model"
	"OpenFabControl/utils"
	"database/sql"
	"net/http"
	"time"
)

// Route to create a session (admin: user_id, resource: access_key, user: user_id)
func Create_session(w http.ResponseWriter, r *http.Request) {

	if utils.Reject_all_methode_exept(r, w, http.MethodPost) != nil {
		return
	}

	var payload struct {
		UserID       *int    `json:"user_id"`
		AccessKey    *string `json:"access_key"`
		ResourceUUID string  `json:"resource_uuid"`
		StartedAt    string  `json:"started_at"`
		EndedAt      string  `json:"ended_at"`
	}

	if utils.Extract_payload_data(r, w, &payload) != nil {
		return
	}

	if !utils.Validate_payload(payload.ResourceUUID == "", "resource_uuid cannot be empty", w) {
		return
	}
	if !utils.Validate_payload(payload.StartedAt == "", "started_at cannot be empty", w) {
		return
	}

	// Resolve user_id from user_id or access_key
	var userID int
	if payload.UserID != nil {
		userID = *payload.UserID
	} else if payload.AccessKey != nil && *payload.AccessKey != "" {
		err := database.Self.QueryRow("SELECT id FROM users WHERE access_key = $1", *payload.AccessKey).Scan(&userID)
		if err == sql.ErrNoRows {
			utils.Respond_error(w, "No user found with this access_key", http.StatusNotFound)
			return
		}
		if err != nil {
			utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
			return
		}
	} else {
		utils.Respond_error(w, "user_id or access_key is required", http.StatusBadRequest)
		return
	}

	// Parse started_at
	startedAt, err := time.Parse(time.RFC3339, payload.StartedAt)
	if err != nil {
		utils.Respond_error(w, "invalid payload: started_at must be RFC3339 format (e.g. 2006-01-02T15:04:05Z07:00)", http.StatusBadRequest)
		return
	}

	if !startedAt.After(time.Now()) {
		utils.Respond_error(w, "started_at must be in the future", http.StatusBadRequest)
		return
	}

	// Parse or compute ended_at
	var endedAt time.Time
	if payload.EndedAt != "" {
		endedAt, err = time.Parse(time.RFC3339, payload.EndedAt)
		if err != nil {
			utils.Respond_error(w, "invalid payload: ended_at must be RFC3339 format", http.StatusBadRequest)
			return
		}
		if !endedAt.After(startedAt) {
			utils.Respond_error(w, "ended_at must be after started_at", http.StatusBadRequest)
			return
		}
	} else {
		endedAt = startedAt.Add(30 * time.Minute)
	}

	// Check user exists
	var exists int
	err = database.Self.QueryRow("SELECT 1 FROM users WHERE id = $1", userID).Scan(&exists)
	if err == sql.ErrNoRows {
		utils.Respond_error(w, "User not found", http.StatusNotFound)
		return
	}
	if err != nil {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}

	// Check resource exists
	err = database.Self.QueryRow("SELECT 1 FROM resources WHERE uuid = $1", payload.ResourceUUID).Scan(&exists)
	if err == sql.ErrNoRows {
		utils.Respond_error(w, "Resource not found with this uuid", http.StatusNotFound)
		return
	}
	if err != nil {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}

	// Check no overlap: existing session overlaps if started_at < new_ended_at AND ended_at > new_started_at
	var overlapping int
	err = database.Self.QueryRow(
		"SELECT 1 FROM sessions WHERE resource_uuid = $1 AND started_at < $2 AND ended_at > $3",
		payload.ResourceUUID, endedAt, startedAt,
	).Scan(&overlapping)
	if err == nil {
		utils.Respond_error(w, "A session already exists for this resource in the requested time slot", http.StatusBadRequest)
		return
	}
	if err != sql.ErrNoRows {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}

	// Insert session
	query := `INSERT INTO sessions (user_id, resource_uuid, started_at, ended_at, time_used, status)
		VALUES ($1, $2, $3, $4, 0, 'planned')
		RETURNING id, user_id, resource_uuid, started_at, ended_at, time_used, status`
	var session model.Session
	err = database.Self.QueryRow(query, userID, payload.ResourceUUID, startedAt, endedAt).
		Scan(&session.ID, &session.UserID, &session.ResourceUUID, &session.StartedAt, &session.EndedAt, &session.TimeUsed, &session.Status)
	if err != nil {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}

	utils.Respond_json(w, map[string]any{
		"session": session,
	}, http.StatusCreated)
}
