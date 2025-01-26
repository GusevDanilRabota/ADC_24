#include "pico/stdlib.h"
#include "hardware/spi.h"

// Определяем пины для SPI и MISO
#define SPI_MOSI 12
#define SPI_MISO1 14  // MISO для первого АЦП
#define SPI_MISO2 15  // MISO для второго АЦП
#define SPI_MISO3 16  // MISO для третьего АЦП
#define SPI_SCK  13   // Вход SCK

// Настройки SPI
#define SPI_BAUD_RATE 2000000  // Увеличиваем скорость до 2 MHz
#define READ_INTERVAL_MS 1000   // Интервал чтения в миллисекундах

// Команды для CS1237
#define CMD_READ_DATA 0x00  // Команда чтения данных

// Функция для чтения данных с АЦП
void read_all_adcs(spi_inst_t *spi, uint32_t *adc_values) {
    uint8_t command = CMD_READ_DATA;
    uint8_t data[3];  // буфер для 24-битного считывания

    // Запускаем SPI
    spi_write_blocking(spi, &command, 1);

    for (int i = 0; i < 3; i++) {
        gpio_set_dir(SPI_MISO1 + i, GPIO_IN);  // Установка MISO в режим входа

        // Чтение данных, 3 байта (24 бита) с текущей MISO линии
        spi_read_blocking(spi, 0x00, data, 3);

        // Объединяем три байта в одно 24-битное значение
        adc_values[i] = ((uint32_t)data[0] << 16) | ((uint32_t)data[1] << 8) | (uint32_t)data[2];
    }
}

int main() {
    // Инициализация
    stdio_init_all();  // Инициализация USB (Serial)

    // Настройка GPIO для MISO пинов
    for (int i = 0; i < 3; i++) {
        gpio_init(SPI_MISO1 + i);
    }

    // Инициализация SPI
    spi_inst_t *spi = spi0;
    spi_init(spi, SPI_BAUD_RATE);
    spi_set_format(spi, 8, SPI_CPHA_0, SPI_CPOL_0, SPI_MSB_FIRST);

    // Таймер для контроля времени
    absolute_time_t last_read_time = get_absolute_time();

    // Записываем заголовок CSV
    printf("Time,ADC1,ADC2,ADC3\n");

    while (true) {
        // Получаем текущее время
        absolute_time_t current_time = get_absolute_time();
        
        // Проверяем, прошло ли нужное количество времени
        if (absolute_time_diff_us(last_read_time, current_time) >= (READ_INTERVAL_MS * 1000)) {
            // Обновляем время последнего чтения
            last_read_time = current_time;

            // Чтение значений с АЦП
            uint32_t adc_values[3];
            read_all_adcs(spi, adc_values);

            // Получаем текущее время в миллисекундах с начала работы
            uint64_t elapsed_time = to_ms_since_boot(current_time);

            // Форматируем выход в CSV
            printf("%llu,%lu,%lu,%lu\n", elapsed_time, adc_values[0], adc_values[1], adc_values[2]);
        }
    }

    return 0;
}

/*
Главные изменения:
Увеличение скорости SPI: Увеличен baud rate до 2 MHz. Убедитесь, что это значение поддерживается вашим АЦП.
Оптимизация считывания данных: Теперь создали функцию read_all_adcs, которая считывает данные с всех трех АЦП за одно обращение, что позволяет быстрее обрабатывать данные.
Снижение количества операций в цикле: Для настройки GPIO мы используем цикл вместо повторяющегося кода.
*/