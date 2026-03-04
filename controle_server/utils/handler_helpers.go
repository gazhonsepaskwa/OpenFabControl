package utils

import (
	"net/http"
	"encoding/json"
	"fmt"
)

// return the error as a json in the folowing format : { "err" : [error msg] }
func Respond_error(w http.ResponseWriter, msg string, status_code int) {
	w.WriteHeader(status_code)
	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(map[string]any{"error": msg})
}

// return the success as a json, necesary key pair: "msg" : "..."
func Respond_json(w http.ResponseWriter, json_map map[string]any, status_code int) {
	w.WriteHeader(status_code)
	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(json_map)
}

// helper to reject a request if the methode used is not the specified one
func Reject_all_methode_exept(r *http.Request, w http.ResponseWriter, methode string) error {
	if r.Method != methode {
		Respond_error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return fmt.Errorf("Method not allowed")
	}
	return nil
}

// helper to extract the json data sent
func Extract_payload_data(r *http.Request, w http.ResponseWriter, payload any) error {
	if err := json.NewDecoder(r.Body).Decode(payload); err != nil {
		Respond_error(w, "invalid json", http.StatusBadRequest)
		return fmt.Errorf("invalid json")
	}
	return nil
}

// helper to check a condition of the payload
func Validate_payload(condition bool, error_msg string, w http.ResponseWriter) bool {
	if condition {
		Respond_error(w, "invalid payload: " + error_msg, http.StatusBadRequest)
		return false
	}
	return true
}

// helper to check a condition of the payload without pre error message
func Validate_payload_no_msg(condition bool, error_msg string, w http.ResponseWriter) bool {
	if condition {
		Respond_error(w, error_msg, http.StatusBadRequest)
		return false
	}
	return true
}
