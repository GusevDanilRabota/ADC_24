/*
Использование трех линий MISO (Master In Slave Out) для взаимодействия с несколькими АЦП без линий CS (Chip Select) может быть реализовано, если ваши АЦП поддерживают этот подход. В этом случае вы сможете считывать данные с каждого АЦП одновременно через одну и ту же линию SCK и MOSI, и каждый АЦП должен выводить свои данные на соответствующую линию MISO.
Однако следует отметить, что для правильной работы вам необходимо будет контролировать, когда каждый АЦП отправляет данные. Обычно это достигается с помощью управления сигналами CS, но если это невозможно, вот пример, как это можно сделать, если у вас есть три отдельных линии MISO, которые вы можете использовать:
Давайте предположим, что вы используете следующие пины:
MISO1, MISO2, MISO3 подключены к GPIO 14, 15 и 16 соответственно.
MOSI и SCK подключены к GPIO 13 и 12.
Вот пример программы:
*/

#include "pico/stdlib.h"
#include "hardware/spi.h"

// Определяем пины для SPI и MISO
#define SPI_MOSI 12
#define SPI_MISO1 14  // МISO для первого АЦП
#define SPI_MISO2 15  // МISO для второго АЦП
#define SPI_MISO3 16  // МISO для третьего АЦП
#define SPI_SCK  13   // Вход SCK

// Настройки SPI
#define SPI_BAUD_RATE 1000000  // 1 MHz

// Команды для CS1237
#define CMD_READ_DATA 0x00  // Команда чтения данных

// Функция для чтения данных с АЦП
uint32_t read_adc(spi_inst_t *spi, uint8_t *miso_pins) {
    uint8_t command = CMD_READ_DATA;
    uint8_t data[3];  // буфер для 24-битного считывания

    // Запускаем SPI
    spi_write_blocking(spi, &command, 1);

    // Чтение данных, 3 байта (24 бита) с каждой MISO линии
    for (int i = 0; i < 3; ++i) {
        uint32_t received_data = 0;
        
        // Установка MISO в режим входа для считывания данных
        gpio_set_dir(miso_pins[i], GPIO_IN);
        
        // Реализуем считывание по очереди
        spi_read_blocking(spi, 0x00, data, 3);
        
        // Объединяем три байта в одно 24-битное значение
        received_data = ((uint32_t)data[0] << 16) | ((uint32_t)data[1] << 8) | (uint32_t)data[2];
        
        // Вернем последний считанный ADC
        if (i == 0) return received_data; // Возвращаем данные от первого ADC
        if (i == 1) return received_data; // Возвращаем данные от второго ADC
        if (i == 2) return received_data; // Возвращаем данные от третьего ADC
    }

    return 0; // По умолчанию
}

int main() {
    // Инициализация
    stdio_init_all();

    // Настройка GPIO для MISO пинов
    gpio_init(SPI_MISO1);
    gpio_init(SPI_MISO2);
    gpio_init(SPI_MISO3);

    // Инициализация SPI
    spi_inst_t *spi = spi0;
    spi_init(spi, SPI_BAUD_RATE);
    spi_set_format(spi, 8, SPI_CPHA_0, SPI_CPOL_0, SPI_MSB_FIRST);

    // Устанавливаем пины MISO в режим выхода
    gpio_set_dir(SPI_MISO1, GPIO_IN);
    gpio_set_dir(SPI_MISO2, GPIO_IN);
    gpio_set_dir(SPI_MISO3, GPIO_IN);

    while (true) {
        uint8_t miso_pins[3] = {SPI_MISO1, SPI_MISO2, SPI_MISO3};
        
        // Чтение значений с АЦП
        uint32_t adc_value1 = read_adc(spi, miso_pins);
        uint32_t adc_value2 = read_adc(spi, miso_pins);
        uint32_t adc_value3 = read_adc(spi, miso_pins);

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
Три линии MISO: Определены три разных вывода MISO GPU, которые подключены к каждому АЦП.
Функция read_adc: Реализуется считывание данных с каждой MISO линии по очереди, используя цикл.
Настройка GPIO: Установлены направления для линий MISO как входы.

Важно:
Чтобы использовать три линии MISO без контроля CS, необходимо, чтобы каждый АЦП мог корректно работать в условиях "шаринга" линии.
В большинстве случаев это может вызывать конфликты, так как несколько устройств могут пытаться отправлять данные одновременно.
Обычно рекомендуется использовать сигналы CS для каждого устройства для предотвращения конфликтов.
Если ваша конфигурация требует использования трех линий MISO без линии CS, нужно убедиться,
что это поддерживается вашим конкретным оборудованием и что АЦП может работать в таких условиях.
Если это не поддерживается, рекомендуется придерживаться стандартного использования линий CS.
*/