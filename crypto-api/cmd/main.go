package main

import (
	"crypto-api/config"
	"crypto-api/handlers"
	"crypto-api/utilities"
	"log"
	"net/http"
	"strconv"

	"github.com/gorilla/mux"
)

func main() {
	// формирование таблицы с парами
	pairList, _, port, _ := config.ConfigRead()
	utilities.InitLots(pairList)

	r := mux.NewRouter()
	
	// Регистрируем обработчики
	r.HandleFunc("/user", handlers.HandleCreateUser).Methods("POST")
	r.HandleFunc("/lot", handlers.HandleGetLot).Methods("GET")
	r.HandleFunc("/pair", handlers.HandlePair).Methods("GET")
	r.HandleFunc("/balance", handlers.HandleGetBalance).Methods("GET")

	r.HandleFunc("/order", handlers.CreateOrder).Methods("POST")
	r.HandleFunc("/order", handlers.GetOrders).Methods("GET")
	r.HandleFunc("/allorder", handlers.GetAllOrders).Methods("GET")
	r.HandleFunc("/order", handlers.DeleteOrder).Methods("DELETE")

	// Запускаем сервер на порту 8080
	http.ListenAndServe(":8080", r)
	log.Println("Сервер запущен на порту " + strconv.Itoa(port) + " ...")
	log.Fatal(http.ListenAndServe(":"+strconv.Itoa(port), nil))

}
