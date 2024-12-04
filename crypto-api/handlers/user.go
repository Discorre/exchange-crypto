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

// Генерация активов пользователей
func assetGen(userKey string) {
	// SQL-запрос для получения идентификатора пользователя по его ключу
	var getUserIDQuery string = "SELECT user.user_id FROM user WHERE user.key = '" + userKey + "'"

	// Выполняем запрос к базе данных для получения идентификатора пользователя
	userIDResponse, err := requestDB.RquestDataBase(getUserIDQuery)
	if err != nil {
		return
	}

	// Убираем лишние символы из ответа базы данных
	userID := userIDResponse[:len(userIDResponse)-2]

	// Считываем конфигурацию (список лотов, IP базы данных, порты)
	lotNames, _, _, _ := config.ConfigRead()

	// Обрабатываем каждый лот из конфигурации
	for _, lotName := range lotNames {
		// SQL-запрос для получения идентификатора лота по его названию
		var getLotIDQuery string = "SELECT lot.lot_id FROM lot WHERE lot.name = '" + lotName + "'"

		// Выполняем запрос к базе данных для получения идентификатора лота
		lotIDResponse, err := requestDB.RquestDataBase(getLotIDQuery)
		if err != nil {
			return
		}

		// Убираем лишние символы из ответа базы данных
		lotID := lotIDResponse[:len(lotIDResponse)-2]

		// SQL-запрос для вставки данных в таблицу `user_lot`
		var insertUserLotQuery string = "INSERT INTO user_lot VALUES ('" + userID + "', '" + lotID + "', '1000')"

		// Выполняем запрос к базе данных для добавления записи
		_, insertErr := requestDB.RquestDataBase(insertUserLotQuery)
		if insertErr != nil {
			return // Завершаем выполнение при ошибке
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

	// Проверка наличия пользователя в таблице
	checkUserQuery := "SELECT user.username FROM user WHERE user.username = '" + req.Username + "'"
	userCheck, err := requestDB.RquestDataBase(checkUserQuery)
	if err != nil {
		http.Error(w, "Ошибка при проверке пользователя", http.StatusInternalServerError)
		return
	}

	// Если есть хотя бы одна строка, значит пользователь уже существует
	if userCheck != "" {
		http.Error(w, userCheck, http.StatusConflict)
		return
	}

	http.Error(w, userCheck, http.StatusConflict)

	userKey := uuid.New().String()

	var reqBD string = "INSERT INTO user VALUES ('" + req.Username + "', '" + userKey + "')"

	_, err = requestDB.RquestDataBase(reqBD)

	if err != nil {
		return
	}
	// генерация активов пользователя
	assetGen(userKey)

	// Формируем и отправляем JSON-ответ клиенту
	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(CreateUserResponse{Key: string(userKey)})
}
