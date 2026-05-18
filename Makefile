CC = gcc
CFLAGS = -Wall -Wextra -I Core/Inc/

all: city_manager monitor_reports

city_manager: Core/Src/city_manager.c Core/Src/commands.c
	$(CC) $(CFLAGS) Core/Src/city_manager.c Core/Src/commands.c -o city_manager

monitor_reports: Core/Src/monitor_reports.c
	$(CC) $(CFLAGS) Core/Src/monitor_reports.c -o monitor_reports

clean:
	rm -f city_manager monitor_reports