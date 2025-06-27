#define M 51  // Orden del filtro FIR

// Coeficientes del filtro FIR generados en MATLAB
const float h[M] = {

0.0031567462,
-0.0171979778,
0.0028574669,
-0.0028050411,
-0.0174425498,
0.0065228524,
-0.0064135262,
0.0129762394,
0.0142171037,
-0.0108995537,
0.0254834645,
0.0038739010,
-0.0026606886,
0.0034470334,
-0.0376848149,
0.0111698077,
-0.0136730318,
-0.0411608176,
0.0195952302,
-0.0223460230,
0.0568772007,
0.0459711314,
-0.0494780688,
0.2021344269,
0.0035803842,
0.6814732212,
0.0035803842,
0.2021344269,
-0.0494780688,
0.0459711314,
0.0568772007,
-0.0223460230,
0.0195952302,
-0.0411608176,
-0.0136730318,
0.0111698077,
-0.0376848149,
0.0034470334,
-0.0026606886,
0.0038739010,
0.0254834645,
-0.0108995537,
0.0142171037,
0.0129762394,
-0.0064135262,
0.0065228524,
-0.0174425498,
-0.0028050411,
0.0028574669,
-0.0171979778,
0.0031567462,
};

// Variables de señal
volatile uint16_t adcValue = 0;
volatile uint16_t dacValue = 0;
volatile bool ledState = false;

float x[M];           // Buffer de muestras
float y_out = 0.0f;   // Salida del filtro

const float offset = 2048.0f;   // Centro del rango de 12 bits (4096 / 2)
const float escala = 1.0f;      // Escalado opcional

// Periodo deseado para 1291.32 Hz
const float periodo_us = 774.4*2;  // en microsegundos

void setup() {
  Serial.begin(115200);

  analogReadResolution(12);    // ADC 12 bits
  analogWriteResolution(12);   // DAC 12 bits

  pinMode(13, OUTPUT);         // LED para verificar interrupción
  configurarTimer1(periodo_us);
}

void loop() {
  // Nada aquí
}

void configurarTimer1(float periodo_us) {
  PMC->PMC_PCER0 |= (1 << ID_TC0); // Habilitar reloj para TC0

  TC0->TC_CHANNEL[0].TC_CMR =
    TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK2;

  uint32_t frecuenciaTimer = SystemCoreClock / 8; // 10.5 MHz
  uint32_t valorRC = (frecuenciaTimer * periodo_us) / 1e6;
  TC0->TC_CHANNEL[0].TC_RC = valorRC;

  TC0->TC_CHANNEL[0].TC_IER = TC_IER_CPCS;
  NVIC_EnableIRQ(TC0_IRQn);
  TC0->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG;

  Serial.print("Timer configurado a: ");
  Serial.print(periodo_us);
  Serial.print(" us (TC_RC = ");
  Serial.print(valorRC);
  Serial.println(")");
}

void TC0_Handler() {
  TC0->TC_CHANNEL[0].TC_SR; // Limpiar bandera de interrupción
   digitalWrite(13, LOW);
  // Desplazar las muestras del buffer (última a la derecha)
  for (int i = M - 1; i > 0; i--) {
    x[i] = x[i - 1];
  }

  // Leer muestra del ADC
  adcValue = analogRead(A0);
  x[0] = ((float)adcValue - offset) * escala;

  // Escribir la señal sin filtrar en DAC1
  analogWrite(DAC1, adcValue);  // DAC1 muestra la señal original

  // Aplicar filtro FIR (convolución)
  y_out = 0.0f;
  for (int i = 0; i < M; i++) {
    y_out += h[i] * x[i];
  }

  // Reconstruir y saturar la señal filtrada
  float salidaFiltrada = y_out + offset;
  if (salidaFiltrada < 0) salidaFiltrada = 0;
  if (salidaFiltrada > 4095) salidaFiltrada = 4095;

  dacValue = (uint16_t)salidaFiltrada;
  analogWrite(DAC0, dacValue);  // DAC0 muestra la señal filtrada

  // Parpadeo LED como marca de muestreo

  digitalWrite(13, HIGH);
}

