clc; clear; close all;
format long;
% Parámetros del filtro
Fs = 1291.32/2; % Frecuencia de muestreo en Hz
Fc1 = 50;      % Frecuencia de corte baja 1 en Hz
Fc2 = 150;      % Frecuencia de corte baja 2 en Hz
Fc3 = 250;      % Frecuencia de corte alta 1 en Hz
Fc4 = 350;   % Frecuencia de corte alta 2 en Hz

% Amplitudes correspondientes a las bandas
A1 = 1;
A2 = 0.5;
A3 = 0.2;
A4 = 1;

% Longitud del filtro (M impar para simetría)
M = 51;  

% Frecuencias normalizadas
w1 = 2 * pi * Fc1 / Fs;
w2 = 2 * pi * Fc2 / Fs;
w3 = 2 * pi * Fc3 / Fs;
w4 = 2 * pi * Fc4 / Fs;

% Vector de índices
n = -(M-1)/2 : (M-1)/2;

% Cálculo de la respuesta al impulso usando la ecuación dada
for i = 1:M
    if n(i) == 0
        h(i) = (A1 * w1 + A2 * (w2 - w1) + A3 * (w3 - w2) + A4 * (w4 - w3)) / pi;
    else
        h(i) = ((A1 - A2) * sin(w1 * n(i)) + (A2 - A3) * sin(w2 * n(i)) + ...
                (A3 - A4) * sin(w3 * n(i)) + (A4 - A1) * sin(w4 * n(i))) / (pi * n(i));
    end
end

% Gráfica de la respuesta al impulso
figure;
stem(n, h, 'filled');
xlabel('Índice n');
ylabel('h(n)');
title('Gráfica');
grid on;
%% Imprimir coeficientes para copiar y pegar
fprintf('\n'); % Línea en blanco opcional
for i = 1:M
    fprintf('%.10f,\n', h(i));
end
