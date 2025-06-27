#define M 51  // Orden del filtro FIR

// Coeficientes del filtro FIR generados en MATLAB
const float h[M] = {
  0.0000000000,
0.0000488487,
0.0001842024,
0.0001669934,
-0.0003353154,
-0.0013106478,
-0.0021070650,
-0.0016697405,
0.0006627721,
0.0042796534,
0.0070758671,
0.0063679827,
0.0007035593,
-0.0084522050,
-0.0164347522,
-0.0172238183,
-0.0069222100,
0.0128276541,
0.0336097788,
0.0425160817,
0.0275310804,
-0.0162213180,
-0.0824751152,
-0.1544523163,
-0.2104301963,
0.7669515658,
-0.2120357940,
-0.1568273857,
-0.0843971954,
-0.0167319940,
0.0286319110,
0.0445952604,
0.0355706070,
0.0137053618,
-0.0074711617,
-0.0187941548,
-0.0181485062,
-0.0094574597,
0.0007989425,
0.0073535970,
0.0083309294,
0.0051551285,
0.0008206443,
-0.0021393059,
-0.0028206709,
-0.0018610536,
-0.0005179691,
0.0002942519,
0.0004118383,
0.0001946544,
0.0000273709,
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

