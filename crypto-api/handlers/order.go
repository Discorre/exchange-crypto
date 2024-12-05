package handlers

import (
	"crypto-api/orderLogic"
	"crypto-api/requestDB"
	"encoding/json"
	"net/http"
	"strconv"
	"strings"
)

type CreateOrderRequestStruct struct {
	PairID   int     `json:"pair_id"`
	Quantity float64 `json:"quantity"`
	Price    float64 `json:"price"`
	Type     string  `json:"type"`
}

// Структура ответа при создании ордера
type CreateOrderResponseStruct struct {
	OrderID int `json:"order_id"`
}

type GetOrderResponseStruct struct {
	OrderID  int     `json:"order_id"`
	UserID   int     `json:"user_id"`
	PairID   int     `json:"lot_id"`
	Quantity float64 `json:"quantity"`
	Price    float64 `json:"price"`
	Type     string  `json:"type"`
	Closed   string  `json:"closed"`
}

// Структура запроса на удаление ордера
type DeleteOrderStruct struct {
	OrderID int `json:"order_id"`
}

func CreateOrder(w http.ResponseWriter, r *http.Request) {
	// Получаем ключ пользователя из заголовка запроса
	userKey := r.Header.Get("X-USER-KEY")

	// Проверка наличия заголовка X-USER-KEY, проверить есть ли такой пользователь
	if userKey == "" {
		http.Error(w, "Unauthorized", http.StatusUnauthorized)
		return
	}

	// Парсинг JSON-запроса
	var req CreateOrderRequestStruct
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		http.Error(w, "Invalid JSON", http.StatusBadRequest)
		return
	}

	// Проверка наличия ключа пользователя в БД
	var reqUserID string = "SELECT user.user_id FROM user WHERE user.key = '" + userKey + "'"
	userID, err := requestDB.RquestDataBase(reqUserID)
	if err != nil || userID == "" {
		http.Error(w, "User unauthorized", http.StatusUnauthorized)
		return
	}
	userID = userID[:len(userID)-2] // Убираем лишние символы из строки

	// Проверка наличия пары в БД
	var reqPairID string = "SELECT pair.pair_id FROM pair WHERE pair.pair_id = '" + strconv.Itoa(req.PairID) + "'"
	pairID, err1 := requestDB.RquestDataBase(reqPairID)
	if err1 != nil || pairID == "" {
		http.Error(w, "Pair not found", http.StatusNotFound)
		return
	}

	// Списание средств со счета пользователя
	payErr := orderLogic.PayByOrder(userID, req.PairID, req.Quantity, req.Price, req.Type, true)
	if payErr != nil {
		http.Error(w, "Not enough funds", http.StatusPaymentRequired)
		return
	}

	// Поиск подходящего ордера на покупку/продажу, если нашелся, начисляем новые средства
	newQuant, searchError := orderLogic.SearchOrder(userID, req.PairID, req.Type, req.Quantity, req.Price, req.Type)
	if searchError != nil {
		http.Error(w, "Not enough orders", http.StatusNotFound)
		return
	}

	// Создаем ордер
	status := ""
	if newQuant == 0 {
		status = "close"
		newQuant = req.Quantity
	} else if newQuant != req.Quantity {
		// Вносим в базу уже закрытый ордер (точнее его часть)
		var closeOrderQuery string = "INSERT INTO order VALUES ('" + userID + "', '" + strconv.Itoa(req.PairID) + "', '" + strconv.FormatFloat(req.Quantity, 'f', -1, 64) + "', '" + strconv.FormatFloat(req.Price, 'f', -1, 64) + "', '" + req.Type + "', 'close')"
		_, err := requestDB.RquestDataBase(closeOrderQuery)
		if err != nil {
			return
		}
		status = "open"
	} else {
		status = "open"
	}

	// Вставка нового ордера в БД
	var reqBD string = "INSERT INTO order VALUES ('" + userID + "', '" + strconv.Itoa(req.PairID) + "', '" + strconv.FormatFloat(newQuant, 'f', -1, 64) + "', '" + strconv.FormatFloat(req.Price, 'f', -1, 64) + "', '" + req.Type + "', '" + status + "')"
	_, err2 := requestDB.RquestDataBase(reqBD)
	if err2 != nil {
		return
	}

	// Получаем order_id (предполагается, что это последний ордер, добавленный в БД)
	reqBD = "SELECT order.order_id FROM order WHERE order.user_id = '" + userID + "' AND order.closed = '" + status + "'"
	orderIDall, err3 := requestDB.RquestDataBase(reqBD)
	if err3 != nil {
		return
	}
	orderID := strings.Split(orderIDall, " \n")
	resOrderID, _ := strconv.Atoi(orderID[len(orderID)-2])

	// Формируем и отправляем JSON-ответ клиенту
	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(CreateOrderResponseStruct{
		OrderID: resOrderID,
	})
}


func GetOrders(w http.ResponseWriter, r *http.Request){
	reqBD := "SELECT * FROM order" //WHERE order.closed = 'open'"

	// Имитируем вызов базы данных
	response, err := requestDB.RquestDataBase(reqBD)
	if err != nil {
		http.Error(w, "Ошибка запроса к базе данных", http.StatusInternalServerError)
		return
	}

	// Преобразуем ответ базы данных в строки
	rows := strings.Split(strings.TrimSpace(response), "\n") // Разделяем строки

	// Массив для хранения ордеров
	var orders []GetOrderResponseStruct

	// Парсим каждую строку
	for _, row := range rows {
		fields := strings.Split(row, " ")
		if len(fields) < 7 {
			continue // Пропускаем строки с недостаточным количеством полей
		}

		// Преобразуем каждое поле и заполняем структуру
		orderID, _ := strconv.Atoi(strings.TrimSpace(fields[0]))
		userID, _ := strconv.Atoi(strings.TrimSpace(fields[1]))
		pairID, _ := strconv.Atoi(strings.TrimSpace(fields[2]))
		quantity, _ := strconv.ParseFloat(strings.TrimSpace(fields[3]), 64)
		orderType := strings.TrimSpace(fields[5])
		price, _ := strconv.ParseFloat(strings.TrimSpace(fields[4]), 64)
		closed := strings.TrimSpace(fields[6])

		order := GetOrderResponseStruct{
			OrderID:  orderID,
			UserID:   userID,
			PairID:   pairID,
			Quantity: quantity,
			Type:     orderType,
			Price:    price,
			Closed:   closed,
		}

		orders = append(orders, order) // Добавляем ордер в массив
	}

	// Устанавливаем заголовки ответа
	w.Header().Set("Content-Type", "application/json")

	// Кодируем массив ордеров в JSON и отправляем клиенту
	json.NewEncoder(w).Encode(orders)
}

func DeleteOrder(w http.ResponseWriter, r *http.Request) {
	// Получаем ключ пользователя из заголовка запроса
	userKey := r.Header.Get("X-USER-KEY")
	if userKey == "" {
		http.Error(w, "Unauthorized", http.StatusUnauthorized)
		return
	}

	// Проверка наличия ключа пользователя в БД
	var reqUserID string = "SELECT user.user_id FROM user WHERE user.key = '" + userKey + "'"
	userID, err := requestDB.RquestDataBase(reqUserID)
	if err != nil || userID == "" {
		http.Error(w, "User unauthorized", http.StatusUnauthorized)
		return
	}

	// Парсинг запроса
	var req DeleteOrderStruct
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		http.Error(w, "Invalid JSON", http.StatusBadRequest)
		return
	}

	// Проверка является ли пользователь создателем запроса
	userID = userID[:len(userID)-2] // Убираем лишние символы из строки
	var reqUserOrder string = "SELECT * FROM order WHERE order.order_id = '" + strconv.Itoa(req.OrderID) + "' AND order.user_id = '" + userID + "' AND order.closed = 'open'"
	check, err2 := requestDB.RquestDataBase(reqUserOrder)
	if err2 != nil || check == "" {
		http.Error(w, "access error", http.StatusUnauthorized)
		return
	}

	// Разбиваем результат запроса на поля
	balanceFields := strings.Split(check, " ")
	if len(balanceFields) < 7 {
		return
	}

	// Удаляем ордер из БД
	var reqBD string = "DELETE FROM order WHERE order.order_id = '" + strconv.Itoa(req.OrderID) + "'"
	_, err3 := requestDB.RquestDataBase(reqBD)
	if err3 != nil {
		return
	}

	// Возвращаем деньги обратно на счет пользователю
	floatQuant, _ := strconv.ParseFloat(strings.TrimSpace(balanceFields[3]), 64)
	floatPrice, _ := strconv.ParseFloat(strings.TrimSpace(balanceFields[4]), 64)
	num, _ := strconv.Atoi(balanceFields[2])
	payErr := orderLogic.PayByOrder(userID, num, floatQuant, floatPrice, balanceFields[5], false)
	if payErr != nil {
		http.Error(w, "Not enough funds", http.StatusPaymentRequired)
		return
	}

	// Формируем и отправляем JSON-ответ клиенту
	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(DeleteOrderStruct{
		OrderID: req.OrderID,
	})
}


