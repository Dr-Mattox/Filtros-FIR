#define M 51  // Orden del filtro FIR

// Coeficientes del filtro FIR generados en MATLAB
const float h[M] = {
  -0.0072223497,
  -0.0128897148,
  -0.0121975968,
  -0.0049459170,
  0.0056361301,
  0.0142614341,
  0.0161466435,
  0.0095582596,
  -0.0029612163,
  -0.0154427188,
  -0.0211964891,
  -0.0162021388,
  -0.0015501482,
  0.0163993898,
  0.0284981292,
  0.0270461157,
  0.0099615489,
  -0.0171035388,
  -0.0419431579,
  -0.0501329294,
  -0.0309497848,
  0.0175345020,
  0.0864308263,
  0.1581739921,
  0.2122370660,
  0.2323204163,
  0.2122370660,
  0.1581739921,
  0.0864308263,
  0.0175345020,
  -0.0309497848,
  -0.0501329294,
  -0.0419431579,
  -0.0171035388,
  0.0099615489,
  0.0270461157,
  0.0284981292,
  0.0163993898,
  -0.0015501482,
  -0.0162021388,
  -0.0211964891,
  -0.0154427188,
  -0.0029612163,
  0.0095582596,
  0.0161466435,
  0.0142614341,
  0.0056361301,
  -0.0049459170,
  -0.0121975968,
  -0.0128897148,
  -0.0072223497
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
const float periodo_us = 774.4;  // en microsegundos

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

