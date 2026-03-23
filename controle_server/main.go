package main

import (
	"OpenFabControl/database"
	"OpenFabControl/routes"
	"log"
	"net/http"
	"os"
)

func main() {
	database.Initdb()
	routes.Setup_routes()

	port := os.Getenv("CONTROLE_SERVER_PORT")
	if port == "" {
		port = "3000"
	}

	log.Printf("controle_server listening on :%s", port)
	if err := http.ListenAndServe(":"+port, nil); err != nil {
		log.Fatalf("server stopped: %v", err)
	}
}
