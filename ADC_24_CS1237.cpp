/*
    АЦП CS1237 является 24-разрядным АЦП, который использует интерфейс SPI.
    Давайте адаптируем программу для работы с CS1237.
    Основные шаги останутся прежними, но нам нужно будет учитывать особенности его работы, в том числе управление сигналами питания и чтение данных.
    Вот адаптированная версия программы для считывания данных с CS1237:
*/

#include "pico/stdlib.h"
#include "hardware/spi.h"

// Определяем пины для SPI и CS
#define SPI_MOSI 15
#define SPI_MISO 14
#define SPI_SCK  13
#define SPI_CS   17

// Настройки SPI
#define SPI_BAUD_RATE 1000000  // 1 MHz

// Команды для CS1237
#define CMD_READ_DATA 0x00  // Чтение данных

// Функция для чтения данных с АЦП
uint32_t read_adc(spi_inst_t *spi) {
    uint8_t command = CMD_READ_DATA;
    uint8_t data[3];  // буфер для 24-битного считывания

    // Выбираем АЦП
    gpio_put(SPI_CS, 0);

    // Отправляем команду для чтения данных
    spi_write_blocking(spi, &command, 1);

    // Чтение данных, 3 байта (24 бита)
    spi_read_blocking(spi, 0x00, data, 3);

    // Освобождаем АЦП
    gpio_put(SPI_CS, 1);

    // Объединяем три байта в одно 24-битное значение
    uint32_t received_data = ((uint32_t)data[0] << 16) | ((uint32_t)data[1] << 8) | (uint32_t)data[2];

    return received_data;
}

int main() {
    // Инициализация
    stdio_init_all();

    // Настройка GPIO для CS (чип-селект)
    gpio_init(SPI_CS);
    gpio_set_dir(SPI_CS, GPIO_OUT);
    gpio_put(SPI_CS, 1); // Неактивен

    // Инициализация SPI
    spi_init(spi0, SPI_BAUD_RATE);
    spi_set_format(spi0, 8, SPI_CPHA_0, SPI_CPOL_0, SPI_MSB_FIRST);

    while (true) {
        // Чтение значения с АЦП
        uint32_t adc_value = read_adc(spi0);

        // Вывод значения в последовательный порт
        printf("ADC value: %lu\n", adc_value);

        // Задержка перед следующим чтением
        sleep_ms(1000);
    }

    return 0;
}

/*
Объяснение изменений:
Команды: Мы определили команду для чтения данных по документации CS1237.
Чтение данных: В функции read_adc реализовано чтение с АЦП, и полученные данные объединяются из трех байтов в 24-битное значение.
Использование Chip Select (CS): Убедитесь, что вы правильно управляете выводами для активации и дезактивации АЦП во время чтения данных.
Частота SPI: В зависимости от вашего проекта, вы можете изменить частоту SPI на более высокую или низкую, но 1 MHz обычно является хорошей отправной точкой.

Примечания:
Перед запуском программы убедитесь, что CS1237 правильно подключен к Raspberry Pi Pico и подается необходимое питание.
Вы также можете добавить задержки или дополнительные проверки состояния в зависимости от того, как быстро ваши внешние устройства реагируют на команды.
Убедитесь, что вы установили правильную среду разработки для Raspberry Pi Pico и использовали все необходимые библиотеки.
*/