package firmware_handler

import (
	"OpenFabControl/utils"
	"encoding/json"
	"errors"
	"net/http"
	"os"
	"path/filepath"
)

type FirmwareVersionFile struct {
	Version string `json:"version"`
}

func Last_firmware_version(w http.ResponseWriter, r *http.Request) {
	if utils.Reject_all_methode_exept(r, w, http.MethodGet) != nil {
		return
	}

	folder := os.Getenv("FIRMWARE_FOLDER_PATH")
	if folder == "" {
		utils.Respond_error(w, "FIRMWARE_FOLDER_PATH is not configured", http.StatusInternalServerError)
		return
	}

	versionPath := filepath.Join(folder, "firmware.version")
	raw, err := os.ReadFile(versionPath)
	if err != nil {
		if errors.Is(err, os.ErrNotExist) {
			utils.Respond_error(w, "firmware.version not found", http.StatusNotFound)
			return
		}
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}

	var vf FirmwareVersionFile
	if err := json.Unmarshal(raw, &vf); err != nil {
		utils.Respond_error(w, "invalid firmware.version json", http.StatusInternalServerError)
		return
	}
	if vf.Version == "" {
		utils.Respond_error(w, "invalid firmware.version json: version cannot be empty", http.StatusInternalServerError)
		return
	}

	utils.Respond_json(w, map[string]any{"version": vf.Version}, http.StatusOK)
}

