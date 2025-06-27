clc; clear; close all;

%% Parámetros del filtro
Fs = 1291.32;       % Frecuencia de muestreo (Hz)
fc = 150;          % Frecuencia de corte (Hz)
M = 51;             % Orden del filtro (debe ser impar)
n = -(M-1)/2 : (M-1)/2;  % Vector de índices

% Conversión de frecuencia de corte a frecuencia angular normalizada
wc = 2 * pi * fc / Fs;

%% Implementación de la ecuación del filtro pasa bajas
h = sin(wc * n) ./ (pi * n);       % Para n ≠ 0
h((M-1)/2 + 1) = wc / pi;          % Caso especial para n = 0

%% Mostrar respuesta al impulso
stem(n, h, 'filled'); 
title('Respuesta al Impulso - Filtro Pasa Bajas'); 
xlabel('n'); 
ylabel('h(n)'); 
grid on;

%% Tabular coeficientes en consola
fprintf('\nTabla de Coeficientes h[n]:\n');
fprintf('n\t\t h(n)\n');
for i = 1:M
    fprintf('%d\t\t %.10f\n', n(i), h(i));
end

%% Formato para C / Arduino (para copiar y pegar)
fprintf('\n\n// Coeficientes del filtro FIR (M = %d)\n', M);
fprintf('const float h[%d] = {\n', M);
for i = 1:M
    if i < M
        fprintf('  %.10f,\n', h(i));
    else
        fprintf('  %.10f\n', h(i));  % último sin coma
    end
end
fprintf('};\n');
