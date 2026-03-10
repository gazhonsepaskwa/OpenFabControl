package session_handler

import (
	"OpenFabControl/database"
	"OpenFabControl/model"
	"OpenFabControl/utils"
	"database/sql"
	"net/http"
	"time"
)

// Route for a resource to add time to the active session (resource_uuid only)
func Add_time(w http.ResponseWriter, r *http.Request) {
	if utils.Reject_all_methode_exept(r, w, http.MethodPost) != nil {
		return
	}

	var payload struct {
		ResourceUUID string `json:"resource_uuid"`
		AddMinutes   int    `json:"add_minutes"`
	}

	if utils.Extract_payload_data(r, w, &payload) != nil {
		return
	}

	if !utils.Validate_payload(payload.ResourceUUID == "", "resource_uuid cannot be empty", w) {
		return
	}
	if payload.AddMinutes <= 0 {
		utils.Respond_error(w, "invalid payload: add_minutes must be > 0", http.StatusBadRequest)
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

	// Compute maximum additional minutes before next booking
	maxAddMinutes, err := computeMaxAddMinutes(session)
	if err != nil {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}

	// If maxAddMinutes == -1, it means unlimited
	if maxAddMinutes != -1 && payload.AddMinutes > maxAddMinutes {
		utils.Respond_error(w, "Not enough free time to extend session", http.StatusBadRequest)
		return
	}

	// Update ended_at by adding the requested minutes
	newEndedAt := session.EndedAt.Add(time.Duration(payload.AddMinutes) * time.Minute)
	_, err = database.Self.Exec("UPDATE sessions SET ended_at = $2 WHERE id = $1", session.ID, newEndedAt)
	if err != nil {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}
	session.EndedAt = newEndedAt

	utils.Respond_json(w, map[string]any{
		"session": session,
	}, http.StatusOK)
}

// Route for a resource to get the maximum time that can be added to the active session
func Get_max_add_time(w http.ResponseWriter, r *http.Request) {
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

	maxAddMinutes, err := computeMaxAddMinutes(session)
	if err != nil {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}

	utils.Respond_json(w, map[string]any{
		"max_add_minutes": maxAddMinutes,
	}, http.StatusOK)
}

// computeMaxAddMinutes returns the maximum number of minutes that can be added
// to the given active session before overlapping the next booking on the same resource.
// If there is no future booking, it returns -1 to indicate \"illimité\".
func computeMaxAddMinutes(active model.Session) (int, error) {
	// Find the next session on the same resource that starts after the current ended_at
	query := `SELECT started_at
		FROM sessions
		WHERE resource_uuid = $1
		  AND id <> $2
		  AND status IN ('planned','progress')
		  AND started_at > $3
		ORDER BY started_at ASC
		LIMIT 1`

	var nextStart time.Time
	err := database.Self.QueryRow(query, active.ResourceUUID, active.ID, active.EndedAt).Scan(&nextStart)
	if err == sql.ErrNoRows {
		// No future session: unlimited
		return -1, nil
	}
	if err != nil {
		return 0, err
	}

	diff := nextStart.Sub(active.EndedAt)
	if diff <= 0 {
		return 0, nil
	}

	return int(diff.Minutes()), nil
}

