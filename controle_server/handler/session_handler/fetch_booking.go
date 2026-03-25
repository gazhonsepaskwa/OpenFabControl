package session_handler

import (
	"OpenFabControl/database"
	"OpenFabControl/model"
	"OpenFabControl/utils"
	"fmt"
	"net/http"
	"os"
	"strconv"
	"strings"
	"time"

	"github.com/golang-jwt/jwt/v5"
)

type fetchBookingPayload struct {
	StartedAt        string   `json:"started_at"`
	EndedAt          string   `json:"ended_at"`
	ResourceUUIDList []string `json:"resource_uuid_list"`
	UserIDList       []int    `json:"user_id_list"`
}

// Route to fetch bookings with filters (timeframe required, resource optional, user optional for admin only).
func Fetch_booking(w http.ResponseWriter, r *http.Request) {
	if utils.Reject_all_methode_exept(r, w, http.MethodPost) != nil {
		return
	}

	var payload fetchBookingPayload
	if utils.Extract_payload_data(r, w, &payload) != nil {
		return
	}

	startedAt, endedAt, ok := validateFetchPayload(payload, w)
	if !ok {
		return
	}

	// user_id_list is accepted only on admin route.
	filterUserList := []int{}
	if strings.HasPrefix(r.URL.Path, "/web-admin-api/") {
		filterUserList = payload.UserIDList
	}

	sessions, err := fetchSessionsByFilters(startedAt, endedAt, payload.ResourceUUIDList, filterUserList)
	if err != nil {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}

	utils.Respond_json(w, map[string]any{
		"sessions": sessions,
	}, http.StatusOK)
}

// Route to fetch authenticated user's bookings with filters (timeframe required, resource optional).
func Fetch_my_booking(w http.ResponseWriter, r *http.Request) {
	if utils.Reject_all_methode_exept(r, w, http.MethodPost) != nil {
		return
	}

	var payload fetchBookingPayload
	if utils.Extract_payload_data(r, w, &payload) != nil {
		return
	}

	startedAt, endedAt, ok := validateFetchPayload(payload, w)
	if !ok {
		return
	}

	userID, err := getUserIDFromToken(r)
	if err != nil {
		utils.Respond_error(w, "Invalid token", http.StatusUnauthorized)
		return
	}

	sessions, err := fetchSessionsByFilters(startedAt, endedAt, payload.ResourceUUIDList, []int{userID})
	if err != nil {
		utils.Respond_error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}

	utils.Respond_json(w, map[string]any{
		"sessions": sessions,
	}, http.StatusOK)
}

func validateFetchPayload(payload fetchBookingPayload, w http.ResponseWriter) (time.Time, time.Time, bool) {
	if !utils.Validate_payload(payload.StartedAt == "", "started_at cannot be empty", w) {
		return time.Time{}, time.Time{}, false
	}
	if !utils.Validate_payload(payload.EndedAt == "", "ended_at cannot be empty", w) {
		return time.Time{}, time.Time{}, false
	}

	startedAt, err := time.Parse(time.RFC3339, payload.StartedAt)
	if err != nil {
		utils.Respond_error(w, "invalid payload: started_at must be RFC3339 format", http.StatusBadRequest)
		return time.Time{}, time.Time{}, false
	}

	endedAt, err := time.Parse(time.RFC3339, payload.EndedAt)
	if err != nil {
		utils.Respond_error(w, "invalid payload: ended_at must be RFC3339 format", http.StatusBadRequest)
		return time.Time{}, time.Time{}, false
	}

	if !utils.Validate_payload(!endedAt.After(startedAt), "ended_at must be after started_at", w) {
		return time.Time{}, time.Time{}, false
	}

	return startedAt, endedAt, true
}

func fetchSessionsByFilters(startedAt time.Time, endedAt time.Time, resourceUUIDList []string, userIDList []int) ([]model.Session, error) {
	query := `SELECT id, user_id, resource_uuid, started_at, ended_at, time_used, status
		FROM sessions
		WHERE started_at < $1 AND ended_at > $2`
	params := []any{endedAt, startedAt}
	paramIndex := 3

	if len(resourceUUIDList) > 0 {
		query += " AND resource_uuid IN (" + buildPlaceholders(paramIndex, len(resourceUUIDList)) + ")"
		for _, resourceUUID := range resourceUUIDList {
			params = append(params, resourceUUID)
		}
		paramIndex += len(resourceUUIDList)
	}

	if len(userIDList) > 0 {
		query += " AND user_id IN (" + buildPlaceholders(paramIndex, len(userIDList)) + ")"
		for _, userID := range userIDList {
			params = append(params, userID)
		}
	}

	query += " ORDER BY started_at ASC"

	rows, err := database.Self.Query(query, params...)
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	var sessions []model.Session
	for rows.Next() {
		var session model.Session
		if err := rows.Scan(&session.ID, &session.UserID, &session.ResourceUUID, &session.StartedAt, &session.EndedAt, &session.TimeUsed, &session.Status); err != nil {
			return nil, err
		}
		sessions = append(sessions, session)
	}

	if err := rows.Err(); err != nil {
		return nil, err
	}

	return sessions, nil
}

func buildPlaceholders(startIndex int, count int) string {
	placeholders := make([]string, 0, count)
	for i := 0; i < count; i++ {
		placeholders = append(placeholders, "$"+strconv.Itoa(startIndex+i))
	}
	return strings.Join(placeholders, ",")
}

func getUserIDFromToken(r *http.Request) (int, error) {
	authHeader := r.Header.Get("Authorization")
	if authHeader == "" {
		return 0, fmt.Errorf("authorization header required")
	}

	tokenString := authHeader
	if len(authHeader) > 7 && authHeader[:7] == "Bearer " {
		tokenString = authHeader[7:]
	}

	claims := &model.Claims{}
	token, err := jwt.ParseWithClaims(tokenString, claims, func(token *jwt.Token) (interface{}, error) {
		return []byte(os.Getenv("JWT_TOKEN")), nil
	})
	if err != nil || !token.Valid {
		return 0, fmt.Errorf("invalid token")
	}

	return claims.USERID, nil
}
