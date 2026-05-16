all:
	docker compose up -d --build

up:
	docker compose up -d

stop:
	docker compose stop

down:
	docker compose down

restart:
	docker compose restart

fclean:
	docker compose down --volumes --rmi all --remove-orphans

re:
	make fclean
	make up

# ------------------------------------------------------------------ #
# Conquest Map (visualisation des bannières 400010 + waypoints)
#   - server: Node + Express + mysql2 → :4317
#   - client: React + nginx          → http://localhost:5317
# ------------------------------------------------------------------ #

CONQUEST_MAP_COMPOSE := docker compose -f tools/conquest-map/docker-compose.yml

conquest-map:
	$(CONQUEST_MAP_COMPOSE) up -d --build
	@echo ""
	@echo "  ➜  Conquest Map prêt :  http://localhost:5317"
	@echo "  ➜  API :                http://localhost:4317/api/health"

conquest-map-up:
	$(CONQUEST_MAP_COMPOSE) up -d

conquest-map-stop:
	$(CONQUEST_MAP_COMPOSE) stop

conquest-map-down:
	$(CONQUEST_MAP_COMPOSE) down

conquest-map-logs:
	$(CONQUEST_MAP_COMPOSE) logs -f --tail=100

conquest-map-re:
	$(CONQUEST_MAP_COMPOSE) down
	$(CONQUEST_MAP_COMPOSE) up -d --build

.PHONY: all up stop down restart fclean re \
	conquest-map conquest-map-up conquest-map-stop \
	conquest-map-down conquest-map-logs conquest-map-re
