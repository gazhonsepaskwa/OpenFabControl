COMPOSE_FILE = docker-compose.yml
COMPOSE_COMMAND = docker-compose

COLIMA_START_OPTIONS = start

# Targets
.PHONY: up down ps logs

up:
	$(COMPOSE_COMMAND) up --build

down:
	$(COMPOSE_COMMAND) down

ps:
	$(COMPOSE_COMMAND) ps

logs:
	$(COMPOSE_COMMAND) logs -f $(SERVICE_NAME)

# Default target
all: up