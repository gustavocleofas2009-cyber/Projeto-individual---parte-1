#include <stdio.h> // Biblioteca padrão para usar printf()

// Biblioteca principal do FreeRTOS
#include "freertos/FreeRTOS.h"

// Biblioteca para usar tasks e delays
#include "freertos/task.h"

// Biblioteca para controlar os GPIOs da ESP32
#include "driver/gpio.h"

// Biblioteca para controle PWM do servo motor
#include "driver/ledc.h"

// Biblioteca para comunicação UART
#include "driver/uart.h"

// Biblioteca para manipulação de strings
#include <string.h>

// Biblioteca para delays em microssegundos
#include "esp_rom_sys.h"

// Biblioteca para medir tempo em microssegundos
#include "esp_timer.h"


// DEFINIÇÃO DOS PINOS

// Pino TRIG do HC-SR04
// Responsável por ENVIAR o ultrassom
#define TRIG_PIN GPIO_NUM_2

// Pino ECHO do HC-SR04
// Responsável por RECEBER o retorno do ultrassom
#define ECHO_PIN GPIO_NUM_13

// Pino conectado ao servo motor
// Responsável por controlar a abertura da porta
#define SERVO_PIN GPIO_NUM_14

// Pino TX da UART
// Responsável por enviar comandos para a ESP32-CAM
#define UART_TX_PIN GPIO_NUM_17

// Pino RX da UART
// Não será utilizado neste projeto
#define UART_RX_PIN GPIO_NUM_16

// FUNÇÃO PARA MEDIR DISTÂNCIA

// Função do tipo float
// Ela retorna um valor decimal da distância
float medir_distancia()
{
    // Variável do tempo inicial
    int64_t inicio = 0;

    // Variável do tempo final
    int64_t fim = 0;


    // DISPARO DO HC-SR04
    
    // Garante que o TRIG comece desligado
    gpio_set_level(TRIG_PIN, 0);

    // Espera 2 microssegundos
    esp_rom_delay_us(2);

    // Liga o TRIG
    // Isso faz o sensor enviar o ultrassom
    gpio_set_level(TRIG_PIN, 1);

    // Mantém ligado por 10 microssegundos
    esp_rom_delay_us(10);

    // Desliga o TRIG
    gpio_set_level(TRIG_PIN, 0);

    
    // ESPERA O ECHO COMEÇAR
   
    /*
       Enquanto o ECHO estiver LOW:

       significa que o ultrassom
       ainda não retornou.
    */
    while (gpio_get_level(ECHO_PIN) == 0)
    {
    }

    // Guarda o instante exato
    // em que o ECHO começou
    inicio = esp_timer_get_time();


    // ESPERA O ECHO TERMINAR
     /*
       Enquanto o ECHO estiver HIGH:

       significa que o sinal ainda
       está retornando ao sensor.
    */
    while (gpio_get_level(ECHO_PIN) == 1)
    {
    }

    // Guarda o instante final
    fim = esp_timer_get_time();

// CÁLCULO DO TEMPO
    
 /*tempo = fim - inicio
Resultado:
tempo total da viagem
do ultrassom.*/
    float tempo = fim - inicio;

    
    // CÁLCULO DA DISTÂNCIA
/* Vamos criar uma variável  float para calcular a distância
detectada pelo sensor ultrassônico. Usamos float porque o valor
da distância pode possuir casas decimais.
O valor 0.034 representa a velocidade do som em cm por microssegundo.
A divisão é por 2 porque o ultrassom percorre o caminho de ida
até o objeto e depois retorna ao sensor. Ai dividimos por 2 por que
prescisamos do resultado para obter apenas a distância entre o
sensor e o objeto.*/

    float distancia = tempo * 0.034 / 2;

    // Retorna a distância medida
    return distancia;
}

// APP PRINCIPAL
void app_main()
{
    // CONFIGURAÇÃO DOS GPIOs

   // Configura o TRIG como saída
    gpio_set_direction(
        TRIG_PIN,
        GPIO_MODE_OUTPUT
    );

    // Configura o ECHO como entrada
    gpio_reset_pin(ECHO_PIN);

    gpio_set_direction(
        ECHO_PIN,
        GPIO_MODE_INPUT
    );

    gpio_set_pull_mode(
        ECHO_PIN,
        GPIO_PULLUP_ONLY
    );

    // CONFIGURAÇÃO PWM DO SERVO
    /*
       Essa parte configura o PWM
       responsável pelo controle
       do servo motor.
    */

    ledc_timer_config_t timer =
    {
// Modo rápido
        .speed_mode = LEDC_HIGH_SPEED_MODE,
// Timer 0
        .timer_num = LEDC_TIMER_0,

        /*
 Resolução de 13 bits
Quanto maior a resolução,
mais preciso fica o servo.
        */
        .duty_resolution = LEDC_TIMER_13_BIT,
// Frequência padrão do SG90
        .freq_hz = 50,
// Clock automático
        .clk_cfg = LEDC_AUTO_CLK
    };

    ledc_timer_config(&timer);

    // CONFIGURAÇÃO DO CANAL PWM


    ledc_channel_config_t canal =
    {
        .gpio_num = SERVO_PIN,

        .speed_mode = LEDC_HIGH_SPEED_MODE,

        .channel = LEDC_CHANNEL_0,

        .timer_sel = LEDC_TIMER_0,

        .duty = 0,

        .hpoint = 0
    };

    ledc_channel_config(&canal);

  // CONFIGURAÇÃO DA UART
     /*
A UART será utilizada para
comunicar com a ESP32-CAM.

Quando alguém for detectado,
será enviada a palavra:
       FOTO

para solicitar a captura
de uma imagem.
    */

    uart_config_t uart_config =
    {
        .baud_rate = 115200,

        .data_bits = UART_DATA_8_BITS,

        .parity = UART_PARITY_DISABLE,

        .stop_bits = UART_STOP_BITS_1,

        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_driver_install(
UART_NUM_1,
1024,
0,
0,
NULL,
0
    );
uart_param_config(
UART_NUM_1,
&uart_config
    );

uart_set_pin(
UART_NUM_1,
UART_TX_PIN,
UART_RX_PIN,
UART_PIN_NO_CHANGE,
UART_PIN_NO_CHANGE
    );

    printf("\n=====================================\n");
    printf(" SISTEMA DE PORTA AUTOMATICA\n");
    printf(" HC-SR04 + SG90 + ESP32-CAM\n");
    printf("=====================================\n");

// LOOP PRINCIPAL
     while (1)
    {
// Mede a distância
float distancia = medir_distancia();
// Mostra a distância
        printf("Distancia: %.2f cm\n",distancia);
// DETECÇÃO DE OBJETO
        
        if (distancia < 20)
        {
            printf("Objeto detectado\n");
// ENVIA COMANDO PARA ESP32-CAM
 /*
Quando um objeto for detectado,
enviamos o comando:

      FOTO
para que a ESP32-CAM
capture uma imagem. */

 char msg[] = "FOTO\n";
 uart_write_bytes(
 UART_NUM_1,
 msg,
 strlen(msg)
 );
 printf("Comando FOTO enviado para ESP32-CAM\n");

// ABERTURA DA PORTA
            
/*Duty 600
 Posição de abertura
 do servo motor. */
ledc_set_duty(
LEDC_HIGH_SPEED_MODE,
LEDC_CHANNEL_0,
600
);

ledc_update_duty(
LEDC_HIGH_SPEED_MODE,
LEDC_CHANNEL_0
);
printf("Porta aberta\n");
// Mantém aberta por 8 segundos
 vTaskDelay(
pdMS_TO_TICKS(8000)
            );
// FECHAMENTO DA PORTA

/*
Duty 200

Posição de fechamento
do servo motor.*/

ledc_set_duty(
LEDC_HIGH_SPEED_MODE,
 LEDC_CHANNEL_0,
 200
);
 ledc_update_duty(
LEDC_HIGH_SPEED_MODE,
LEDC_CHANNEL_0
);

 printf("Porta fechada\n");
   }

 // Aguarda 500 ms
// antes da próxima leitura
vTaskDelay(
pdMS_TO_TICKS(500)
 );
}
}
