// pch.h: это предварительно скомпилированный заголовочный файл.
// Перечисленные ниже файлы компилируются только один раз, что ускоряет последующие сборки.
// Это также влияет на работу IntelliSense, включая многие функции просмотра и завершения кода.
// Однако изменение любого из приведенных здесь файлов между операциями сборки приведет к повторной компиляции всех(!) этих файлов.
// Не добавляйте сюда файлы, которые планируете часто изменять, так как в этом случае выигрыша в производительности не будет.

#ifndef PCH_H
#define PCH_H

// Добавьте сюда заголовочные файлы для предварительной компиляции
#define NOMINMAX

#include "framework.h"
#include < windows.h >
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#include <cassert>
#include <iostream>
#include <memory>

#include <map>
#include <string>
#include <mutex>

#include "Poco/Net/StreamSocket.h"
#include "amqpcpp.h"

#include "Consts.h"

using namespace std;
#include "RabbitWorker.h"
#endif //PCH_H
