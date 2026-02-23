package resource_handler

import (
	"OpenFabControl/database"
	"OpenFabControl/utils"
	"database/sql"
	"net/http"
)

// Route for a resource (machine controller) to check if it is approved
func Check_approval_status(w http.ResponseWriter, r *http.Request) {

	if utils.Reject_all_methode_exept(r, w, http.MethodPost) != nil {
		return
	}

	var payload struct {
		UUID string `json:"uuid"`
	}

	if utils.Extract_payload_data(r, w, &payload) != nil {
		return
	}

	if !utils.Validate_payload(payload.UUID == "", "uuid cannot be empty", w) {
		return
	}

	var approved bool
	err := database.Self.QueryRow("SELECT approved FROM resources WHERE uuid = $1", payload.UUID).Scan(&approved)
	if err == sql.ErrNoRows {
		utils.Respond_error(w, "Resource not found with this UUID", http.StatusNotFound)
		return
	}
	if err != nil {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}

	utils.Respond_json(w, map[string]any{
		"approved": approved,
	}, http.StatusOK)
}
