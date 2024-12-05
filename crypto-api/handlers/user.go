package handlers

import (
	"crypto-api/requestDB"
	"crypto-api/utilities"

	"encoding/json"
	"net/http"

	"github.com/google/uuid"
)

// Структура для запроса создания пользователя
type CreateUserRequest struct {
	Username string `json:"username"`
}

// Структура ответа при создании пользователя
type CreateUserResponse struct {
	Key string `json:"key"`
}

// Функция для создания пользователя
func HandleCreateUser(w http.ResponseWriter, r *http.Request) {
	// Парсинг JSON-запроса от клиента
	var req CreateUserRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		utilities.SendJSONError(w, "Ошибка при разборе JSON", http.StatusBadRequest)
		return
	}

	// Проверка наличия пользователя в таблице
	checkUserQuery := "SELECT user.username FROM user WHERE user.username = '" + req.Username + "'"
	userCheck, err := requestDB.RquestDataBase(checkUserQuery)
	if err != nil {
		utilities.SendJSONError(w, "Ошибка при проверке пользователя", http.StatusInternalServerError)
		return
	}

	// Если есть хотя бы одна строка, значит пользователь уже существует
	if userCheck != "" {
		utilities.SendJSONError(w, "Username занят другим пользователем", http.StatusConflict)
		return
	}

	userKey := uuid.New().String()

	var reqBD string = "INSERT INTO user VALUES ('" + req.Username + "', '" + userKey + "')"

	_, err = requestDB.RquestDataBase(reqBD)
	if err != nil {
		utilities.SendJSONError(w, "Ошибка при создании пользователя", http.StatusInternalServerError)
		return
	}

	// Генерация активов пользователя
	utilities.GenerateMoney(userKey)

	// Формируем и отправляем JSON-ответ клиенту
	utilities.SendJSONResponse(w, CreateUserResponse{Key: userKey}, http.StatusCreated)
}