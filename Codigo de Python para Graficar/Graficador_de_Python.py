import tkinter as tk
from tkinter import filedialog
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import os

colores = ['blue', 'black', 'red', 'orange', 'cyan', 'magenta']

class AppGraficador:
    def __init__(self, root):
        self.root = root
        self.root.title("Graficador de Capturas de Osciloscopio")
        self.archivos = []
        self.fig = None
        self.ax = None
        self.canvas = None
        self.titulo_grafica = "Comparación Filtros"

        # tamaño ventana medio
        self.root.geometry("1300x750")

        # Frame horizontal
        self.main_frame = tk.Frame(root, bg="#f0f0f0")
        self.main_frame.pack(fill="both", expand=True)

        # Frame izquierdo
        self.frame_grafica = tk.Frame(self.main_frame, bg="#ffffff")
        self.frame_grafica.pack(side="left", fill="both", expand=True, padx=5, pady=5)

        # Frame derecho estético
        self.frame_controles = tk.Frame(self.main_frame, bg="#e0e0e0", relief="raised", bd=2)
        self.frame_controles.pack(side="right", fill="y", padx=5, pady=5)

        # Botón de carga
        self.btn_cargar = tk.Button(self.frame_controles, text="Seleccionar archivos CSV/XLSX",
                                    font=("Arial", 11, "bold"), bg="#4CAF50", fg="white",
                                    command=self.cargar_archivos)
        self.btn_cargar.pack(pady=15, padx=10, fill="x")

        # Sliders
        tk.Label(self.frame_controles, text="Zoom eje X (porcentaje)", bg="#e0e0e0",
                 font=("Arial", 10, "bold")).pack(pady=10)

        self.slider_tiempo_min = tk.Scale(self.frame_controles, from_=0, to=100,
                                          orient="horizontal", bg="#d0d0d0", troughcolor="#c0c0c0",
                                          command=self.actualizar_slider)
        self.slider_tiempo_min.set(0)
        self.slider_tiempo_min.pack(pady=5, padx=10, fill="x")
        self.lbl_tiempo_min = tk.Label(self.frame_controles, text="0.0 s", bg="#e0e0e0")
        self.lbl_tiempo_min.pack()

        self.slider_tiempo_max = tk.Scale(self.frame_controles, from_=0, to=100,
                                          orient="horizontal", bg="#d0d0d0", troughcolor="#c0c0c0",
                                          command=self.actualizar_slider)
        self.slider_tiempo_max.set(100)
        self.slider_tiempo_max.pack(pady=5, padx=10, fill="x")
        self.lbl_tiempo_max = tk.Label(self.frame_controles, text="0.0 s", bg="#e0e0e0")
        self.lbl_tiempo_max.pack()

        # Botón para guardar
        self.btn_guardar = tk.Button(self.frame_controles, text="Guardar captura",
                                     font=("Arial", 11, "bold"), bg="#2196F3", fg="white",
                                     command=self.guardar_captura)
        self.btn_guardar.pack(pady=20, padx=10, fill="x")

    def cargar_archivos(self):
        files = filedialog.askopenfilenames(title="Seleccionar archivos CSV/XLSX", 
                                            filetypes=[("CSV files", "*.csv"), ("Excel files", "*.xlsx")])
        if files:
            self.archivos = files
            self.graficar()

    def graficar(self):
        # Limpiar figura
        for widget in self.frame_grafica.winfo_children():
            widget.destroy()

        self.fig, self.ax = plt.subplots(figsize=(10,6))

        self.tiempo_total_min = float('inf')
        self.tiempo_total_max = float('-inf')

        for idx, archivo in enumerate(self.archivos):
            try:
                if archivo.endswith(".csv"):
                    df = pd.read_csv(archivo, skiprows=3, header=None)
                elif archivo.endswith(".xlsx"):
                    df = pd.read_excel(archivo, header=None, skiprows=3)
                else:
                    continue

                tiempo = df.iloc[:,0]
                amplitud = df.iloc[:,1]

                self.tiempo_total_min = min(self.tiempo_total_min, tiempo.min())
                self.tiempo_total_max = max(self.tiempo_total_max, tiempo.max())

                nombre_base = os.path.basename(archivo)
                partes = nombre_base.split("_")
                if partes[-1].split(".")[0].isdigit():
                    frecuencia = partes[-1].split(".")[0]
                else:
                    frecuencia = "desconocida"

                label = f"{frecuencia} Hz"
                color = colores[idx % len(colores)]
                self.ax.plot(tiempo, amplitud, label=label, color=color)

            except Exception as e:
                print(f"Error leyendo {archivo}: {e}")
                continue

        # Construir título dinámico con F mayúscula
        if self.archivos:
            partes = os.path.basename(self.archivos[0]).split("_")
            if len(partes) >= 5:
                tipo_filtro = partes[2]
                banda = partes[3] + "_" + partes[4]
                banda = banda.replace("_", " ")
                self.titulo_grafica = f"Comparación Filtros {tipo_filtro} {banda}"
            else:
                self.titulo_grafica = "Comparación Filtros"

            self.ax.set_title(self.titulo_grafica)

        self.ax.set_xlabel("Tiempo (s)")
        self.ax.set_ylabel("Amplitud (uV)")
        self.ax.grid(True)
        self.ax.legend()

        self.canvas = FigureCanvasTkAgg(self.fig, master=self.frame_grafica)
        self.canvas.draw()
        self.canvas.get_tk_widget().pack(fill="both", expand=True)

        self.slider_tiempo_min.set(0)
        self.slider_tiempo_max.set(100)
        self.actualizar_slider(None)

    def actualizar_slider(self, event):
        try:
            porcentaje_min = self.slider_tiempo_min.get()
            porcentaje_max = self.slider_tiempo_max.get()

            if porcentaje_min >= porcentaje_max:
                porcentaje_min = porcentaje_max - 1
                self.slider_tiempo_min.set(porcentaje_min)

            rango_total = self.tiempo_total_max - self.tiempo_total_min

            tiempo_min = self.tiempo_total_min + (porcentaje_min/100.0)*rango_total
            tiempo_max = self.tiempo_total_min + (porcentaje_max/100.0)*rango_total

            self.lbl_tiempo_min.config(text=f"{tiempo_min:.6f} s")
            self.lbl_tiempo_max.config(text=f"{tiempo_max:.6f} s")

            if self.ax:
                self.ax.set_xlim(tiempo_min, tiempo_max)
                self.canvas.draw()
        except Exception as e:
            print(f"Error ajustando zoom: {e}")

    def guardar_captura(self):
        try:
            sugerencia = self.titulo_grafica.replace(" ", "_") + ".png"
            archivo = filedialog.asksaveasfilename(
                defaultextension=".png",
                initialfile=sugerencia,
                filetypes=[("PNG files", "*.png")]
            )
            if archivo:
                self.fig.savefig(archivo, dpi=300)
                print(f"Guardado en: {archivo}")
        except Exception as e:
            print(f"Error guardando archivo: {e}")

# Ejecutar
root = tk.Tk()
app = AppGraficador(root)
root.mainloop()
