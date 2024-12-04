package handlers

import (
	"crypto-api/config"
	"crypto-api/requestDB"
	"encoding/json"
	"fmt"
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

// Генерация уникального ключа для пользователя
func assetGen(userKey string) {
	var reqBDcheck string = "SELECT user.user_id FROM user WHERE user.key = '" + userKey + "'"
	response, err := requestDB.RquestDataBase(reqBDcheck)
	if err != nil {
		return
	}
	response = response[:len(response)-2]
	lots, _, _, _ := config.ConfigRead()
	for i := 0; i < len(lots); i++ {
		var reqBDsearch string = "SELECT lot.lot_id FROM lot WHERE lot.name = '" + lots[i] + "'"

		lotID, err2 := requestDB.RquestDataBase(reqBDsearch)
		if err2 != nil {
			return
		}
		lotID = lotID[:len(lotID)-2]
		var reqBD string = "INSERT INTO user_lot VALUES ('" + response + "', '" + lotID + "', '1000')"
		_, err3 := requestDB.RquestDataBase(reqBD)
		if err3 != nil {
			return
		}
	}
}

// Функция для создания пользователя
func HandleCreateUser(w http.ResponseWriter, r *http.Request) {
	// Парсинг JSON-запроса от клиента
	var req CreateUserRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		http.Error(w, "Ошибка при разборе JSON", http.StatusBadRequest)
		return
	}
	fmt.Println(req)

	userKey := uuid.New().String()

	var reqBD string = "INSERT INTO user VALUES ('" + req.Username + "', '" + userKey + "')"

	_, err := requestDB.RquestDataBase(reqBD)
	if err != nil {
		return
	}
	// генерация активов пользователя
	assetGen(userKey)

	// Формируем и отправляем JSON-ответ клиенту
	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(CreateUserResponse{Key: string(userKey)})
}
