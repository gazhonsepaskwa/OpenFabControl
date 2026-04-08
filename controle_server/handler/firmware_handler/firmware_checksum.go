package firmware_handler

import (
	"OpenFabControl/utils"
	"crypto/sha256"
	"encoding/hex"
	"errors"
	"io"
	"net/http"
	"os"
	"path/filepath"
)

func Firmware_checksum(w http.ResponseWriter, r *http.Request) {
	if utils.Reject_all_methode_exept(r, w, http.MethodGet) != nil {
		return
	}

	folder := os.Getenv("FIRMWARE_FOLDER_PATH")
	if folder == "" {
		utils.Respond_error(w, "FIRMWARE_FOLDER_PATH is not configured", http.StatusInternalServerError)
		return
	}

	fwPath := filepath.Join(folder, "firmware.bin")
	f, err := os.Open(fwPath)
	if err != nil {
		if errors.Is(err, os.ErrNotExist) {
			utils.Respond_error(w, "firmware.bin not found", http.StatusNotFound)
			return
		}
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}
	defer f.Close()

	h := sha256.New()
	if _, err := io.Copy(h, f); err != nil {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}

	sum := hex.EncodeToString(h.Sum(nil))
	utils.Respond_json(w, map[string]any{"sha256": sum}, http.StatusOK)
}

