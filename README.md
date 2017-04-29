# CNC_Arduino


Autor: Gabriel Noya
Ultima modificación: 27-04-2017

Codigo de una maquina CNC cuya funcion es dibujar en papel cualquier imagen (con no muy alta resolucion) que se le envie desde la computadora.

Para el uso de esta CNC necesitamos dos archivos:

- CNC.ino
- CNC.exe

CNC.ino es el codigo que debera ser subido al arduino.
main.cpp es el codigo en C++ cuyo proposito es enviar por serial al Arduino las instrucciones leidas en un archivo que contenga el GCode del objeto a dibujar.

# USO:

El GCode se genera en un archivo de texto, y se genera utilizando el programa Inkscape a traves de una serie de pasos.

- Para el CNC.exe: 
Introducir el nombre del archivo donde se encuentra el GCODE (debe estar en la misma carpeta que el ejecutable) y luego introducir el puerto COM donde esta conectado el Arduino.

- Para el CNC.ino: 
Asignar las constantes PrevX, PrevY y PrevZ al punto donde inicialmente se encuentre el marcador sobre el papel, esto se debe hacer cada vez que se suba el codigo al Arduino.


# CUIDADO:

Mi CNC, fisicamente, trabaja con una hoja tamaño A4, sin embargo esta tiene unas limitaciones, por lo tanto hay que tener un margen de 2 cm en la parte superior, 3 cm en la parte inferior y 1 cm de cada lado restante (Esto se toma en cuenta al momento de generar el GCode).
En caso de ignorar esta advertencia y el GCode envia una instruccion para mover el marcador a un punto fuera del margen, uno de los engranajes se romperá.
Para evitar estos problemas, se añadio un algoritmo de proteccion al .cpp, el cual revisa el archivo antes de enviarlo por serial y si hay una coordenada fuera del margen, este te lo indicara.


