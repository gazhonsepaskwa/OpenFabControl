package user_handler

import (
	"OpenFabControl/database"
	"OpenFabControl/model"
	"OpenFabControl/utils"
	"encoding/json"
	"log"
	"net/http"
)

// handler for the get user route
func Get_user_list(w http.ResponseWriter, r *http.Request) {

	if utils.Reject_all_methode_exept(r, w, http.MethodGet) != nil {
		return
	}

	// get the users
	query := "SELECT id, email, access_key, first_name, last_name, tva, facturation_address, facturation_account, status, created_at FROM users"
	var users []model.User
	rows, err := database.Self.Query(query)
	if err != nil {
		utils.Respond_error(w, "Internal server error", http.StatusInternalServerError)
		return
	}
	defer rows.Close()

	// translate the rows
	for rows.Next() {
		var user model.User
		if err := rows.Scan(&user.ID,
			&user.EMAIL,
			&user.ACCESS_KEY,
			&user.FIRST_NAME,
			&user.LAST_NAME,
			&user.TVA,
			&user.FACTURATION_ADDRESS,
			&user.FACTURATION_ACCOUNT,
			&user.STATUS,
			&user.CreatedAt); err != nil {
			utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
			log.Printf("%v", err)
			return
		}
		users = append(users, user)
	}

	// send data
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	if err := json.NewEncoder(w).Encode(users); err != nil {
		utils.Respond_error(w, "Internal server error", http.StatusInternalServerError)
		return
	}
}
