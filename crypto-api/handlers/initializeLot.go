package handlers

import (
	"crypto-api/requestDB"
	"fmt"
)

// добавление лотов в таблицу lot и pair
func Init(lotNames []string) {
	// Проверяем и добавляем отсутствующие лоты в базу данных
	for index := 0; index < len(lotNames); index++ {
		// SQL-запрос для проверки наличия лота в базе данных
		var checkLotQuery string = "SELECT * FROM lot WHERE lot.name = '" + lotNames[index] + "'"
		dbResponse, err := requestDB.RquestDataBase(checkLotQuery)
		if err != nil {
			return // Если ошибка, выходим из функции
		} else if dbResponse == "" {
			// SQL-запрос для вставки нового лота, если его нет
			var insertLotQuery string = "INSERT INTO lot VALUES ('" + lotNames[index] + "')"
			_, insertErr := requestDB.RquestDataBase(insertLotQuery)
			if insertErr != nil {
				return // Если ошибка при добавлении, выходим из функции
			}
		}
	}

	// Массив для хранения идентификаторов всех лотов
	var lotIDs []string

	// Получаем идентификаторы лотов для всех имен
	for index := 0; index < len(lotNames); index++ {
		// SQL-запрос для получения идентификатора лота
		var getLotIDQuery string = "SELECT lot.lot_id FROM lot WHERE lot.name = '" + lotNames[index] + "'"
		dbResponse, err := requestDB.RquestDataBase(getLotIDQuery)
		if err != nil {
			return // Если ошибка, выходим из функции
		}

		// Удаляем лишние символы из ответа базы данных
		dbResponse = dbResponse[:len(dbResponse)-2]
		lotIDs = append(lotIDs, dbResponse) // Добавляем идентификатор в массив
	}

	// Формируем пустые пары
	for index := 0; index < len(lotIDs); index++ {
		fmt.Println() // Заглушка, здесь можно добавить вывод или другие действия
	}

	// Проверяем существование пар и добавляем отсутствующие пары
	for firstIndex := 0; firstIndex < len(lotIDs); firstIndex++ {
		for secondIndex := firstIndex + 1; secondIndex < len(lotIDs); secondIndex++ {
			// SQL-запрос для проверки существования пары в базе данных
			var checkPairQuery string = "SELECT * FROM pair WHERE pair.first_lot_id = '" + lotIDs[firstIndex] + "' AND pair.second_lot_id = '" + lotIDs[secondIndex] + "'"
			dbResponse, err := requestDB.RquestDataBase(checkPairQuery)
			if err != nil {
				return // Если ошибка, выходим из функции
			} else if dbResponse == "" {
				// SQL-запрос для вставки новой пары, если она отсутствует
				var insertPairQuery string = "INSERT INTO pair VALUES ('" + lotIDs[firstIndex] + "', '" + lotIDs[secondIndex] + "')"
				_, insertErr := requestDB.RquestDataBase(insertPairQuery)
				if insertErr != nil {
					return // Если ошибка при добавлении, выходим из функции
				}
			}
		}
	}
}

