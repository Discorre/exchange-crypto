package handlers

import (
	"crypto-api/requestDB"
	"crypto-api/utilities"
	"fmt"

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
		fmt.Println("Ошибка при разборе JSON: ", err)
		return
	}

	// Проверка наличия пользователя в таблице
	checkUserQuery := "SELECT user.username FROM user WHERE user.username = '" + req.Username + "'"
	userCheck, err := requestDB.RquestDataBase(checkUserQuery)
	if err != nil {
		utilities.SendJSONError(w, "Ошибка при проверке пользователя", http.StatusInternalServerError)
		fmt.Println("Ошибка при проверке пользователя: ", err)
		return
	}

	// Если есть хотя бы одна строка, значит пользователь уже существует
	if userCheck != "" {
		utilities.SendJSONError(w, "Username занят другим пользователем", http.StatusConflict)
		fmt.Println("Username " + req.Username + " already exists")
		return
	}

	userKey := uuid.New().String()

	var reqBD string = "INSERT INTO user VALUES ('" + req.Username + "', '" + userKey + "')"

	_, err = requestDB.RquestDataBase(reqBD)
	if err != nil {
		utilities.SendJSONError(w, "Ошибка при создании пользователя", http.StatusInternalServerError)
		fmt.Println("Ошибка при создании пользователя: ", err)
		return
	}

	// Генерация активов пользователя
	utilities.GenerateMoney(userKey)

	fmt.Println("Пользователь " + req.Username + " создан успешно")

	// Формируем и отправляем JSON-ответ клиенту
	utilities.SendJSONResponse(w, CreateUserResponse{Key: userKey}, http.StatusCreated)
}