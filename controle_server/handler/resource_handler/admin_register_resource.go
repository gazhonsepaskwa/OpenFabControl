package resource_handler

import (
	"OpenFabControl/database"
	"OpenFabControl/utils"
	"database/sql"
	"log"
	"net/http"
)

// admin route to manually register a resource (no type restriction)
func Admin_register_resource(w http.ResponseWriter, r *http.Request) {

	if utils.Reject_all_methode_exept(r, w, http.MethodPost) != nil {
		return
	}

	var payload struct {
		UUID         string  `json:"uuid"`
		NAME         string  `json:"name"`
		TYPE         string  `json:"type"`
		ZONE         string  `json:"zone"`
		MANUAL       string  `json:"manual"`
		PRICE_BOOKING float64 `json:"price_booking_in_eur"`
		PRICE_USAGE   float64 `json:"price_usage_in_eur"`
	}

	if utils.Extract_payload_data(r, w, &payload) != nil {
		return
	}

	if !utils.Validate_payload(payload.UUID == "", "uuid cannot be empty", w) {
		return
	}
	if !utils.Validate_payload(payload.NAME == "", "name cannot be empty", w) {
		return
	}
	if !utils.Validate_payload(payload.TYPE == "", "type cannot be empty", w) {
		return
	}

	zone := payload.ZONE
	if zone == "" {
		zone = "UNDEFINED"
	}
	manual := payload.MANUAL
	if manual == "" {
		manual = "UNDEFINED"
	}

	var existingUUID string
	err := database.Self.QueryRow(`SELECT uuid FROM resources WHERE uuid = $1`, payload.UUID).Scan(&existingUUID)
	if err == nil {
		utils.Respond_error(w, "UUID already registered", http.StatusBadRequest)
		return
	}
	if err != sql.ErrNoRows {
		log.Printf("Admin_register_resource: uuid check error: %v", err)
		utils.Respond_error(w, "internal server error", http.StatusInternalServerError)
		return
	}

	_, err = database.Self.Exec(
		`INSERT INTO resources (uuid, type, zone, name, manual, price_booking_in_eur, price_usage_in_eur, approved)
		 VALUES ($1, $2, $3, $4, $5, $6, $7, $8)`,
		payload.UUID, payload.TYPE, zone, payload.NAME, manual, payload.PRICE_BOOKING, payload.PRICE_USAGE, false,
	)
	if err != nil {
		log.Printf("Admin_register_resource: insert error: %v", err)
		utils.Respond_error(w, "internal server error", http.StatusInternalServerError)
		return
	}

	utils.Respond_json(w, map[string]any{
		"msg":  "resource registered",
		"uuid": payload.UUID,
		"name": payload.NAME,
	}, http.StatusCreated)
}
