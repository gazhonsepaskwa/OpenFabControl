package database

import (
	"context"
	"database/sql"
	"fmt"
	"log"
	"os"
	"time"

	_ "github.com/lib/pq" // postgress specific package
)

var Self *sql.DB

func Initdb() {
	pgUser, pgPass, pgHost, pgPort, pgDB := getenv()

	dsn := fmt.Sprintf("postgresql://%s:%s@%s:%s/%s", pgUser, pgPass, pgHost, pgPort, pgDB)
	// disable SSL for local development
	dsn += "?sslmode=disable"

	// connect to database
	var err error
	Self, err = connectWithRetries(dsn, 15, 2*time.Second)
	if err != nil {
		log.Fatalf("failed to connect to db: %v", err)
	}

	// ensure table exists
	if err := ensureTable(); err != nil {
		log.Fatalf("failed to ensure table: %v", err)
	}
}

func getenv() (string, string, string, string, string) {
	pgUser := os.Getenv("POSTGRES_USER")
	pgPass := os.Getenv("POSTGRES_PASSWORD")
	pgHost := os.Getenv("POSTGRES_HOST")
	pgPort := os.Getenv("POSTGRES_PORT")
	pgDB := os.Getenv("POSTGRES_DB")

	return pgUser, pgPass, pgHost, pgPort, pgDB
}

func connectWithRetries(dsn string, maxRetries int, delay time.Duration) (*sql.DB, error) {
	var err error
	for i := 0; i < maxRetries; i++ {
		db, err := sql.Open("postgres", dsn)
		if err != nil {
			log.Printf("db open error: %v", err)
		} else {
			ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
			defer cancel()
			if pingErr := db.PingContext(ctx); pingErr == nil {
				return db, nil
			} else {
				log.Printf("db ping error: %v", pingErr)
				db.Close()
			}
		}
		log.Printf("retrying DB connection in %s... (%d/%d)", delay, i+1, maxRetries)
		time.Sleep(delay)
	}
	return nil, fmt.Errorf("could not connect to DB after %d attempts: last error: %v", maxRetries, err)
}

func ensureTable() error {
	// create machine controler table
	create := `CREATE TABLE IF NOT EXISTS resources (
		id                   SERIAL    PRIMARY KEY,
		uuid                 TEXT      UNIQUE                     NOT NULL,
		type                 TEXT                                 NOT NULL,
		zone                 TEXT                                 NOT NULL,
		name                 TEXT                                 NOT NULL,
		manual               TEXT                                 NOT NULL,
		price_booking_in_eur FLOAT                                NOT NULL,
		price_usage_in_eur   FLOAT                                NOT NULL,
		approved             BOOLEAN                DEFAULT false NOT NULL,
		created_at           TIMESTAMP              WITH TIME ZONE DEFAULT now()
	);` // FLOAT is synonym to double pressision (64 bit float)
	_, err := Self.Exec(create)
	if err != nil {
		return err
	}

	// create the roles table
	create = `CREATE TABLE IF NOT EXISTS roles (
		id         SERIAL      PRIMARY KEY,
		name       VARCHAR(32) UNIQUE      DEFAULT '' NOT NULL,
		created_at TIMESTAMP               WITH TIME ZONE DEFAULT now()
	);`
	_, err = Self.Exec(create)
	if err != nil {
		return err
	}

	// create users table
	create = `CREATE TABLE IF NOT EXISTS users (
		id                  SERIAL       PRIMARY KEY,
		access_key          TEXT                     DEFAULT '' NOT NULL,
		email               VARCHAR(255) UNIQUE      DEFAULT '' NOT NULL,
		password            VARCHAR(255)             DEFAULT '' NOT NULL,
		first_name          VARCHAR(64)              DEFAULT '' NOT NULL,
		last_name           VARCHAR(64)              DEFAULT '' NOT NULL,
		tva                 VARCHAR(16)              DEFAULT '' NOT NULL,
		facturation_address VARCHAR(255)             DEFAULT '' NOT NULL,
		facturation_account VARCHAR(34)              DEFAULT '' NOT NULL,
		activation_code     VARCHAR(32)              DEFAULT '' NOT NULL,
		status              VARCHAR(16)              DEFAULT 'pending' NOT NULL,
		created_at          TIMESTAMP                WITH TIME ZONE DEFAULT now()
	);`
	_, err = Self.Exec(create)
	if err != nil {
		return err
	}

	// create users_roles junction table
	create = `CREATE TABLE IF NOT EXISTS users_roles (
		user_id INTEGER REFERENCES users(id) ON DELETE CASCADE,
		role_id INTEGER REFERENCES roles(id) ON DELETE CASCADE,
		PRIMARY KEY (user_id, role_id)
	);`
	_, err = Self.Exec(create)
	if err != nil {
		return err
	}

	// create sessions table
	create = `CREATE TABLE IF NOT EXISTS sessions (
		id            SERIAL       PRIMARY KEY,
		user_id       INTEGER      NOT NULL REFERENCES users(id) ON DELETE CASCADE,
		resource_uuid TEXT         NOT NULL REFERENCES resources(uuid) ON DELETE CASCADE,
		started_at    TIMESTAMPTZ  NOT NULL,
		ended_at      TIMESTAMPTZ  NOT NULL,
		time_used     INTEGER      NOT NULL DEFAULT 0,
		status        VARCHAR(16)  NOT NULL CHECK (status IN ('planned','progress','done'))
	);`
	_, err = Self.Exec(create)
	if err != nil {
		return err
	}

	return nil
}
