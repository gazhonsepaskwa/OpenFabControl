package session_handler

import (
	"OpenFabControl/utils"
	"OpenFabControl/model"
	"OpenFabControl/database"
	"net/http"
	"database/sql"
)

// Route to create a session (admin: user_id, resource: access_key, user: JWT context)
func Delete_session(w http.ResponseWriter, r* http.Request) {

	if utils.Reject_all_methode_exept(r, w, http.MethodDelete) != nil { return }

	var payload struct {
		SessionID	 *int 	 `json:"session_id"`
	}

	if utils.Extract_payload_data(r, w, &payload) != nil { return }

	if !utils.Validate_payload(payload.SessionID == nil, "session_id cannot be empty", w) { return }

	var userID int
	if id, ok := r.Context().Value("user_id").(int); ok && id != 0 {
		userID = id
	}

	// querry the user_id linked to the sesssion (for the premition check and the msg if don't exist)
	query := `SELECT user_id FROM sessions WHERE id = $1`
	var session model.Session
	row := database.Self.QueryRow(query, payload.SessionID)
	if err := row.Scan(&session.UserID); err != nil {
		if err == sql.ErrNoRows {
			utils.Respond_error(w, "The session you tried to delete does not exist", http.StatusNotFound)
			return
		}
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}

	// check if the session belong to the user if not admin
	if admin, ok := r.Context().Value("is_admin").(bool); ok && admin == false {
		if (session.UserID != userID) {
			utils.Respond_error(w, "The session you tried to delete does not belong to you", http.StatusUnauthorized)
			return
		}
	}

	// delete the session from the db
		query = `DELETE FROM sessions WHERE id = $1`
		res, err := database.Self.Exec(query, payload.SessionID)
		if err != nil {
			utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
			return
		}
		if rows_affected, _ := res.RowsAffected(); rows_affected == 0 {
			utils.Respond_error(w, "This session does not exist", http.StatusNotFound)
			return
		}
		utils.Respond_json(w, map[string]any{
			"msg" : "Session deleted successfully",
		}, http.StatusOK)

}
