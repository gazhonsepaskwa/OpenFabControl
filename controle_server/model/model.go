package model

import (
	"time"

	"github.com/golang-jwt/jwt/v5"
)

///////////////
// Data Base //
///////////////

type Machine_controller struct {
	ID                   int       `json:"id"`
	UUID                 string    `json:"uuid"`
	TYPE                 string    `json:"type"`
	ZONE                 string    `json:"zone"`
	NAME                 string    `json:"name"`
	MANUAL               string    `json:"manual"`
	PRICE_BOOKING_IN_EUR float64   `json:"price_booking_in_eur"`
	PRICE_USAGE_IN_EUR   float64   `json:"price_usage_in_eur"`
	Approved             bool      `json:"approved"`
	CreatedAt            time.Time `json:"created_at"`
}

type User struct {
	ID                  int       `json:"id"`
	EMAIL			   	string    `json:"email"`
	ACCESS_KEY          string    `json:"access_key"`
	PASSWORD            string    `json:"password"`
	FIRST_NAME          string    `json:"first_name"`
	LAST_NAME           string    `json:"last_name"`
	TVA                 string    `json:"tva"`
	FACTURATION_ADDRESS string    `json:"facturation_address"`
	FACTURATION_ACCOUNT string    `json:"facturation_account"`
	ACTIVATION_CODE     string    `json:"activation_code"`
	STATUS              string    `json:"status"`
	CreatedAt           time.Time `json:"created_at"`
}

type Role struct {
	ID        int       `json:"id"`
	NAME      string    `json:"name"`
	CreatedAt time.Time `json:"created_at"`
}

//////////////
// jwt auth //
//////////////

type Claims struct {
	USERID int    `json:"user_id"`
	EMAIL  string `json:"email"`
	jwt.RegisteredClaims
}
