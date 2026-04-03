package user_handler

import (
	"OpenFabControl/database"
	"OpenFabControl/model"
	"OpenFabControl/utils"
	"database/sql"
	"net/http"
	"os"
	"time"

	"github.com/golang-jwt/jwt/v5"
)

// route to login as admin (requires 'admin' role)
func AdminLogin(w http.ResponseWriter, r *http.Request) {
	if utils.Reject_all_methode_exept(r, w, http.MethodPost) != nil {
		return
	}

	var payload struct {
		EMAIL    string `json:"email"`
		PASSWORD string `json:"password"`
	}

	if utils.Extract_payload_data(r, w, &payload) != nil {
		return
	}

	if !utils.Validate_payload(payload.EMAIL == "", "email cannot be empty", w) {
		return
	}
	if !utils.Validate_payload(payload.PASSWORD == "", "password cannot be empty", w) {
		return
	}

	hash, profile, err := fetchUserByEmailForLogin(payload.EMAIL)
	if err != nil {
		if err == sql.ErrNoRows {
			utils.Respond_error(w, "Invalid credential", http.StatusForbidden)
			return
		}
		utils.Respond_error(w, "Internal server error", http.StatusInternalServerError)
		return
	}

	if !utils.CheckPasswordHash(payload.PASSWORD, hash) {
		utils.Respond_error(w, "Invalid credential", http.StatusForbidden)
		return
	}

	userID := profile.ID

	// enforce account status (same logic as auth middleware)
	if utils.Reject_user_status(w, userID, []string{"pending", "desactivated"}) != nil {
		utils.Respond_error(w, "Your account is desactivated or pending activation, if you're part of the system, contact the administrator", http.StatusUnauthorized)
		return
	}

	// enforce admin role
	var exists int
	err = database.Self.QueryRow(
		`SELECT 1
		FROM users_roles ur
		JOIN roles r ON r.id = ur.role_id
		WHERE ur.user_id = $1 AND r.name = 'admin'`,
		userID,
	).Scan(&exists)
	if err != nil {
		if err == sql.ErrNoRows {
			utils.Respond_error(w, "Forbidden", http.StatusForbidden)
			return
		}
		utils.Respond_error(w, "internal server error", http.StatusInternalServerError)
		return
	}

	roles, err := fetchRolesForUser(userID)
	if err != nil {
		utils.Respond_error(w, "Internal server error", http.StatusInternalServerError)
		return
	}
	profile.Roles = roles

	// JWT token
	expirationTime := time.Now().Add(24 * time.Hour)
	claims := model.Claims{
		USERID: userID,
		EMAIL:  payload.EMAIL,
		RegisteredClaims: jwt.RegisteredClaims{
			ExpiresAt: jwt.NewNumericDate(expirationTime),
			IssuedAt:  jwt.NewNumericDate(time.Now()),
		},
	}

	token := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)
	secretKey := []byte(os.Getenv("JWT_TOKEN"))
	tokenString, err := token.SignedString(secretKey)
	if err != nil {
		utils.Respond_error(w, "Error generating token", http.StatusInternalServerError)
		return
	}

	utils.Respond_json(w, map[string]any{
		"msg":   "logged in successfully",
		"token": tokenString,
		"user":  profile,
	}, http.StatusOK)
}
