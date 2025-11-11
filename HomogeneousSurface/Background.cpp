/**
 * Background.cpp
 *
 * Загрузка и отрисовка фонового изображения в формате BMP.
 *
 * Возможности:
 * - Кроссплатформенная загрузка BMP (24/32 бит, несжатый)
 * - Создание мипмапов для качественного масштабирования
 * - Отрисовка текстуры на весь экран без влияния на 3D-сцену
 * - Автоматическая инверсия Y-координаты для корректной ориентации
 */

#include "Background.hpp"
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
// На Windows используем системные структуры BMP из windows.h
#include <windows.h>
#else
// На других платформах (macOS, Linux) определяем структуры BMP вручную
#include <stdint.h>

// #pragma pack(push, 1) — выравнивание структур по 1 байту (без padding)
// Это критично для корректного чтения бинарных файлов BMP
#pragma pack(push, 1)

/**
 * BITMAPFILEHEADER — заголовок BMP-файла (14 байт)
 *
 * Структура:
 * - bfType (2 байта): сигнатура "BM" = 0x4D42
 * - bfSize (4 байта): размер файла в байтах
 * - bfReserved1, bfReserved2 (по 2 байта): зарезервировано (обычно 0)
 * - bfOffBits (4 байта): смещение до начала пиксельных данных
 */
typedef struct {
    uint16_t bfType;         // Сигнатура файла ("BM" = 0x4D42)
    uint32_t bfSize;         // Размер файла в байтах
    uint16_t bfReserved1;    // Зарезервировано (0)
    uint16_t bfReserved2;    // Зарезервировано (0)
    uint32_t bfOffBits;      // Смещение до начала данных изображения
} BITMAPFILEHEADER;

/**
 * BITMAPINFOHEADER — информационный заголовок BMP (40 байт)
 *
 * Структура:
 * - biSize (4 байта): размер этой структуры (40)
 * - biWidth (4 байта): ширина изображения в пикселях
 * - biHeight (4 байта): высота изображения (если >0, то снизу вверх; если <0, то сверху вниз)
 * - biPlanes (2 байта): количество цветовых плоскостей (всегда 1)
 * - biBitCount (2 байта): бит на пиксель (1, 4, 8, 16, 24, 32)
 * - biCompression (4 байта): тип сжатия (0 = BI_RGB = без сжатия)
 * - biSizeImage (4 байта): размер данных изображения в байтах
 * - biXPelsPerMeter, biYPelsPerMeter (по 4 байта): разрешение в пикселях на метр
 * - biClrUsed (4 байта): количество используемых цветов палитры (0 = все)
 * - biClrImportant (4 байта): количество важных цветов (0 = все)
 */
typedef struct {
    uint32_t biSize;           // Размер структуры (40 байт)
    int32_t  biWidth;          // Ширина изображения
    int32_t  biHeight;         // Высота (>0 = снизу вверх, <0 = сверху вниз)
    uint16_t biPlanes;         // Цветовых плоскостей (всегда 1)
    uint16_t biBitCount;       // Бит на пиксель (24 или 32)
    uint32_t biCompression;    // Тип сжатия (0 = нет)
    uint32_t biSizeImage;      // Размер данных изображения
    int32_t  biXPelsPerMeter;  // Горизонтальное разрешение (пикселей/метр)
    int32_t  biYPelsPerMeter;  // Вертикальное разрешение
    uint32_t biClrUsed;        // Использовано цветов палитры
    uint32_t biClrImportant;   // Важных цветов
} BITMAPINFOHEADER;

#pragma pack(pop)  // Восстанавливаем обычное выравнивание

// Константа для несжатого формата BMP
#define BI_RGB 0
#endif

// ============================================================================
// Загрузка BMP-файла
// ============================================================================

/**
 * Загрузка BMP-файла в память
 *
 * Поддерживаемые форматы:
 * - 24-bit RGB (по 8 бит на канал)
 * - 32-bit RGBA (по 8 бит на канал, альфа игнорируется)
 * - Несжатый (BI_RGB)
 *
 * Алгоритм:
 * 1. Открываем файл в бинарном режиме
 * 2. Читаем BITMAPFILEHEADER (14 байт)
 * 3. Проверяем сигнатуру "BM" (0x4D42)
 * 4. Читаем BITMAPINFOHEADER (40 байт)
 * 5. Проверяем формат (несжатый, 24 или 32 бит)
 * 6. Вычисляем параметры строк (stride с выравниванием по 4 байта)
 * 7. Читаем пиксельные данные
 * 8. Конвертируем BGR -> RGB и переворачиваем Y при необходимости
 *
 * @param filename - путь к BMP-файлу
 * @return указатель на RGBImageRec или nullptr при ошибке
 */
RGBImageRec* Background::loadBMP(const char* filename) {
    // Открываем файл в бинарном режиме для чтения
    FILE* fp = fopen(filename, "rb");
    if (!fp) return nullptr;  // файл не найден

    // Структуры для заголовков BMP
    BITMAPFILEHEADER bfh{};  // File header (14 байт)
    BITMAPINFOHEADER bih{};  // Info header (40 байт)

    // ШАГ 1: Чтение file header
    if (fread(&bfh, sizeof(bfh), 1, fp) != 1) {
        fclose(fp);
        return nullptr;  // ошибка чтения
    }

    // ШАГ 2: Проверка сигнатуры "BM" (0x4D42 = 'B' + 'M'<<8)
    if (bfh.bfType != 0x4D42) {
        fclose(fp);
        return nullptr;  // не BMP файл
    }

    // ШАГ 3: Чтение info header
    if (fread(&bih, sizeof(bih), 1, fp) != 1) {
        fclose(fp);
        return nullptr;
    }

    // ШАГ 4: Проверка формата
    // Поддерживаем только несжатый RGB (BI_RGB = 0) с 24 или 32 битами
    if (bih.biCompression != BI_RGB || (bih.biBitCount != 24 && bih.biBitCount != 32)) {
        fclose(fp);
        return nullptr;  // неподдерживаемый формат
    }

    // Проверка корректности размеров
    if (bih.biWidth <= 0 || bih.biHeight == 0) {
        fclose(fp);
        return nullptr;  // некорректные размеры
    }

    // ШАГ 5: Извлекаем параметры изображения
    int width = bih.biWidth;
    int height = std::abs(bih.biHeight);  // высота может быть отрицательной

    // Флаг переворота по Y:
    // - biHeight > 0 => пиксели хранятся снизу вверх (standard BMP)
    // - biHeight < 0 => пиксели хранятся сверху вниз (top-down BMP)
    bool flipY = (bih.biHeight > 0);

    // ШАГ 6: Перемещаемся к началу пиксельных данных
    fseek(fp, bfh.bfOffBits, SEEK_SET);

    // ШАГ 7: Вычисляем параметры строк
    // BMP хранит пиксели построчно с выравниванием каждой строки по 4 байта
    const int bpp = bih.biBitCount / 8;   // байт на пиксель (3 для 24-bit, 4 для 32-bit)
    const int rowRaw = width * bpp;       // количество байт в строке без padding
    const int rowStride = ((rowRaw + 3) / 4) * 4;  // с padding до кратного 4

    // Пример: width=100, bpp=3 => rowRaw=300 => rowStride=300 (делится на 4)
    // Пример: width=101, bpp=3 => rowRaw=303 => rowStride=304 (+ 1 байт padding)

    // ШАГ 8: Читаем все пиксельные данные
    std::vector<unsigned char> raw(rowStride * height);
    if (fread(raw.data(), 1, raw.size(), fp) != raw.size()) {
        fclose(fp);
        return nullptr;  // ошибка чтения
    }
    fclose(fp);

    // ШАГ 9: Выделяем память для RGB данных (без альфа-канала)
    // OpenGL требует формат RGB (3 байта на пиксель)
    unsigned char* rgb = (unsigned char*)malloc(width * height * 3);
    if (!rgb) return nullptr;  // не хватило памяти

    // ШАГ 10: Конвертируем BGR -> RGB и переворачиваем Y при необходимости
    // BMP хранит пиксели в формате BGR (а не RGB!)
    auto copyRow = [&](int srcRow, int dstRow) {
        const unsigned char* src = raw.data() + srcRow * rowStride;
        unsigned char* dst = rgb + dstRow * width * 3;

        for (int x = 0; x < width; ++x) {
            // Читаем BGR из BMP
            unsigned char B = src[x * bpp + 0];  // Blue
            unsigned char G = src[x * bpp + 1];  // Green
            unsigned char R = src[x * bpp + 2];  // Red
            // (4-й байт для 32-bit = альфа, игнорируем)

            // Записываем в RGB для OpenGL
            dst[x * 3 + 0] = R;
            dst[x * 3 + 1] = G;
            dst[x * 3 + 2] = B;
        }
    };

    // Переворачиваем изображение по вертикали, если нужно
    if (flipY) {
        // BMP хранит строки снизу вверх, переворачиваем
        for (int y = 0; y < height; ++y) {
            copyRow(y, height - 1 - y);  // первая строка BMP -> последняя строка RGB
        }
    }
    else {
        // BMP уже сверху вниз, копируем как есть
        for (int y = 0; y < height; ++y) {
            copyRow(y, y);
        }
    }

    // ШАГ 11: Создаём структуру результата
    auto* img = (RGBImageRec*)malloc(sizeof(RGBImageRec));
    img->sizeX = width;
    img->sizeY = height;
    img->data = rgb;
    return img;
}

// ============================================================================
// Методы класса Background
// ============================================================================

/**
 * Конструктор
 */
Background::Background() : mTex(0), mLoaded(false) {}

/**
 * Деструктор - освобождает текстуру OpenGL
 */
Background::~Background() {
    release();
}

/**
 * Загрузка фона из BMP-файла
 *
 * Процесс:
 * 1. Освобождаем предыдущую текстуру (если была)
 * 2. Загружаем BMP в память через loadBMP()
 * 3. Создаём текстуру OpenGL
 * 4. Настраиваем фильтрацию и параметры
 * 5. Генерируем мипмапы для качественного масштабирования
 *
 * OpenGL функции текстур:
 * - glGenTextures() — создать ID текстуры
 * - glBindTexture() — сделать текстуру активной
 * - glPixelStorei(GL_UNPACK_ALIGNMENT, 1) — выравнивание данных по 1 байту
 * - glTexEnvi() — режим применения текстуры (GL_REPLACE = заменить цвет)
 * - glTexParameteri() — параметры фильтрации и обёртки:
 *   - GL_TEXTURE_MIN_FILTER — фильтр при уменьшении (GL_LINEAR_MIPMAP_LINEAR = трилинейная)
 *   - GL_TEXTURE_MAG_FILTER — фильтр при увеличении (GL_LINEAR = билинейная)
 *   - GL_TEXTURE_WRAP_S/T — обёртка текстуры (GL_CLAMP = обрезать края)
 * - gluBuild2DMipmaps() — создать мипмапы (уменьшенные копии текстуры)
 *
 * Мипмапы (mipmaps):
 * Серия уменьшенных копий текстуры (512x512, 256x256, 128x128, ..., 1x1)
 * Используются при отдалении камеры для:
 * - Улучшения качества (уменьшение муара и мерцания)
 * - Увеличения производительности (меньше выборок текстуры)
 *
 * @param relativePath - путь к BMP-файлу
 * @return true при успехе, false при ошибке
 */
bool Background::loadFromPath(const char* relativePath) {
    release();  // освобождаем старую текстуру

    // Загружаем BMP в память
    RGBImageRec* img = loadBMP(relativePath);
    if (!img) return false;  // ошибка загрузки

    // ============================================================
    // Создание текстуры OpenGL
    // ============================================================

    // Генерируем ID текстуры, если ещё не создан
    if (!mTex) glGenTextures(1, &mTex);

    // Делаем текстуру активной
    glBindTexture(GL_TEXTURE_2D, mTex);

    // ============================================================
    // Настройка параметров загрузки
    // ============================================================

    // GL_UNPACK_ALIGNMENT = 1: данные выровнены по 1 байту (плотная упаковка)
    // Это важно для текстур с нестандартной шириной
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // GL_REPLACE: текстура полностью заменяет цвет фрагмента (игнорирует освещение)
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    // ============================================================
    // Настройка фильтрации и обёртки
    // ============================================================

    // MIN_FILTER (при уменьшении): трилинейная фильтрация с мипмапами
    // LINEAR_MIPMAP_LINEAR = линейная интерполяция между двумя ближайшими мипмапами,
    // внутри каждого мипмапа тоже линейная интерполяция
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // MAG_FILTER (при увеличении): билинейная фильтрация
    // LINEAR = линейная интерполяция между соседними пикселями (размытие)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // WRAP_S/T (обёртка по координатам U/V): GL_CLAMP = обрезать края
    // При координатах >1.0 или <0.0 используется граничный пиксель
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    // ============================================================
    // Загрузка данных и создание мипмапов
    // ============================================================

    // gluBuild2DMipmaps — создаёт полный набор мипмапов
    // Автоматически обрабатывает текстуры с размерами не степени двойки (NPOT)
    // Например, 512x341 корректно обработается
    //
    // Параметры:
    // - GL_TEXTURE_2D — тип текстуры
    // - GL_RGB — внутренний формат OpenGL (3 компоненты)
    // - img->sizeX, img->sizeY — размеры
    // - GL_RGB — формат входных данных (Red, Green, Blue)
    // - GL_UNSIGNED_BYTE — тип данных (unsigned char)
    // - img->data — указатель на пиксели
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB,
        img->sizeX, img->sizeY,
        GL_RGB, GL_UNSIGNED_BYTE, img->data);

    // Освобождаем временные данные
    if (img->data) free(img->data);
    free(img);

    mLoaded = true;
    return true;
}

/**
 * Отрисовка фона на весь экран
 *
 * Алгоритм:
 * 1. Сохраняем текущие состояния OpenGL
 * 2. Отключаем тесты глубины, освещение, альфа-тест
 * 3. Включаем текстурирование
 * 4. Переходим в ортогональную проекцию 2D
 * 5. Рисуем текстурированный квад на весь экран
 * 6. Восстанавливаем все состояния
 *
 * OpenGL функции:
 * - glPushMatrix()/glPopMatrix() — сохранить/восстановить текущую матрицу
 * - glLoadIdentity() — загрузить единичную матрицу (сброс трансформаций)
 * - glOrtho(0,1, 0,1, -1,1) — ортогональная проекция [0,1]×[0,1]
 * - glBegin(GL_QUADS) — начать рисовать четырёхугольники
 * - glTexCoord2f(u,v) — задать текстурные координаты для следующей вершины
 * - glVertex2f(x,y) — задать позицию вершины в 2D
 *
 * Координаты текстуры:
 * - (0,0) — левый нижний угол текстуры
 * - (1,0) — правый нижний угол
 * - (1,1) — правый верхний угол
 * - (0,1) — левый верхний угол
 *
 * Инверсия V-координаты:
 * BMP хранит данные «вверх ногами» относительно OpenGL,
 * поэтому мы инвертируем V (меняем 0↔1) при отрисовке
 */
void Background::draw() const {
    if (!mLoaded || !mTex) return;  // нечего рисовать

    // ============================================================
    // Сохранение состояний OpenGL
    // ============================================================
    GLboolean wasDepth = glIsEnabled(GL_DEPTH_TEST);   // тест глубины (Z-буфер)
    GLboolean wasLight = glIsEnabled(GL_LIGHTING);     // освещение
    GLboolean wasAlpha = glIsEnabled(GL_ALPHA_TEST);   // альфа-тест

    // ============================================================
    // Настройка состояний для 2D-отрисовки
    // ============================================================

    // Отключаем Z-буфер (фон всегда позади всего)
    glDisable(GL_DEPTH_TEST);

    // Отключаем освещение (используем цвет текстуры как есть)
    glDisable(GL_LIGHTING);

    // Отключаем альфа-тест (фон непрозрачен)
    glDisable(GL_ALPHA_TEST);

    // Включаем текстурирование
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, mTex);  // активируем нашу текстуру

    // Белый цвет (1,1,1,1) = не модулируем текстуру
    glColor4f(1.f, 1.f, 1.f, 1.f);

    // ============================================================
    // Переход в ортогональную 2D-проекцию
    // ============================================================

    // Сохраняем текущую матрицу MODELVIEW
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();  // сбрасываем все трансформации

    // Сохраняем текущую матрицу PROJECTION
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    // Устанавливаем ортогональную проекцию:
    // X: [0, 1], Y: [0, 1], Z: [-1, 1]
    // Это означает, что координаты вершин в диапазоне [0,1] заполнят весь экран
    glOrtho(0, 1, 0, 1, -1, 1);

    // ============================================================
    // Отрисовка текстурированного квада
    // ============================================================

    // GL_QUADS — режим четырёхугольников (каждые 4 вершины = 1 квад)
    glBegin(GL_QUADS);

    // Вершины квада с текстурными координатами:
    //
    // Позиция (X,Y)    Текстура (U,V)    Описание
    // (0, 0)           (0, 1)            Левый нижний угол (инвертирован V)
    // (1, 0)           (1, 1)            Правый нижний угол
    // (1, 1)           (1, 0)            Правый верхний угол (инвертирован V)
    // (0, 1)           (0, 0)            Левый верхний угол
    //
    // Инверсия V нужна, т.к. BMP хранит строки снизу вверх

    glTexCoord2f(0.f, 1.f); glVertex2f(0.f, 0.f);  // левый нижний
    glTexCoord2f(1.f, 1.f); glVertex2f(1.f, 0.f);  // правый нижний
    glTexCoord2f(1.f, 0.f); glVertex2f(1.f, 1.f);  // правый верхний
    glTexCoord2f(0.f, 0.f); glVertex2f(0.f, 1.f);  // левый верхний

    glEnd();

    // ============================================================
    // Восстановление матриц и состояний
    // ============================================================

    // Восстанавливаем матрицу проекции
    glPopMatrix();

    // Переключаемся обратно на MODELVIEW и восстанавливаем её
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    // Отключаем текстурирование
    glDisable(GL_TEXTURE_2D);

    // Восстанавливаем исходные состояния
    if (wasAlpha) glEnable(GL_ALPHA_TEST);
    if (wasLight) glEnable(GL_LIGHTING);
    if (wasDepth) glEnable(GL_DEPTH_TEST);
}

/**
 * Освобождение ресурсов
 *
 * Удаляет текстуру из видеопамяти OpenGL
 */
void Background::release() {
    if (mTex) {
        glDeleteTextures(1, &mTex);  // освобождаем текстуру в GPU
        mTex = 0;
    }
    mLoaded = false;
}
