clc; 
clear; 
close all;

%% Parámetros del filtro
format long
Fs = 1291.32;       % Frecuencia de muestreo (Hz)
fc = 150;           % Frecuencia de corte (Hz)
M = 51;             % Orden del filtro (debe ser impar)
n = -(M-1)/2 : (M-1)/2;  % Vector de índices

% Conversión de frecuencia de corte a frecuencia angular normalizada
wc = 2 * pi * fc / Fs;

for i = 1:M
    if n(i) == 0
        h(i) = 1 - (wc / pi); % Caso especial cuando n = 0
    else
        h(i) = -sin(wc * n(i)) / (pi * n(i)); % Ecuación para n ≠ 0
    end
    hamming(i) = (1 - cos(2 * pi * (i - 1) / M)) / 2;
end

%% Aplicación de la ventana de Hamming
hw = h .* hamming;

%% Gráfica de la respuesta al impulso
stem(n, hw, 'filled'); 
title('Gráfica'); 
xlabel('n'); 
ylabel('h(n)'); 
%grid on;

%% Imprimir solo los coeficientes para copiar/pegar en Arduino
fprintf('\n'); % Línea en blanco opcional
for i = 1:M
    fprintf('%.10f,\n', hw(i));
end
