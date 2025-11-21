all:
	docker compose up -d --build

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
