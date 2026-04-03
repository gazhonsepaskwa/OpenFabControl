package user_handler
// helpers for the response of the login route

import (
	"OpenFabControl/database"
	"OpenFabControl/model"
	"time"
)

type loginUserResponse struct {
	ID                  int          `json:"id"`
	EMAIL               string       `json:"email"`
	ACCESS_KEY          string       `json:"access_key"`
	FIRST_NAME          string       `json:"first_name"`
	LAST_NAME           string       `json:"last_name"`
	TVA                 string       `json:"tva"`
	FACTURATION_ADDRESS string       `json:"facturation_address"`
	FACTURATION_ACCOUNT string       `json:"facturation_account"`
	STATUS              string       `json:"status"`
	CreatedAt           time.Time    `json:"created_at"`
	Roles               []model.Role `json:"roles"`
}

// returns all roles assigned to the user (same rows as get_user_roles).
func fetchRolesForUser(userID int) ([]model.Role, error) {
	query := `SELECT r.id, r.name, r.created_at FROM roles r JOIN users_roles ur ON r.id = ur.role_id WHERE ur.user_id = $1`
	rows, err := database.Self.Query(query, userID)
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	var roles []model.Role
	for rows.Next() {
		var role model.Role
		if err := rows.Scan(&role.ID, &role.NAME, &role.CreatedAt); err != nil {
			return nil, err
		}
		roles = append(roles, role)
	}
	if err := rows.Err(); err != nil {
		return nil, err
	}
	return roles, nil
}

// loads the password hash and profile fields for login.
func fetchUserByEmailForLogin(email string) (passwordHash string, profile loginUserResponse, err error) {
	err = database.Self.QueryRow(
		`SELECT password, id, email, access_key, first_name, last_name, tva, facturation_address, facturation_account, status, created_at FROM users WHERE email = $1`,
		email,
	).Scan(
		&passwordHash,
		&profile.ID,
		&profile.EMAIL,
		&profile.ACCESS_KEY,
		&profile.FIRST_NAME,
		&profile.LAST_NAME,
		&profile.TVA,
		&profile.FACTURATION_ADDRESS,
		&profile.FACTURATION_ACCOUNT,
		&profile.STATUS,
		&profile.CreatedAt,
	)
	if err != nil {
		return "", loginUserResponse{}, err
	}
	return passwordHash, profile, nil
}
