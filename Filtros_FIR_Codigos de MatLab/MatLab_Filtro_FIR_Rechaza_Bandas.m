clc; clear; close all;

% Parámetros del filtro
Fs = 1291.32; % Frecuencia de muestreo en Hz
Fc1 = 150;      % Frecuencia de corte baja en Hz
Fc2 = 170;      % Frecuencia de corte alta en Hz
M = 51;        % Longitud del filtro (impar para simetría)

% Frecuencias normalizadas
wc1 = 2 * pi * Fc1 / Fs;
wc2 = 2 * pi * Fc2 / Fs;

% Vector de índices
n = -(M-1)/2 : (M-1)/2;


% Cálculo de la respuesta al impulso
for i = 1:M
    if n(i) == 0
        h(i) = 1 - (wc2 - wc1) / pi;
    else
        h(i) = (sin(wc1 * n(i)) - sin(wc2 * n(i))) / (pi * n(i));
    end
end

% Definir la ventana personalizada
w = (21 - 25 * cos(2 * pi * (n + (M-1)/2) / M) + 4 * cos(4 * pi * (n + (M-1)/2) / M)) / 50;

% Aplicar la ventana al filtro
h = h .* w;

% Gráfica de la respuesta al impulso
figure;
stem(n, h, 'filled');
xlabel('Índice n');
ylabel('h(n)');
title('Grafica');
%grid on;

%% Imprimir solo los coeficientes para copiar/pegar en Arduino
fprintf('\n'); % Línea en blanco opcional
for i = 1:M
    fprintf('%.10f,\n', h(i));
end
