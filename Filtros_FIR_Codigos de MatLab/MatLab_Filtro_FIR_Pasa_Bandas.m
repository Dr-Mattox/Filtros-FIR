clc; 
clear; 
close all;

%% Parámetros del filtro
format long
Fs = 1291.32;       % Frecuencia de muestreo (Hz)
fc1 = 150;             % Frecuencia de corte inferior (Hz)
fc2 = 170;             % Frecuencia de corte superior (Hz)
M = 51;               % Orden del filtro (debe ser impar)
n = -(M-1)/2 : (M-1)/2;  % Vector de índices

% Conversión de frecuencia de corte a frecuencia angular normalizada
wc1 = 2 * pi * fc1 / Fs;
wc2 = 2 * pi * fc2 / Fs;

for i = 1:M
    if n(i) == 0
        h(i) = (wc2 - wc1) / pi; % Caso especial cuando n = 0
    else
        h(i) = (sin(wc2 * n(i)) - sin(wc1 * n(i))) / (pi * n(i)); % Ecuación para n ≠ 0
    end
    hanning(i)= (27 - 23 * cos(2 * pi * (i - 1) / M)) / 50;
end

%% Aplicación de la ventana de Hamming
hw = h .* hanning;

% Respuesta al impulso
stem(n, hw, 'filled'); 
title('Gráfica'); 
xlabel('n'); 
ylabel('h(n)'); 
%grid on;

%% Imprimir coeficientes para copiar y pegar
fprintf('\n'); % Línea en blanco opcional
for i = 1:M
    fprintf('%.10f,\n', hw(i));
end
