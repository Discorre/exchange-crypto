package handlers

import (
	"crypto-api/requestDB"
	"encoding/json"
	"net/http"
	"strconv"
	"strings"
)

type PairResponse struct {
	Pair_id     int `json:"pair_id"`
	Sale_lot_id int `json:"sale_lot_id"`
	Buy_lot_id  int `json:"buy_lot_id"`
}

// Получение информации о парах
func HandlePair(w http.ResponseWriter, r *http.Request) {
	// SQL-запрос для получения всех пар из базы данных
	var getPairsQuery string = "SELECT * FROM pair"

	// Выполняем запрос к базе данных
	dbResponse, err := requestDB.RquestDataBase(getPairsQuery)
	if err != nil {
		return // Если произошла ошибка, завершаем выполнение
	}

	// Разделяем ответ базы данных на строки
	dbRows := strings.Split(strings.TrimSpace(dbResponse), "\n") // Каждая строка соответствует записи

	// Массив для хранения данных о парах
	var pairs []PairResponse

	// Обрабатываем каждую строку ответа
	for _, dbRow := range dbRows {
		// Разделяем строку на отдельные поля
		fields := strings.Split(dbRow, " ")
		if len(fields) < 3 {
			continue // Пропускаем строки, где недостаточно данных
		}

		// Преобразуем данные из строки в нужный формат
		pairID, _ := strconv.Atoi(strings.TrimSpace(fields[0]))   // Идентификатор пары
		saleLotID, _ := strconv.Atoi(strings.TrimSpace(fields[1])) // Идентификатор продаваемого лота
		buyLotID, _ := strconv.Atoi(strings.TrimSpace(fields[2]))  // Идентификатор покупаемого лота

		// Создаем структуру для пары
		pairResponse := PairResponse{
			Pair_id:     pairID,
			Sale_lot_id: saleLotID,
			Buy_lot_id:  buyLotID,
		}

		// Добавляем структуру в массив ответов
		pairs = append(pairs, pairResponse)
	}

	// Устанавливаем заголовок ответа и отправляем данные в формате JSON
	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(pairs)
}

