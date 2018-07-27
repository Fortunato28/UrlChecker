# Описание:
Программа опрашивает сервера и измеряет время отклика.

## Описание модулей:
В функции main парсятся аргументы командной строки и конфигурационного файла.  
Класс Controller запускает многопоточный опрос сайтов, выводит итоговый результат в файл.  
Класс UrlWrapper опрашивает конктреный сервер несколько раз с заданной задержкой.  
Файлы url_parcer представляют собой готовый url-парсер


# Сборка:
1. git clone https://github.com/Fortunato28/UrlChecker.git
2. cd UrlChecker
3. cmake CMakeList.txt
4. make

# Пример использования:
./UrlChecker -i testConfig.txt -n 3 -t 5 -o output.txt

# Пример конфигурационного файла:
testConfig.txt:
url = https://www.google.ru/  
url = https://www.yandex.ru/
