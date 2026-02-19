package resource_handler

import (
	"OpenFabControl/database"
	"OpenFabControl/utils"
	"database/sql"
	"net/http"
)

// route register a new machine controler
func Register(w http.ResponseWriter, r *http.Request) {

	if utils.Reject_all_methode_exept(r, w, http.MethodPost) != nil {
		return
	}

	var payload struct {
		UUID string `json:"uuid"`
		NAME string `json:"name"`
		TYPE string `json:"type"`
	}

	if utils.Extract_payload_data(r, w, &payload) != nil {
		return
	}


	// validate payload data
	if !utils.Validate_payload(payload.UUID == "", "uuid cannot be empty", w) {
		return
	}
	if !utils.Validate_payload(payload.NAME == "", "name cannot be empty", w) {
		return
	}
	if !utils.Validate_payload(
		payload.TYPE != "fm-bv2", // && payload.TYPE != "ofmc" && payload.TYPE != "toolsquare" // (future suport)
		"unknown or unsuported machine type. Curently supported: 'fm-bv2'. You use '"+payload.TYPE+"'. Is the server up to date ?",
		w,
	) {
		return
	}

	query := `INSERT INTO resources (uuid, type, zone, name, manual, price_booking_in_eur, price_usage_in_eur, approved) VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
	ON CONFLICT (uuid) DO NOTHING`

	// Check if UUID already exists
	var existingUUID string
	err := database.Self.QueryRow(`SELECT uuid FROM resources WHERE uuid = $1`, payload.UUID).Scan(&existingUUID)
	if err == nil {
		// UUID already exists
		utils.Respond_error(w, "UUID already registered", http.StatusBadRequest)
		return
	}
	if err != sql.ErrNoRows {
		utils.Respond_error(w, "internal server error", http.StatusInternalServerError)
		return
	}

	// UUID doesn't exist, proceed with insertion
	_, err = database.Self.Exec(query, payload.UUID, payload.TYPE, "UNDEFINED", payload.NAME, "UNDEFINED", 0, 0, false)
	if err != nil {
		utils.Respond_error(w, "internal server error", http.StatusInternalServerError)
		return
	}

	utils.Respond_json(w, map[string]any{
		"msg":  "registration saved",
		"uuid": payload.UUID,
		"name": payload.NAME,
	}, http.StatusCreated)
}
