/*
Если вы хотите переписать код без использования задержек (например, sleep_ms), вы можете просто использовать метод опроса, 
чтобы постоянно считывать данные с АЦП и выводить их на последовательный порт. Вместо задержки вы можете проверять, прошло ли время с последнего считывания,
и только тогда выполнять новый опрос.
Для этого вам потребуется использовать таймер, чтобы отслеживать интервал времени, который вы хотите установить между считываниями.
Например, можно использовать absolute_time_t для управления временными интервалами. Затем вы можете выполнять считывание только тогда,
когда прошло заданное количество миллисекунд.
Вот как может выглядеть обновленная программа:
*/

#include "pico/stdlib.h"
#include "hardware/spi.h"

// Определяем пины для SPI и MISO
#define SPI_MOSI 12
#define SPI_MISO1 14  // МISO для первого АЦП
#define SPI_MISO2 15  // MISO для второго АЦП
#define SPI_MISO3 16  // MISO для третьего АЦП
#define SPI_SCK  13   // Вход SCK

// Настройки SPI
#define SPI_BAUD_RATE 1000000  // 1 MHz
#define READ_INTERVAL_MS 1000   // Интервал чтения в миллисекундах

// Команды для CS1237
#define CMD_READ_DATA 0x00  // Команда чтения данных

// Функция для чтения данных с АЦП
uint32_t read_adc(spi_inst_t *spi, uint8_t miso_pin) {
    uint8_t command = CMD_READ_DATA;
    uint8_t data[3];  // буфер для 24-битного считывания

    // Запускаем SPI
    spi_write_blocking(spi, &command, 1);

    // Установка MISO в режим входа для считывания данных
    gpio_set_dir(miso_pin, GPIO_IN);
    
    // Чтение данных, 3 байта (24 бита) с текущей MISO линии
    spi_read_blocking(spi, 0x00, data, 3);
    
    // Объединяем три байта в одно 24-битное значение
    uint32_t received_data = ((uint32_t)data[0] << 16) | ((uint32_t)data[1] << 8) | (uint32_t)data[2];

    return received_data;
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

    // Таймер для контроля времени
    absolute_time_t last_read_time = get_absolute_time();

    while (true) {
        // Получаем текущее время
        absolute_time_t current_time = get_absolute_time();
        
        // Проверяем, прошло ли нужное количество времени
        if (absolute_time_diff_us(last_read_time, current_time) >= (READ_INTERVAL_MS * 1000)) {
            // Обновляем время последнего чтения
            last_read_time = current_time;

            // Чтение значений с АЦП
            uint32_t adc_value1 = read_adc(spi, SPI_MISO1);
            uint32_t adc_value2 = read_adc(spi, SPI_MISO2);
            uint32_t adc_value3 = read_adc(spi, SPI_MISO3);

            // Вывод значений в последовательный порт
            printf("ADC1 value: %lu\n", adc_value1);
            printf("ADC2 value: %lu\n", adc_value2);
            printf("ADC3 value: %lu\n", adc_value3);
        }

        // Здесь можно добавлять другие задачи, если это необходимо
    }

    return 0;
}

/*
Объяснение изменений:
Асинхронное считывание: Задержка была убрана, вместо этого используется таймер для контроля времени считывания данных с АЦП.
Контроль времени: Мы используем absolute_time_t и функцию absolute_time_diff_us для проверки, прошло ли заданное количество времени между считываниями. В этом примере, если прошло более 1000 мс, происходит новое считывание.
Устойчивость к блокировкам: Программа не блокирует выполнение, позволяя выполнять другие задачи (если они нужны) без задержек.
Примечание:
Приведенный код будет работать в основном цикле, позволяя выполнять другие операции в между считываниями. Однако следует помнить, что постоянное считывание может привести к повышенному потреблению ресурсов и работе на пределе возможностей системы, если это будет выполняться слишком часто. Вы можете изменить значение READ_INTERVAL_MS, чтобы адаптировать его к вашим требованиям.
*/