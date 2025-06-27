// === Variables de entrada y salida ===
volatile float x0 = 0, x1 = 0, x2 = 0;
volatile float yf0 = 0, yf1 = 0, yf2 = 0;

// === Coeficientes del filtro IIR pasa bajas ===
// Forma estándar: y[n] = a1*x[n] + a2*x[n-1] + a3*x[n-2] - b2*y[n-1] - b3*y[n-2]
float a1 = 0.709679718566340; 
float a2 = -1.419359437132681;
float a3 = 0.709679718566340;
float b2 = -1.331921143871802;
float b3 = -0.506797730393560;

void setup() {
  analogReadResolution(12);   // 0-4095
  analogWriteResolution(12);  // 0-4095
  tcConfigure();
}

// === Configuración del temporizador ===
void tcConfigure() {
  pmc_set_writeprotect(false);
  pmc_enable_periph_clk(ID_TC0);

  TC_Configure(TC0, 0,
    TC_CMR_TCCLKS_TIMER_CLOCK1 |  // reloj MCK/2 (42 MHz)
    TC_CMR_WAVE |                 
    TC_CMR_WAVSEL_UP_RC);

uint32_t rc = (uint32_t)(42000000.0 / (1291.32/1));
  TC_SetRA(TC0, 0, rc / 2);
  TC_SetRC(TC0, 0, rc);

  TC0->TC_CHANNEL[0].TC_IER = TC_IER_CPCS;
  TC0->TC_CHANNEL[0].TC_IDR = ~TC_IER_CPCS;
  NVIC_EnableIRQ(TC0_IRQn);
  TC_Start(TC0, 0);
}

// === Interrupción del temporizador ===
void TC0_Handler() {
  TC_GetStatus(TC0, 0);  // limpia bandera de interrupción

  x0 = analogRead(A0);  // lectura de entrada

  // Ecuación de diferencias para filtro IIR
  yf0 = a1 * x0 + a2 * x1 + a3 * x2 + b2 * yf1 + b3 * yf2;

  // Saturación a 12 bits
  if (yf0 < 0) yf0 = 0;
  if (yf0 > 4095) yf0 = 4095;

  analogWrite(DAC0, (int)yf0);  // salida

  // Shift de muestras
  x2 = x1;
  x1 = x0;
  yf2 = yf1;
  yf1 = yf0;
}

void loop() {
  // Nada que hacer aquí
}
