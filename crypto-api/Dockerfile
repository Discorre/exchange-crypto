# Используем базовый образ Alpine с Go
FROM golang:1.22.2-alpine

# Устанавливаем необходимые зависимости
RUN apk add --no-cache git curl build-base

# Создаём рабочую директорию внутри контейнера
WORKDIR /app

# Копируем файлы зависимостей (go.mod и go.sum) и устанавливаем их
COPY go.mod ./

RUN go mod download

# Копируем весь код приложения в контейнер
COPY . ./

# Собираем бинарный файл приложения
RUN go build ./cmd/main.go

# Указываем порт, который будет использовать приложение
EXPOSE 8080
EXPOSE 7432

# Запускаем приложение
CMD ["./main"]