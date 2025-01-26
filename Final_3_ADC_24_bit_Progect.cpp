#include "pico/stdlib.h"
#include "hardware/spi.h"

// Определяем пины для SPI и MISO
#define SPI_MOSI 12
#define SPI_MISO1 14  // MISO для первого АЦП
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
    stdio_init_all();  // Инициализация USB (Serial)

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
            uint32_t adc_value1 = read_adc(spi, SPI_MISO1);
            uint32_t adc_value2 = read_adc(spi, SPI_MISO2);
            uint32_t adc_value3 = read_adc(spi, SPI_MISO3);

            // Получаем текущее время в миллисекундах с начала работы
            uint64_t elapsed_time = to_ms_since_boot(current_time);

            // Форматируем выход в CSV
            printf("%llu,%lu,%lu,%lu\n", elapsed_time, adc_value1, adc_value2, adc_value3);
        }
    }

    return 0;
}

/*
Объяснение изменений:
Заголовок CSV: В начале программы добавлен заголовок "Time,ADC1,ADC2,ADC3\n", чтобы в файле была первая строка с названиями колонок.
Форматирование вывода: printf теперь форматирует вывод так, что каждый набор данных занимает одну строку, разделенную запятыми, что делает его подходящим для CSV.
    Добавлено также текущее время в миллисекундах, чтобы отслеживать временные задержки между чтениями.
Печать данных: Данные ADC выводятся на USB, чтобы их можно было записывать на компьютер.

Запись данных на компьютере:
Теперь, чтобы сохранить данные в файл CSV на компьютере, вы можете использовать терминал (например, PuTTY или любой другой) и перенаправить вывод в файл:
Подключите Raspberry Pi Pico к компьютеру через USB.
Запустите терминал на вашем компьютере и соединитесь с соответствующим COM-портом, который отображается для вашего устройства.
Перенаправьте вывод в файл. Например, если вы используете командную строку Linux или MacOS, вы можете выполнить:
    screen /dev/ttyACM0 115200 | tee output.csv
Или в Windows:
    putty -serial COM3 -sercfg 115200,8,n,1,N -log output.csv
*/