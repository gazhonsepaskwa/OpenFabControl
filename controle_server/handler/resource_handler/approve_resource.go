package resource_handler

import (
	"OpenFabControl/database"
	"OpenFabControl/utils"
	"net/http"
)

// Route to approve a resource in the system
func Approve_resource(w http.ResponseWriter, r *http.Request) {

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

	// Set approved to true in the db
	query := "UPDATE resources SET approved = TRUE WHERE uuid = $1"
	result, err := database.Self.Exec(query, payload.UUID)
	if err != nil {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}
	if rows_affected, _ := result.RowsAffected(); rows_affected == 0 {
		utils.Respond_error(w, "No device waiting approving with this UUID", http.StatusNotFound)
		return
	}

	utils.Respond_json(w, map[string]any{
		"msg": "Machine controler approved successfully",
	}, http.StatusOK)
}
