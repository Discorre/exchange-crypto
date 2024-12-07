package handlers

import (
	"crypto-api/requestDB"
	"encoding/json"
	"net/http"
	"fmt"
	"strconv"
	"strings"
)

type LotResponse struct {
	Lot_id int    `json:"lot_id"`
	Name   string `json:"name"`
}

// Получение информации о лотах
func HandleGetLot(w http.ResponseWriter, r *http.Request) {
	// SQL-запрос для получения всех лотов
	var getLotsQuery string = "SELECT * FROM lot"

	// Выполняем запрос к базе данных
	dbResponse, err := requestDB.RquestDataBase(getLotsQuery)
	if err != nil {
		fmt.Printf("Error getting: %v\n", err)
		return // Если ошибка, выходим из функции
	}

	// Разделяем ответ базы данных на строки
	dbRows := strings.Split(strings.TrimSpace(dbResponse), "\n")

	// Массив для хранения данных о лотах
	var lotResponses []LotResponse

	// Обрабатываем каждую строку ответа
	for _, dbRow := range dbRows {
		// Разделяем строку на отдельные поля
		fields := strings.Split(dbRow, " ")
		if len(fields) < 2 {
			continue // Пропускаем строки с недостаточным количеством полей
		}

		// Преобразуем данные из строки в нужный формат
		lotID, _ := strconv.Atoi(strings.TrimSpace(fields[0])) // Идентификатор лота
		lotName := strings.TrimSpace(fields[1])               // Название лота

		// Создаем структуру ответа
		lotResponse := LotResponse{
			Lot_id: lotID,
			Name:   lotName,
		}

		// Добавляем структуру в массив ответов
		lotResponses = append(lotResponses, lotResponse)
	}

	// Устанавливаем заголовок ответа и отправляем данные в формате JSON
	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(lotResponses)
}


