/*
    Для чтения данных с 24-разрядного аналого-цифрового преобразователя (АЦП) с использованием Raspberry Pi Pico,
    вы можете использовать библиотеку SPI для взаимодействия с устройствами, поддерживающими протокол SPI.
    В качестве примера, давайте предположим, что вы используете АЦП на базе интерфейса SPI, такой как ADS1256.
    Вот пример программы на C++, которая считывает данные с 24-разрядного АЦП и выводит их на последовательный порт:
*/

#include "pico/stdlib.h"
#include "hardware/spi.h"

// Определяем пины для SPI
#define SPI_MOSI 15
#define SPI_MISO 14
#define SPI_SCK  13
#define SPI_CS   17

// Настройки SPI
#define SPI_BAUD_RATE 1000000  // 1 MHz

// Функция для чтения данных с АЦП
uint32_t read_adc(spi_inst_t *spi, uint32_t *data) {
    uint32_t received_data = 0;
    uint8_t command = 0x01; // Команда для начала чтения данных

    // Выбираем АЦП
    gpio_put(SPI_CS, 0);

    // Предаем команду для начала чтения
    spi_write_blocking(spi, &command, 1);

    // Чтение данных, 3 байта (24 бита)
    uint8_t buffer[3];
    spi_read_blocking(spi, 0x00, buffer, 3);

    // Записываем полученные данные в переменную
    received_data = (static_cast<uint32_t>(buffer[0]) << 16) |
                    (static_cast<uint32_t>(buffer[1]) << 8) |
                    (static_cast<uint32_t>(buffer[2]));

    // Освобождаем АЦП
    gpio_put(SPI_CS, 1);

    *data = received_data;
    return received_data;
}

int main() {
    // Инициализация
    stdio_init_all();

    // Настройка GPIO для SPI
    gpio_init(SPI_CS);
    gpio_set_dir(SPI_CS, GPIO_OUT);
    gpio_put(SPI_CS, 1); // Неактивен

    // Инициализация SPI
    spi_init(spi0, SPI_BAUD_RATE);
    spi_set_format(spi0, 8, SPI_CPHA_0, SPI_CPOL_0, SPI_MSB_FIRST);

    while (true) {
        uint32_t adc_value;

        // Чтение значения с АЦП
        read_adc(spi0, &adc_value);

        // Вывод значения в последовательный порт
        printf("ADC value: %lu\n", adc_value);

        // Задержка перед следующим чтением
        sleep_ms(1000);
    }

    return 0;
}