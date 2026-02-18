package resource_handler

import (
	"OpenFabControl/database"
	"OpenFabControl/utils"
	"fmt"
	"net/http"
)

// route to edit a resource
func Edit_resource(w http.ResponseWriter, r *http.Request) {

	if utils.Reject_all_methode_exept(r, w, http.MethodPost) != nil {
		return
	}

	var payload struct {
		UUID          string `json:"uuid"`
		ZONE          string `json:"zone"`
		NAME          string `json:"name"`
		MANUAL        string `json:"manual"`
		PRICE_BOOKING string `json:"price_booking_in_eur"`
		PRICE_USAGE   string `json:"price_usage_in_eur"`
	}

	if utils.Extract_payload_data(r, w, &payload) != nil {
		return
	}

	if !utils.Validate_payload(payload.UUID == "", "uuid cannot be empty", w) {
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
	if payload.ZONE != "" {
		set_close += fmt.Sprint(comma(i), "zone = $", i+1)
		i++
	}
	if payload.NAME != "" {
		set_close += fmt.Sprint(comma(i), "name = $", i+1)
		i++
	}
	if payload.MANUAL != "" {
		set_close += fmt.Sprint(comma(i), "manual = $", i+1)
		i++
	}
	if payload.PRICE_BOOKING != "" {
		set_close += fmt.Sprint(comma(i), "price_booking_in_eur = $", i+1)
		i++
	}
	if payload.PRICE_USAGE != "" {
		set_close += fmt.Sprint(comma(i), "price_usage_in_eur = $", i+1)
		i++
	}

	if set_close == "" {
		utils.Respond_error(w, "invalid json: no data to update send", http.StatusBadRequest)
		return
	}
	// build the querry
	query := "UPDATE resources SET " + set_close + fmt.Sprint(" WHERE uuid = $", i+1, ";")

	// build parameters
	params := []interface{}{}
	if payload.ZONE != "" {
		params = append(params, payload.ZONE)
	}
	if payload.NAME != "" {
		params = append(params, payload.NAME)
	}
	if payload.MANUAL != "" {
		params = append(params, payload.MANUAL)
	}
	if payload.PRICE_BOOKING != "" {
		params = append(params, payload.PRICE_BOOKING)
	}
	if payload.PRICE_USAGE != "" {
		params = append(params, payload.PRICE_USAGE)
	}
	params = append(params, payload.UUID)

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
		"msg": "Machine controler edited successfully",
	}, http.StatusOK)
}
