# Part of software for memory testing
Данное программное обеспечение является частью программно-аппаратного комплекса для тестирования микросхем памяти. Оно предназначено для микроконтроллера PIC18F4520. Для работы программного обеспечения необходимо подключить библиотеки C_String и UART, данные библиотеки являются системными в mikroE Libraries при работе в MikroC PRO for PIC. В цикле while микроконтроллер ждёт получения кодовых символов, для запуска тестирования микросхемы памяти по протоколу I2C. При функциональном контроле существует 5 возможных маршевых тестов (March tests) микросхемы памяти на выбор. При тестировании статических параметров используется также схема реле, подключенных через матрицы из транзисторов дарлингтона к микроконтроллеру.
