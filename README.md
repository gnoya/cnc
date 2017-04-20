# CNC_Arduino


Autor: Gabriel Noya

Code de una maquina CNC cuya funcion es dibujar en papel cualquier imagen que se le envie desde la computadora.

Para el uso de esta CNC necesitamos dos archivos:

- CNC.ino
- main.cpp

CNC.ino es el codigo que debera ser subido al arduino.
main.cpp es el codigo en C++ cuyo proposito es enviar por serial al Arduino las instrucciones leidas en un archivo que contenga el GCode del objeto a dibujar.

# USO:

El GCode se genera en un archivo de texto, y se genera utilizando el programa Inkscape a traves de una serie de pasos.

- Para el main.cpp: 
Se debe cambiar el nombre del archivo que se esta abriendo (linea 36), y cambiar el puerto serial a donde este conectado el arduino (linea 46).

- Para el CNC.ino: 
Asignar las constantes PrevX, PrevY y PrevZ al punto donde inicialmente se encuentre el marcador sobre el papel, esto se debe hacer cada vez que se suba el codigo al Arduino. (No hay que subirlo despues de cada dibujo, solo se sube nuevamente en caso de un error).

# CUIDADO:

Mi CNC, fisicamente, trabaja con una hoja tamaño A4, sin embargo esta tiene unas limitaciones fisicas, por lo tanto hay que tener un margen de 4cm verticalmente y 3 cm horizontalmente (Esto se toma en cuenta al momento de generar el GCode).
En caso de ignorar esta advertencia y el GCode envia una instruccion para mover el marcador a un punto fuera del margen, uno de los engranajes se romperá.


