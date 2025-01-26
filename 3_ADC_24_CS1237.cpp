/*
Для работы с тремя АЦП CS1237 через SPI на Raspberry Pi Pico, вам нужно будет подключить каждый из них через разные линии CS (чип-селект), чтобы вы могли управлять каждым АЦП отдельно.
Я адаптирую предыдущую программу, чтобы учитывать три АЦП, используя три разные линии CS. Предположим, мы подключим их следующим образом:
CS1 подключен к GPIO 17
CS2 подключен к GPIO 18
CS3 подключен к GPIO 19
Вот обновленный код:
*/

#include "pico/stdlib.h"
#include "hardware/spi.h"

// Определяем пины для SPI и CS (чип-селекты)
#define SPI_MOSI 15
#define SPI_MISO 14
#define SPI_SCK  13
#define SPI_CS1  17  // CS для первого АЦП
#define SPI_CS2  18  // CS для второго АЦП
#define SPI_CS3  19  // CS для третьего АЦП

// Настройки SPI
#define SPI_BAUD_RATE 1000000  // 1 MHz

// Команды для CS1237
#define CMD_READ_DATA 0x00  // Чтение данных

// Функция для чтения данных с одного АЦП
uint32_t read_adc(spi_inst_t *spi, uint pin_cs) {
    uint8_t command = CMD_READ_DATA;
    uint8_t data[3];  // буфер для 24-битного считывания

    // Выбираем АЦП
    gpio_put(pin_cs, 0);

    // Отправляем команду для чтения данных
    spi_write_blocking(spi, &command, 1);

    // Чтение данных, 3 байта (24 бита)
    spi_read_blocking(spi, 0x00, data, 3);

    // Освобождаем АЦП
    gpio_put(pin_cs, 1);

    // Объединяем три байта в одно 24-битное значение
    uint32_t received_data = ((uint32_t)data[0] << 16) | ((uint32_t)data[1] << 8) | (uint32_t)data[2];

    return received_data;
}

int main() {
    // Инициализация
    stdio_init_all();

    // Настройка GPIO для CS (чип-селект)
    gpio_init(SPI_CS1);
    gpio_set_dir(SPI_CS1, GPIO_OUT);
    gpio_put(SPI_CS1, 1); // Неактивен

    gpio_init(SPI_CS2);
    gpio_set_dir(SPI_CS2, GPIO_OUT);
    gpio_put(SPI_CS2, 1); // Неактивен

    gpio_init(SPI_CS3);
    gpio_set_dir(SPI_CS3, GPIO_OUT);
    gpio_put(SPI_CS3, 1); // Неактивен

    // Инициализация SPI
    spi_init(spi0, SPI_BAUD_RATE);
    spi_set_format(spi0, 8, SPI_CPHA_0, SPI_CPOL_0, SPI_MSB_FIRST);

    while (true) {
        // Чтение значений с всех трех АЦП
        uint32_t adc_value1 = read_adc(spi0, SPI_CS1);
        uint32_t adc_value2 = read_adc(spi0, SPI_CS2);
        uint32_t adc_value3 = read_adc(spi0, SPI_CS3);

        // Вывод значений в последовательный порт
        printf("ADC1 value: %lu\n", adc_value1);
        printf("ADC2 value: %lu\n", adc_value2);
        printf("ADC3 value: %lu\n", adc_value3);

        // Задержка перед следующим чтением
        sleep_ms(1000);
    }

    return 0;
}

/*
Объяснение изменений:
Три CS линии: Добавлены определения для трех линий CS, чтобы управлять тремя АЦП.
Функция read_adc: Эта функция теперь принимает дополнительный параметр pin_cs, который указывает, какой АЦП нужно активировать.
Чтение из трех АЦП: В основном цикле мы теперь читаем данные из трех АЦП, вызывая функцию read_adc для каждого из них с соответствующими линиями CS.
Вывод значений: Все полученные значения выводятся на последовательный порт.

Не забудьте:
Убедитесь, что все три АЦП правильно подключены к Raspberry Pi Pico.
Проверьте, что все пины находятся в нужных состояниях и что у вас достаточно источников питания для всех устройств.
В зависимости от вашей конфигурации и требования к производительности, вы можете настроить задержку между чтениями.
*/