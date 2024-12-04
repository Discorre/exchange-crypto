# Используем минимальный базовый образ с поддержкой C++
FROM debian:bullseye-slim

# Устанавливаем необходимые зависимости для сборки проекта
RUN apt-get update && apt-get install -y --no-install-recommends \
    g++ \
    make \
    #libboost-all-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Указываем рабочую директорию
WORKDIR /app

# Копируем файлы проекта в контейнер
COPY . .

# Собираем проект
RUN make rebuild

# Открываем порт для взаимодействия по TCP
EXPOSE 7432

# Команда запуска приложения
CMD ["./Database"]
