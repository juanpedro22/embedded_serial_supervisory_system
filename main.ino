#include <ESP32Time.h>
#include <HardwareSerial.h>


#define LED_PWM_PIN     23
#define POT_PIN         34
#define SERIAL_RX       16
#define SERIAL_TX       17
#define BUTTON_PIN      18
#define SERVO_PIN       19
#define LED2_PIN 5
#define LED3_PIN 12
#define BUZZER_PIN 27
bool aguardandoDataHora = false;
bool estadoLed2 = false;
bool estadoLed3 = false;
const int buzzerChannel = 2;    // Canal PWM exclusivo para o buzzer
const int buzzerFreq = 2000;    // Frequência inicial (em Hz)
const int buzzerResolution = 8; // Resolução de 8 bits (0–255)
HardwareSerial SerialUser(2);
ESP32Time rtc;

volatile bool botaoPressionado = false;
volatile int contadorBotao = 0;
unsigned long ultimaInterrupcao = 0;  // nova variável
String ultimaDataHora = "";

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

bool modoManual = false;
int valorManual = 0;

// LED PWM
const int pwmFreq = 5000;
const int pwmResolution = 8;
const int pwmChannel = 0;

// Servo PWM
const int servoPWMChannel = 1;
const int servoResolution = 10;
const float servoT = 20.0;
const float servoTmin = 0.5;
const float servoTmax = 2.5;
const float servoFreq = 1000.0 / servoT;
const int servoNsteps = pow(2, servoResolution);
int posicaoAtualGraus = 0;

// === Funções de LOG ===
void enviar(String msg) {
  Serial.print(msg);
  SerialUser.print(msg);
}

void enviarln(String msg) {
  Serial.println(msg);
  SerialUser.println(msg);
}

// === Funções Auxiliares ===
void mostrarPrompt() {
  String agora = rtc.getTime("%d/%m/%Y %H:%M:%S");
  enviar("[");
  enviar(agora);
  enviarln("] Digite o comando: ");
}
void tocarBuzzerPor3Segundos() {
  // Define parâmetros locais compatíveis com ledcAttachChannel
  const int freq = 2000;       // Frequência de 2 kHz
  const int resolution = 8;    // Resolução de 8 bits (0–255)
  const int dutyCycle = 220;   // 50% de duty cycle

  // Anexa canal apenas uma vez (pode mover para o setup se quiser evitar repetição)
  ledcAttachChannel(BUZZER_PIN, freq, resolution, buzzerChannel);

  // Toca o buzzer
  ledcWriteChannel(buzzerChannel, dutyCycle);
  delay(3000); // Mantém o som por 3 segundos

  // Para o som
  ledcWriteChannel(buzzerChannel, 0);
}



void moverServoPorPercentual(int percentual) {
  percentual = constrain(percentual, 0, 100);
  float pulso_ms = servoTmin + (servoTmax - servoTmin) * (percentual / 100.0);
  int dutyCycle = servoNsteps * pulso_ms / servoT;
  ledcWriteChannel(servoPWMChannel, dutyCycle);
  posicaoAtualGraus = map(percentual, 0, 100, 0, 180);
}

bool configurarHora(String dataHora) {
  if (dataHora.length() != 19) return false;
  int dia = dataHora.substring(0, 2).toInt();
  int mes = dataHora.substring(3, 5).toInt();
  int ano = dataHora.substring(6, 10).toInt();
  int hora = dataHora.substring(11, 13).toInt();
  int min = dataHora.substring(14, 16).toInt();
  int seg = dataHora.substring(17, 19).toInt();

  if (ano < 2020 || mes < 1 || mes > 12 || dia < 1 || dia > 31 || hora > 23 || min > 59 || seg > 59)
    return false;

  rtc.setTime(seg, min, hora, dia, mes, ano);
  return true;
}

// === Interrupção do Botão ===
void IRAM_ATTR handleBotao() {
  unsigned long agora = millis();
  if (agora - ultimaInterrupcao > 200) {  // 200 ms de proteção contra bounce
    ultimaInterrupcao = agora;
    botaoPressionado = true;
  }
}
void tocarBuzzer() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
}


// === Setup ===
void setup() {
  Serial.begin(115200);
  SerialUser.begin(115200, SERIAL_8N1, SERIAL_RX, SERIAL_TX);

  pinMode(LED_PWM_PIN, OUTPUT);
  pinMode(POT_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  SerialUser.setTimeout(10000); // 10 segundos

  ledcAttachChannel(LED_PWM_PIN, pwmFreq, pwmResolution, pwmChannel);
  ledcAttachChannel(SERVO_PIN, servoFreq, servoResolution, servoPWMChannel);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleBotao, FALLING);

  enviarln("=== Controle de Dispositivos com Hora ===");
  enviarln("Comandos: led1, led1manual, auto, sethora DD/MM/AAAA HH:MM:SS");
  mostrarPrompt();
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED3_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  
}

// === Loop Principal ===
void loop() {
  if (!modoManual) {

    int valorPot = analogRead(POT_PIN);
    int pwmValue = map(valorPot, 0, 4095, 0, 255);
    ledcWriteChannel(pwmChannel, pwmValue);
  } else {
    ledcWriteChannel(pwmChannel, valorManual);
  }

  if (SerialUser.available()) {

    String input = SerialUser.readStringUntil('\r');
    input.trim();
    Serial.print("[USER] ");
    Serial.println(input);

    if (aguardandoDataHora) {
      aguardandoDataHora = false;
      if (configurarHora(input)) {
        enviarln("Hora atualizada com sucesso!");
      } else {
        enviarln("Formato inválido. Use: DD/MM/AAAA HH:MM:SS");
      }
      mostrarPrompt();
      return; // para não continuar verificando os outros comandos
    }  
    else if (input.equalsIgnoreCase("pot")) {
      int valorPot = analogRead(POT_PIN);
      enviar("Valor do potenciometro (GPIO34): ");
      enviarln(String(valorPot));
    }
    else if (input.equalsIgnoreCase("led1")) {
      int pwmAtual = modoManual ? valorManual : map(analogRead(POT_PIN), 0, 4095, 0, 255);
      enviar("Valor atual do LED1 (GPIO23): ");
      enviarln(String(pwmAtual));
    }
    else if (input.equalsIgnoreCase("led1manual")) {
      modoManual = true;
      enviarln("Insira o valor (0-255) para ajustar o led1:");
      while (!SerialUser.available()) delay(10);

      String valorStr = SerialUser.readStringUntil('\n');
      valorStr.trim();
      Serial.print("[USER] ");
      Serial.println(valorStr);

      int val = constrain(valorStr.toInt(), 0, 255);
      valorManual = val;
      enviar("Valor manual ajustado para: ");
      enviarln(String(valorManual));
    }
    else if (input.equalsIgnoreCase("buzzer")) {
      tocarBuzzerPor3Segundos();
      enviarln("Buzzer tocou por 3 segundos.");
    }
    else if (input.equalsIgnoreCase("status")) {
      enviarln("=== STATUS DO SISTEMA ===");

      // Estado do LED1 (PWM)
      int pwmAtual = modoManual ? valorManual : map(analogRead(POT_PIN), 0, 4095, 0, 255);
      enviar("LED1 (GPIO23) - Modo: ");
      enviarln(modoManual ? "Manual" : "Automatico");
      enviar("PWM atual: ");
      enviarln(String(pwmAtual));

      // Valor atual do potenciômetro
      int valorPot = analogRead(POT_PIN);
      enviar("Potenciometro (GPIO34): ");
      enviarln(String(valorPot));

      // LED2
      enviar("LED2 (GPIO5): ");
      enviarln(estadoLed2 ? "Aceso" : "Apagado");

      // LED3
      enviar("LED3 (GPIO12): ");
      enviarln(estadoLed3 ? "Aceso" : "Apagado");

      // Servo
      enviar("Servo (GPIO19) - Posicao: ");
      enviar(String(posicaoAtualGraus));
      enviarln(" graus");

      // Push button
      portENTER_CRITICAL(&mux);
      int total = contadorBotao;
      String ultima = ultimaDataHora;
      portEXIT_CRITICAL(&mux);
      enviar("Push Button (GPIO18) - Total de acionamentos: ");
      enviarln(String(total));
      enviar("Ultimo acionamento em: ");
      enviarln(ultima);
    }
    else if (input.equalsIgnoreCase("auto")) {
      modoManual = false;
      enviarln("Modo automatico reativado.");
    }
    else if (input.equalsIgnoreCase("sethora")) {
      aguardandoDataHora = true;
      enviarln("Insira a data e hora no formato DD/MM/AAAA HH:MM:SS:");
    }
    else if (input.equalsIgnoreCase("help")) {
      enviarln("=== Lista de Comandos Disponiveis ===");
      enviarln("status       -> Mostra o estado atual de todos os dispositivos e sensores");
      enviarln("led1         -> Realiza a leitura do valor atual do PWM aplicado no LED1 (GPIO23)");
      enviarln("pot          -> Realiza a leitura do potenciometro (GPIO34)");
      enviarln("led1manual   -> Permite inserir valor manual (0-255) para o LED");
      enviarln("auto         -> Retorna controle do LED ao potenciometro");
      enviarln("sethora      -> Ajusta data/hora do sistema");
      enviarln("               Exemplo: sethora 04/05/2025 08:00:00");
      enviarln("help     -> Mostra esta lista de comandos");
      enviarln("servo        -> Mostra a posicao atual do servo (em graus)");
      enviarln("servo <pct>  -> Ajusta posicao do servo em graus de 0 a 180");
      enviarln("botao        -> Mostra status do botasethorao (contagem e hora)");
      enviarln("led2         -> Verifica estado do LED2 (GPIO5)");
      enviarln("led2 on      -> Acende o LED2 e toca buzzer");
      enviarln("led2 off     -> Apaga o LED2");
      enviarln("led3         -> Verifica estado do LED3 (GPIO12)");
      enviarln("led3 on      -> Acende o LED3 e toca buzzer");
      enviarln("led3 off     -> Apaga o LED3");
      enviarln("buzzer        -> Ativa o buzzer no GPIO27 por 3 segundos");
    }
    else if (input.equalsIgnoreCase("botao")) {
      portENTER_CRITICAL(&mux);
      int total = contadorBotao;
      String ultima = ultimaDataHora;
      portEXIT_CRITICAL(&mux);

      enviarln("=== Status do Push Button ===");
      enviar("Número de acionamentos: ");
      enviarln(String(total));
      enviar("Último acionamento em: ");
      enviarln(ultima);
    }
    else if (input.startsWith("servo")) {
      String valorStr = input.substring(5);
      valorStr.trim();

      if (valorStr.length() == 0) {
        // Comando apenas "servo" → mostrar posição atual
        enviar("Posição atual do servo: ");
        enviar(String(posicaoAtualGraus));
        enviarln(" graus");
      } else {
        // Comando "servo <graus>"
        int graus = valorStr.toInt();
        if (graus >= 0 && graus <= 180) {
          int percentual = map(graus, 0, 180, 0, 100); // converte para %
          moverServoPorPercentual(percentual);
          enviar("Servo ajustado para ");
          enviar(String(graus));
          enviarln(" graus");
        } else {
          enviarln("Valor invalido. Use: servo <0 a 180>");
        }
      }
    }
    else if (input.equalsIgnoreCase("led2 on")) {
      digitalWrite(LED2_PIN, HIGH);
      estadoLed2 = true;
      tocarBuzzer();
      enviarln("LED2 aceso.");
    }
    else if (input.equalsIgnoreCase("led2 off")) {
      digitalWrite(LED2_PIN, LOW);
      estadoLed2 = false;
      enviarln("LED2 apagado.");
    }
    else if (input.equalsIgnoreCase("led2")) {
      enviar("LED2 esta ");
      enviarln(estadoLed2 ? "aceso." : "apagado.");
    }

    else if (input.equalsIgnoreCase("led3 on")) {
      digitalWrite(LED3_PIN, HIGH);
      estadoLed3 = true;
      tocarBuzzer();
      enviarln("LED3 aceso.");
    }
    else if (input.equalsIgnoreCase("led3 off")) {
      digitalWrite(LED3_PIN, LOW);
      estadoLed3 = false;
      enviarln("LED3 apagado.");
    }
    else if (input.equalsIgnoreCase("led3")) {
      enviar("LED3 esta ");
      enviarln(estadoLed3 ? "aceso." : "apagado.");
    }
    else {
      enviarln("Comando desconhecido.");
    }

    mostrarPrompt();
  }

  if (botaoPressionado) {
    botaoPressionado = false;
    contadorBotao++;
    ultimaDataHora = rtc.getTime("%d/%m/%Y %H:%M:%S");

    if (rtc.getYear() > 1970) {
      enviar("[");
      enviar(ultimaDataHora);
      enviar("] ALERTA: Push button pressionado! Total: ");
      enviarln(String(contadorBotao));
    } else {
      enviarln("[???] ALERTA: Push button pressionado! Configure a hora com 'sethora'.");
    }
  }

  delay(50);
}
