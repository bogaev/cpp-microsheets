# 📋 microSheets

**microSheets** — релизация приложения таблицы типа Google Sheets на языке C++.
Таблица поддерживает ввод и вывод числовых и строковых значений, а также вычисление упрощенного набора математических операций с возможностью ссылок на другие ячейки таблицы.
Приложение поддерживает терминальный и файловый ввод/вывод.

## ⚙️ Системные требования

- C++17 standard
- STL [^1]
- Cmake [^2] minimum version: 3.7
- Java 11 [^3]
- ANTLR [^4]

## 🛠️ Сборка программы в Ubuntu

### <img src="https://www.svgrepo.com/download/184143/java.svg" alt="java" width="30"/> Установка Java

- Установите Java 11 с помощью apt

	```
	sudo apt update
	sudo apt install openjdk-11-jdk
	java -version
	```

- Вы должны увидеть, что установлена версия 11:

	```
	openjdk version "11.0.19" 2023-04-18
	OpenJDK Runtime Environment (build 11.0.19+7-post-Ubuntu-0ubuntu122.04.1)
	OpenJDK 64-Bit Server VM (build 11.0.19+7-post-Ubuntu-0ubuntu122.04.1, mixed mode, sharing)
	```

- Если у вас установлено несколько версий Java, используйте ```update-alternatives``` для установки текущей версии

	```
	sudo update-alternatives --config java
	```

### <img src="https://avatars.githubusercontent.com/u/80584?s=200&v=4" alt="java" width="30"/> Использование ANTLR

- Перед сборкой проекта используется ```../thirdparty/antlr/antlr-4.13.0-complete.jar``` для генерации C++-исходников грамматики языка на основе файла описания ```Formula.g4```
- На этапе сборки конечного приложения необходима runtime-библиотека ANTLR от которой зависят классы грамматики языка
- Оба этапа автоматизированы с помощью cmake и нет необходимости устанавливать ANTLR вручную (см. следующий пункт)

### <img src="https://upload.wikimedia.org/wikipedia/commons/thumb/1/13/Cmake.svg/1200px-Cmake.svg.png" alt="java" width="30"/> Сборка с помощью Cmake

- Установите cmake

	```
	sudo apt install cmake
	```

- Соберите приложение spreadsheet с помощью cmake в директории build:

	```
	cd [path-to-cpp-spreadsheet-main]\
	mkdir build && cd build
	cmake .. && make
	```
	⚠️ Сборка библиотеки ANTLR может занять значительное время.

## 💡 Использование microSheets

- При запуске таблицы без параметров ```./spreadsheet``` команды вводятся пользователем в режиме терминала:

	```
	./spreadsheet
	# Присвоение значения ячейке таблицы
	> SetCell A1 2
	> SetCell A2 3                     
	# Вычисление выражения, основанное на значениях других ячеек
	> SetCell B1 =(A1*3+A2)/5-A1
	# Вывод итогового значения ячейки
	> GetCell B1
	-0.2
	> exit
	```

- Для вывода справочной информации:

	```./spreadsheet --help```

- Для вывода тестовой информации:

	```./spreadsheet --test```

- Для запуска таблицы в режиме ввода/вывода в файл (см. пример далее):

	```./spreadsheet [input_filepath] [output_filepath]```

## 👨‍💻 Пример

- Получим вывод таблицы для следующего набора команд (файл [input](input)):

	```
	# Тестируем присвоение численных значений ячейкам
	SetCell A1 1
	# Ячейки могут ссылаться друг на друга
	SetCell E2 =A1
	SetCell E4 =E2
	# Значение ячеек E2 и E4 должно быть равно значению ячейки A1, т.е. 1
	GetCell E4
	# Изменяем значение исходной ячейки с вычислением выражения
	SetCell A1 =(2+2)*3/4
	# И проверяем, что значения зависимых ячеек также изменились
	GetCell A1
	GetCell E2
	GetCell E4
	# Программа не разрешит данную ссылку, т.к. она является циклической
	SetCell A1 =E4
	# Значение ячейки при этом не изменилось
	GetCell E4
	# Тест ввода и вывода строковых значений
	SetCell A1 "Meow"
	SetCell E2 "Woof"
	GetCell A1
	GetCell E2
	```

- Запускаем таблицу с выводом в файл ```output```:

	```./spreadsheet input output```

- Вывод (файл ```output```):

	```
	1
	2
	3
	3
	3
	Error setting cell value: Found circular dependency!
	3
	"Meow"
	"Woof"
	```
	
<!--
## Примеры
-->

[^1]: https://en.wikipedia.org/wiki/Standard_Template_Library
[^2]: https://cmake.org/download/
[^3]: https://www.java.com/en/download/
[^4]: https://www.antlr.org/download.html