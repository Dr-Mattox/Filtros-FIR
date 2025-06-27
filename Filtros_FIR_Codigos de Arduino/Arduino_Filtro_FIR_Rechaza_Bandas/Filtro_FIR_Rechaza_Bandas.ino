#define M 51  // Orden del filtro FIR

// Coeficientes del filtro FIR generados en MATLAB
const float h[M] = {
0.0000000000,
-0.0000032118,
-0.0000906676,
-0.0002498368,
-0.0000103429,
0.0009901544,
0.0018076828,
0.0006488582,
-0.0023722290,
-0.0040048886,
-0.0017539591,
0.0014023073,
0.0003041911,
-0.0026202797,
0.0031337542,
0.0193097651,
0.0253257266,
-0.0032086817,
-0.0544419783,
-0.0728929351,
-0.0150753195,
0.0847109176,
0.1278397133,
0.0524968011,
-0.0850939345,
0.8438058192,
-0.0861606223,
0.0538241446,
0.1327370852,
0.0890884308,
-0.0160621475,
-0.0787056418,
-0.0595932444,
-0.0035622696,
0.0285320894,
0.0220903263,
0.0036432042,
-0.0030986536,
0.0003663438,
0.0017224448,
-0.0022014803,
-0.0051498825,
-0.0031364458,
0.0008867306,
0.0025742232,
0.0014888876,
-0.0000168093,
-0.0004592234,
-0.0002092637,
-0.0000130544,
0.0000048452,
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

