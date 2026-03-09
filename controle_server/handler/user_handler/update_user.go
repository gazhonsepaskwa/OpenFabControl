package user_handler

import (
	"OpenFabControl/database"
	"OpenFabControl/utils"
	"fmt"
	"net/http"
)

// route to edit a user data
func Update_user(w http.ResponseWriter, r *http.Request) {

	if utils.Reject_all_methode_exept(r, w, http.MethodPost) != nil {
		return
	}

	var payload struct {
		ID                  int `json:"id"`
		ACCESS_KEY          string `json:"access_key"`
		EMAIL               string `json:"email"`
		FIRST_NAME          string `json:"first_name"`
		LAST_NAME           string `json:"last_name"`
		TVA                 string `json:"tva"`
		FACTURATION_ADDRESS string `json:"facturation_address"`
		FACTURATION_ACCOUNT string `json:"facturation_account"`
	}

	if utils.Extract_payload_data(r, w, &payload) != nil {
		return
	}

	if !utils.Validate_payload(payload.ID == 0, "id cannot be empty", w) {
		return
	}

	// construct the SET close of the querry
	set_close := ""
	// anonymous fct to set a comma in front of every elems of the set close exept the first one
	comma := func(i int) string {
		if i != 0 {
			return ", "
		}
		return ""
	}
	i := 0
	if payload.ACCESS_KEY != "" {
		set_close += fmt.Sprint(comma(i), "access_key = $", i+1)
		i++
	}
	if payload.EMAIL != "" {
		set_close += fmt.Sprint(comma(i), "email = $", i+1)
		i++
	}
	if payload.FIRST_NAME != "" {
		set_close += fmt.Sprint(comma(i), "first_name = $", i+1)
		i++
	}
	if payload.LAST_NAME != "" {
		set_close += fmt.Sprint(comma(i), "last_name = $", i+1)
		i++
	}
	if payload.TVA != "" {
		set_close += fmt.Sprint(comma(i), "tva = $", i+1)
		i++
	}
	if payload.FACTURATION_ADDRESS != "" {
		set_close += fmt.Sprint(comma(i), "facturation_address = $", i+1)
		i++
	}
	if payload.FACTURATION_ACCOUNT != "" {
		set_close += fmt.Sprint(comma(i), "facturation_account = $", i+1)
		i++
	}

	if set_close == "" {
		utils.Respond_error(w, "invalid json: no data to update send", http.StatusBadRequest)
		return
	}
	// build the querry
	query := "UPDATE users SET " + set_close + fmt.Sprint(" WHERE id = $", i+1, ";")

	// build parameters
	params := []interface{}{}
	if payload.ACCESS_KEY != "" {
		params = append(params, payload.ACCESS_KEY)
	}
	if payload.EMAIL != "" {
		params = append(params, payload.EMAIL)
	}
	if payload.FIRST_NAME != "" {
		params = append(params, payload.FIRST_NAME)
	}
	if payload.LAST_NAME != "" {
		params = append(params, payload.LAST_NAME)
	}
	if payload.TVA != "" {
		params = append(params, payload.TVA)
	}
	if payload.FACTURATION_ADDRESS != "" {
		params = append(params, payload.FACTURATION_ADDRESS)
	}
	if payload.FACTURATION_ACCOUNT != "" {
		params = append(params, payload.FACTURATION_ACCOUNT)
	}
	params = append(params, payload.ID)

	result, err := database.Self.Exec(query, params...)
	if err != nil {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}
	if rows_affected, _ := result.RowsAffected(); rows_affected == 0 {
		utils.Respond_error(w, "No device with this UUID registered", http.StatusNotFound)
		return
	}

	utils.Respond_json(w, map[string]any{
		"msg": "User edited successfully",
	}, http.StatusOK)
}
