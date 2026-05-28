#include <stdio.h> // Biblioteca padrão para usar printf()
// Biblioteca principal do FreeRTOS
#include "freertos/FreeRTOS.h"
// Biblioteca para usar tasks e delays
#include "freertos/task.h"
// Biblioteca para controlar os GPIOs da ESP32
#include "driver/gpio.h"
// Biblioteca para controle PWM do servo motor
#include "driver/ledc.h"
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

// FUNÇÃO PARA MEDIR DISTÂNCIA

// Função do tipo float
// Ela retorna um valor decimal da distância
float medir_distancia()
{
   
    // VARIÁVEIS DE TEMPO
    int32_t contagem = 1000000;
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
    // Esse é o tempo exigido pelo HC-SR04
    esp_rom_delay_us(5);

    // Desliga o TRIG
    // O pulso terminou
    gpio_set_level(TRIG_PIN, 0);

    
    // ESPERA O ECHO COMEÇAR
    
    /*
       Enquanto o ECHO estiver LOW:
       significa que o ultrassom ainda não voltou.
    */

    while (gpio_get_level(ECHO_PIN) == 0)
    {
        contagem--;
    }

    // TEMPO INICIAL
    
    // Guarda o instante exato que o ECHO começou
    inicio = esp_timer_get_time();

    // ESPERA O ECHO TERMINAR
     /*
       Enquanto o ECHO estiver HIGH:
       significa que o ultrassom ainda está retornando.
    */

    while (gpio_get_level(ECHO_PIN) == 1)
    {
    }
// TEMPO FINAL -------
    
    // Guarda o instante que o retorno terminou
    fim = esp_timer_get_time();

    
    // CÁLCULO DO TEMPO
    

    /*
       Calcula:
tempo final - tempo inicial
Resultado:
  tempo total do ultrassom
    */

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
void app_main()
{
    
    // CONFIGURAÇÃO DOS GPIOs
    

    // Configura o TRIG como saída, porque ele envia o sinal
gpio_set_direction(TRIG_PIN, GPIO_MODE_OUTPUT);

// Configura o ECHO como entrada, porque ele recebe o sinal
gpio_reset_pin(ECHO_PIN);
gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);
gpio_set_pull_mode(ECHO_PIN,GPIO_PULLUP_ONLY);


    // CONFIGURAÇÃO PWM DO SERVO
   /*
       Essa parte configura o PWM
       responsável pelo controle
       do servo motor.
  */
// Cria a estrutura do timer PWM
ledc_timer_config_t timer = {
// Modo rápido
.speed_mode = LEDC_HIGH_SPEED_MODE,
// Utiliza o timer 0
.timer_num = LEDC_TIMER_0,

 /*
Define resolução do PWM.
13 bits:
maior precisão no controle.
*/
.duty_resolution = LEDC_TIMER_13_BIT,
// Frequência padrão de servo motor
 .freq_hz = 50,

/*
 A ESP32 escolhe automaticamente
 o clock do PWM.
*/
.clk_cfg = LEDC_AUTO_CLK};
// Aplica a configuração do timer
ledc_timer_config(&timer);

    // CONFIGURAÇÃO DO CANAL PWM
    
ledc_channel_config_t canal = {
// GPIO do servo motor
.gpio_num = SERVO_PIN,
// Modo rápido
.speed_mode = LEDC_HIGH_SPEED_MODE,
// Canal PWM 0
.channel = LEDC_CHANNEL_0,
// Usa timer 0
.timer_sel = LEDC_TIMER_0,
// PWM começa desligado
.duty = 0,
// Ponto inicial do PWM
.hpoint = 0};

// Aplica configuração do canal
ledc_channel_config(&canal);

    // LOOP PRINCIPAL
    
    while (1)
    {
// Mede a distância
float distancia = medir_distancia();

// Mostra a distância no terminal
printf("Distancia: %.2f cm\n", distancia);

         // VERIFICA APROXIMAÇÃO
// Se a distância for menor que 20 cm
if (distancia < 20)
        {
// Mostra mensagem
printf("Objeto detectado\n");

 // ABRIR PORTA
/*
Define PWM do servo
para posição de abertura.*/
ledc_set_duty(
LEDC_HIGH_SPEED_MODE,
LEDC_CHANNEL_0,
600);

// Atualiza o PWM
ledc_update_duty(
LEDC_HIGH_SPEED_MODE,
LEDC_CHANNEL_0);

// Mostra mensagem
printf("Porta aberta\n");

// Espera 3 segundos
vTaskDelay(pdMS_TO_TICKS(8000));

           
// FECHAR PORTA
 /*  Define PWM do servo
     para posição de fechamento.*/
 ledc_set_duty(
 LEDC_HIGH_SPEED_MODE,
LEDC_CHANNEL_0,
200);
// Atualiza PWM
 ledc_update_duty(
 LEDC_HIGH_SPEED_MODE,
 LEDC_CHANNEL_0);
 // Mostra mensagem
 printf("Porta fechada\n");
        }
// Espera 500 ms antes da próxima leitura
 vTaskDelay(pdMS_TO_TICKS(500));
    }
}