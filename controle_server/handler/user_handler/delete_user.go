package user_handler

import (
	"OpenFabControl/utils"
	"OpenFabControl/database"
	"net/http"
)

// route to delete a user
func Delete_user(w http.ResponseWriter, r* http.Request) {

	if utils.Reject_all_methode_exept(r, w, http.MethodDelete) != nil { return }

	var payload struct {
		USER_ID		int	`json:"user_id"`
	}

	if utils.Extract_payload_data(r, w, &payload) != nil { return }

	if !utils.Validate_payload(payload.USER_ID == 0, "user_id cannot be empty", w) { return }

	// delete the role from the db
	query := `DELETE FROM users WHERE id = $1`
	res, err := database.Self.Exec(query, payload.USER_ID)
	if err != nil {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}
	if rows_affected, _ := res.RowsAffected(); rows_affected == 0 {
		utils.Respond_error(w, "No user with this id saved", http.StatusNotFound)
		return
	}
	utils.Respond_json(w, map[string]any{
		"msg" : "User deleted successfully",
	}, http.StatusOK)

}
