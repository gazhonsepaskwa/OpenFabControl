package user_handler

import (
	"OpenFabControl/database"
	"OpenFabControl/utils"
	"database/sql"
	"fmt"
	"math/rand"
	"net/http"
	"net/smtp"
	"os"
	"strings"
	"time"
)

// use smtp server to send email
func sendConfirmationEmail(recipientEmail string, verificationLink string) error {
	// SMTP Credentials (from .env)
	smtpHost := os.Getenv("SMTP_HOST")
	smtpPort := os.Getenv("SMTP_PORT")
	smtpUsername := os.Getenv("SMTP_USERNAME")
	smtpPassword := os.Getenv("SMTP_PASSWORD")

	// Construct SMTP authentication
	auth := smtp.PlainAuth("", smtpUsername, smtpPassword, smtpHost)

	// Email message
	from := smtpUsername
	to := []string{recipientEmail}
	subject := "Account Confirmation"
	body := fmt.Sprintf(`
Dear User,

Your account has been created. To activate your account, please click the following link:

%s

Thank you!
`, verificationLink)

	message := strings.Join([]string{
		"From: " + from + "\r\n",
		"To: " + strings.Join(to, ", ") + "\r\n",
		"Subject: " + subject + "\r\n",
		"\r\n",
		body + "\r\n"}, "")

	// Connect to the SMTP server
	addr := smtpHost + ":" + smtpPort
	conn, err := smtp.Dial(addr)
	if err != nil {
		return err
	}
	defer conn.Close()

	// Authenticate
	if err := conn.Auth(auth); err != nil {
		return err
	}

	// Set the sender and recipient
	if err := conn.Mail(from); err != nil {
		return err
	}
	for _, recipient := range to {
		if err := conn.Rcpt(recipient); err != nil {
			return err
		}
	}

	// Send the email body
	w, err := conn.Data()
	if err != nil {
		return err
	}

	_, err = w.Write([]byte(message))
	if err != nil {
		return err
	}

	err = w.Close()
	if err != nil {
		return err
	}

	// Close the connection
	err = conn.Quit()
	if err != nil {
		return err
	}

	return nil
}

// route to create a new User (and send an email to the user then so he can complete the account)
func Create_user(w http.ResponseWriter, r *http.Request) {
	if utils.Reject_all_methode_exept(r, w, http.MethodPost) != nil {
		return
	}

	var payload struct {
		ACCESS_KEY string `json:"access_key"`
		EMAIL      string `json:"email"`
	}

	if utils.Extract_payload_data(r, w, &payload) != nil {
		return
	}

	if !utils.Validate_payload(payload.ACCESS_KEY == "", "access_key cannot be empty", w) {
		return
	}
	if !utils.Validate_payload(payload.EMAIL == "", "email cannot be empty", w) {
		return
	}

	// check if email already registered
	var existing_email string
	err := database.Self.QueryRow(`SELECT email FROM users WHERE email = $1`, payload.EMAIL).Scan(&existing_email)
	if err == nil {
		utils.Respond_error(w, "Email already registered", http.StatusBadRequest)
		return
	} else if err != sql.ErrNoRows {
		utils.Respond_error(w, "internal server error", http.StatusInternalServerError)
		return
	}

	// generate the verif code
	seeder := 0
	p_seeder := &seeder
	// use the address of the int as seed (yeah I know what you think, answer: why not)
	rand.Seed(int64(*p_seeder))
	randomNum := rand.Intn(999999)
	verif_code := fmt.Sprintf("%v-%v", time.Now().UnixMilli(), randomNum)

	// insert user in the DB
	query := `INSERT INTO users (access_key, email, password, activation_code) VALUES ($1, $2, $3, $4) ON CONFLICT (email) DO NOTHING`
	_, err = database.Self.Exec(query, payload.ACCESS_KEY, payload.EMAIL, "UNDEFINED", verif_code)
	if err != nil {
		utils.Respond_error(w, "internal server error", http.StatusInternalServerError)
		return
	}

	// send email to create password
	if err = sendConfirmationEmail(payload.EMAIL, os.Getenv("DOMAIN_NAME") + "/confirm-email?code=" + verif_code); err != nil {
	utils.Respond_error(w, fmt.Sprintf("Error sending the email: %v", err), http.StatusInternalServerError)
	}

	// do NOT leave the confirm link here (test purpose only)
	utils.Respond_json(w, map[string]any{
		"msg":       "user created, email sent",
		"link_sent": os.Getenv("DOMAIN_NAME") + "/confirm-email?code=" + verif_code,
	}, http.StatusCreated)
}
