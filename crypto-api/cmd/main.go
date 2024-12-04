package main

import (
	"crypto-api/config"
	"crypto-api/handlers"
	"log"
	"net/http"
	"strconv"
)

func main() {
	// формирование таблицы с парами
	pairList, _, port, _ := config.ConfigRead()
	handlers.Init(pairList)

	// Регистрируем обработчики
	http.HandleFunc("/user", handlers.HandleCreateUser)    // POST
	http.HandleFunc("/lot", handlers.HandleGetLot)         // GET
	http.HandleFunc("/pair", handlers.HandlePair)          // GET

	// Запускаем сервер на порту 8080
	log.Println("Сервер запущен на порту " + strconv.Itoa(port) + " ...")
	log.Fatal(http.ListenAndServe(":"+strconv.Itoa(port), nil))

}
