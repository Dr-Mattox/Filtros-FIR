#define M 51  // Orden del filtro FIR

// Coeficientes del filtro FIR generados en MATLAB
const float h[M] = {
0.0015624311,
0.0020093686,
0.0013719784,
-0.0004254968,
-0.0027930302,
-0.0042888580,
-0.0032579541,
0.0008083088,
0.0061849387,
0.0093022349,
0.0069063354,
-0.0012169724,
-0.0110555176,
-0.0161517080,
-0.0117431425,
0.0013636743,
0.0161718572,
0.0232185108,
0.0166894723,
-0.0010848753,
-0.0201430123,
-0.0286491896,
-0.0204704307,
0.0004166341,
0.0218722974,
0.0309490299,
0.0220257283,
0.0004225170,
-0.0209076509,
-0.0294736237,
-0.0208767088,
-0.0011329953,
0.0175678356,
0.0246421352,
0.0173116975,
0.0014730667,
-0.0128072263,
-0.0177953795,
-0.0123133519,
-0.0013712075,
0.0078781756,
0.0107506792,
0.0072459654,
0.0009600761,
-0.0039201060,
-0.0052143078,
-0.0034110927,
-0.0005161860,
0.0016228586,
0.0022598423,
0.0016305250,
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

