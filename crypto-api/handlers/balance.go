package handlers

import (
	"crypto-api/requestDB"
	"encoding/json"
	"net/http"
	"strconv"
	"strings"
)

type BalanceResponse struct {
	Lot_id   int     `json:"lot_id"`
	Quantity float64 `json:"quantity"`
}

func HandleGetBalance(w http.ResponseWriter, r *http.Request) {
	// Получаем ключ пользователя из заголовка запроса
	userKey := r.Header.Get("X-USER-KEY")

	// Проверяем наличие заголовка X-USER-KEY
	if userKey == "" {
		http.Error(w, "Unauthorized", http.StatusUnauthorized)
		return
	}

	// SQL-запрос для получения идентификатора пользователя по его ключу
	var getUserQuery string = "SELECT user.user_id, user.key FROM user WHERE user.key = '" + userKey + "'"

	// Выполняем запрос к базе данных
	userData, err := requestDB.RquestDataBase(getUserQuery)
	if err != nil {
		http.Error(w, "User unauthorized", http.StatusUnauthorized)
		return
	}

	// Разделяем ответ для извлечения идентификатора пользователя
	userDataFields := strings.Split(userData, " ")
	if len(userDataFields) < 1 {
		http.Error(w, "User not found", http.StatusUnauthorized)
		return
	}
	userID := userDataFields[0]

	// SQL-запрос для получения баланса пользователя по его идентификатору
	var getBalanceQuery string = "SELECT user_lot.lot_id, user_lot.quantity FROM user_lot WHERE user_lot.user_id = '" + userID + "'"

	// Выполняем запрос к базе данных для получения данных о балансе
	balanceResponse, err2 := requestDB.RquestDataBase(getBalanceQuery)
	if err2 != nil {
		http.Error(w, "Failed to retrieve balance", http.StatusInternalServerError)
		return
	}

	// Разделяем ответ базы данных на строки
	balanceRows := strings.Split(strings.TrimSpace(balanceResponse), "\n")

	// Массив для хранения балансов пользователя
	var balances []BalanceResponse

	// Обрабатываем каждую строку ответа
	for _, balanceRow := range balanceRows {
		// Разделяем строку на отдельные поля
		fields := strings.Split(balanceRow, " ")
		if len(fields) < 2 {
			continue // Пропускаем строки с недостаточным количеством полей
		}

		// Преобразуем данные из строки в нужный формат
		lotID, _ := strconv.Atoi(strings.TrimSpace(fields[0]))   // Идентификатор лота
		quantity, _ := strconv.ParseFloat(strings.TrimSpace(fields[1]), 64) // Количество

		// Создаем структуру баланса
		balance := BalanceResponse{
			Lot_id:   lotID,
			Quantity: quantity,
		}

		// Добавляем структуру в массив балансов
		balances = append(balances, balance)
	}

	// Устанавливаем заголовок ответа и отправляем данные в формате JSON
	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(balances)
}

