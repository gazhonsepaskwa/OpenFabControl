package routes

import (
	"OpenFabControl/database"
	"OpenFabControl/model"
	"OpenFabControl/utils"
	"context"
	"database/sql"
	"net/http"
	"os"

	"github.com/golang-jwt/jwt/v5"
)

// middleware to check if a user is authentificated with a JWT token
func auth_middleware(next http.HandlerFunc) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		authHeader := r.Header.Get("Authorization")
		if authHeader == "" {
			utils.Respond_error(w, "Authorization header required", http.StatusUnauthorized)
			return
		}

		// remove "Bearer from token"
		tokenString := authHeader
		if len(authHeader) > 7 && authHeader[:7] == "Bearer " {
			tokenString = authHeader[7:]
		}

		// validate token
		claims := &model.Claims{}
		token, err := jwt.ParseWithClaims(tokenString, claims, func(token *jwt.Token) (interface{}, error) {
			return []byte(os.Getenv("JWT_TOKEN")), nil
		})
		if err != nil || !token.Valid {
			utils.Respond_error(w, "Invalid token", http.StatusUnauthorized)
			return
		}
		if utils.Reject_user_status(w, claims.USERID, []string{"pending", "desactivated"}) != nil {
			utils.Respond_error(w, "Your account is desactivated or pending activation, if you're part of the system, contact the administrator", http.StatusUnauthorized)
			return
		}

		// Add user info to request context
		ctx := context.WithValue(r.Context(), "user_id", claims.USERID)
		ctx = context.WithValue(ctx, "username", claims.EMAIL)

		next(w, r.WithContext(ctx))
	}
}

func require_role(roleName string) func(http.HandlerFunc) http.HandlerFunc {
	return func(next http.HandlerFunc) http.HandlerFunc {
		return func(w http.ResponseWriter, r *http.Request) {
			userIDVal := r.Context().Value("user_id")
			userID, ok := userIDVal.(int)
			if !ok || userID <= 0 {
				utils.Respond_error(w, "Unauthorized", http.StatusUnauthorized)
				return
			}

			var exists int
			err := database.Self.QueryRow(
				`SELECT 1
				FROM users_roles ur
				JOIN roles r ON r.id = ur.role_id
				WHERE ur.user_id = $1 AND r.name = $2`,
				userID,
				roleName,
			).Scan(&exists)
			if err != nil {
				if err == sql.ErrNoRows {
					utils.Respond_error(w, "Forbidden", http.StatusForbidden)
					return
				}
				utils.Respond_error(w, "internal server error", http.StatusInternalServerError)
				return
			}

			next(w, r)
		}
	}
}

func admin_middleware(next http.HandlerFunc) http.HandlerFunc {
	return auth_middleware(require_role("admin")(next))
}
