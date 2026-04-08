package firmware_handler

import (
	"OpenFabControl/utils"
	"errors"
	"io"
	"net/http"
	"os"
	"path/filepath"
	"strconv"
)

func Firmware(w http.ResponseWriter, r *http.Request) {
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

	st, err := f.Stat()
	if err != nil {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}
	if st.IsDir() {
		utils.Respond_error(w, "firmware.bin not found", http.StatusNotFound)
		return
	}

	w.Header().Set("Content-Type", "application/octet-stream")
	w.Header().Set("Content-Length", strconv.FormatInt(st.Size(), 10))
	w.WriteHeader(http.StatusOK)
	_, _ = io.Copy(w, f)
}

