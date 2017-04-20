//
// serial.c / serial.cpp
// A simple serial port writing example
// Serial communication set-up written by Ted Burke - last updated 13-2-2013
//
//
// Edited for CNC usage by Gabriel Noya - last update 20-04-2017
// CNC usage: everything that is not serial communication set-up.

// To compile with MinGW:
//
//      gcc -o serial.exe serial.c
//
// To compile with cl, the Microsoft compiler:
//
//      cl serial.cpp
//
// To run:
//
//      serial.exe
//



// POR HACER:
// Hacer el file .exe.
// Poner datos al .exe
// Pedir el puerto serial por consola.

#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

int fill_array(char *arr, int size, FILE *file);
bool Protection(char *bytes);
float FindCoord(char Coordinate, char *Bytes);

char FileName[50];
char Origen[] = "G00 X0.000 Y0.000 ";
char OrigenFixed[] = "G00 X180.000000 Y230.000000 ";


int main(){
    int i=0;
    char bytes_to_send[64];
    int bytes_to_receive[1];
    bytes_to_receive[0]=0;
    FILE *coordenadas;

    // Preguntamos al usuario por el nombre del archivo.

    printf("Introduce el nombre del archivo: ");
    gets(FileName);
	coordenadas = fopen ( FileName, "r");

    // Declare variables and structures
    HANDLE hSerial;
    DCB dcbSerialParams = {0};
    COMMTIMEOUTS timeouts = {0};

    // Open the highest available serial port number
    fprintf(stderr, "Opening serial port...");
    hSerial = CreateFile(
                "\\\\.\\COM3", GENERIC_READ|GENERIC_WRITE, 0, NULL,
                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if (hSerial == INVALID_HANDLE_VALUE)
    {
            fprintf(stderr, "Error\n");
            return 1;
    }
    else fprintf(stderr, "OK\n");

    // Set device parameters (38400 (9600 en mi caso) baud, 1 start bit,
    // 1 stop bit, no parity)
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (GetCommState(hSerial, &dcbSerialParams) == 0)
    {
        fprintf(stderr, "Error getting device state\n");
        CloseHandle(hSerial);
        return 1;
    }

    // Cuidado con el Baudrate
    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    if(SetCommState(hSerial, &dcbSerialParams) == 0)
    {
        fprintf(stderr, "Error setting device parameters\n");
        CloseHandle(hSerial);
        return 1;
    }

    // Set COM port timeout settings
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if(SetCommTimeouts(hSerial, &timeouts) == 0)
    {
        fprintf(stderr, "Error setting timeouts\n");
        CloseHandle(hSerial);
        return 1;
    }

    // Send specified text (remaining command line arguments)
    DWORD bytes_written, total_bytes_written = 0;
    DWORD bytes_written2, total_bytes_written2 =0;
    DWORD bytes_read;

    if (coordenadas == NULL){
        fputs ("Error al abrir el archivo.",stderr);
        exit (0);
    }

    // Chequeamos si se cumple el margen de dibujo.

    while (feof(coordenadas) == 0) {
        if(fill_array(bytes_to_send, 64, coordenadas)==0) break;
        //Chequeamos G00 X0 Y0.
        if(strcmp(bytes_to_send,Origen)==0){
            memcpy(bytes_to_send,OrigenFixed, 64); // Cambiamos el G00 X0 Y0 por la instruccion de ir a nuestro origen particular.
        }

        printf("Linea %d: %s\n",i,bytes_to_send);
        if(!Protection(bytes_to_send)){
            printf("La instruccion excede las coordenadas margen. Revise la linea indicada.\n\n");
            return 0;
        }
        i++;
    }
    system("cls");
    printf("Archivo correcto.\n\n");
    i=0;

    // Retornamos el puntero al comienzo del archivo.
    rewind(coordenadas);


    while (feof(coordenadas) == 0) {
            // Esperamos que el arduino envie el handshake.
        do{
            ReadFile(hSerial, bytes_to_receive, 1, &bytes_read, NULL);
        }while(bytes_to_receive[0]==0);

        // Rellenamos el arreglo con la instruccion del archivo.
        if(fill_array(bytes_to_send, 64, coordenadas)==0) break;
        printf("%s",bytes_to_send);
        printf("\n");

        // Chequeamos origen.
        fprintf(stderr, "Sending bytes... ");

        if(!WriteFile(hSerial, bytes_to_send, 64, &bytes_written, NULL)) // Se manda el arreglo, elemento por elemento.
        {
            fprintf(stderr, "Error\n");
            CloseHandle(hSerial);
            return 1;
        }

        fprintf(stderr, "%d bytes written\n", bytes_written);
        bytes_to_receive[0]=0;
    }



    fprintf(stderr, "Done. \n");

	fclose (coordenadas);

    // Close serial port
    fprintf(stderr, "Closing serial port...\n");
    if (CloseHandle(hSerial) == 0)
    {
        fprintf(stderr, "Error\n");
        return 1;
    }
    fprintf(stderr, "OK\n");

    // exit normally
    return 0;
}


int fill_array(char *arr, int size, FILE *file){

    char aux[100] ={0};
    int i=0;
    int j=0;

    // Inicializamos el arr en 0.
    for (i=0;i<size;i++){
        arr[i] = 0;
    }
    i=0;

    // Llenara el auxiliar con las coordenadas que empiecen con G0 hasta el final de linea.
    if(fgets(aux, 100, file)==NULL) return 0;

    while (aux[0]!='G' || aux[1]!='0'){ // if
        if(fgets(aux, 100, file)==NULL) return 0;
    }

    // Llenamos el arreglo con lo que nos interesa.
    while (i<size && j<100){

        if (aux[j]=='\n'){
            arr[i] = ' ';    // Mandar espacio
            break;
        }
        if(aux[j]=='(' || aux[j]=='%' || aux[j]=='p'){
            break;
           }

        arr[i] = aux[j];

        if (aux[j]=='.'){

            if (aux[j+1]=='(')
                    break;
            arr[i+1] = aux[j+1];

            if (aux[j+2]=='(')
                    break;
            arr[i+2] = aux[j+2];

            if (aux[j+3]=='(')
                    break;
            arr[i+3] = aux[j+3];

            // La instruccion G00 x0 y0 tiene 4 decimales en lugar de 6, asi que agregamos esta condicion.
            if(arr[i-1]=='0' && arr[i]=='.' && arr[i+1]=='0' && arr[i+2]=='0' && arr[i+3]=='0'){
                i=i+3;
                j=j+4;
            }
            else{
                i = i+3;
                j = j+6;
            }
        }
        i++;
        j++;
    }
    return 1;
}

bool Protection(char *bytes){
    float X=FindCoord('X', bytes);
    float Y=FindCoord('Y', bytes);

    // Parametros de proteccion.
    if(X<28 || X>192 || Y<38 || Y>242){
        return false;
    }
    else{
        return true;
    }
}

float FindCoord(char Coordinate, char *Bytes){
  int Sign=1;                       // Signo de la coordenada.
  int Dot=2;                        // Posicion del punto decimal. Se inicia en 2 ya que si nunca encuentra punto(Cuando buscamos GO) el numero que retornara sera el que queremos (Ver codigo y asignacion de Dot).
  float C=0;
  int Aux[10];
  int i,j;
  memset(Aux,0,sizeof(Aux));
  i=0;
  j=0;

  while(!(Bytes[i]==Coordinate)){   // Busca la coordenada en el arreglo.
    if(i>64){
      return 150; // Si es un GO0 Z la proteccion fallara, por lo tanto retornamos 150 para que no falle.
    }
    i++;
  }
  i++;
  if(Bytes[i]=='-'){               // Revisa si la coordenada es negativa.
    Sign=-1;
    i++;
  }

  while(!(Bytes[i]==' ')){          // Guarda los valores siguientes a la coordenada en un vector auxiliar, esto se hace para ubicar el punto decimal y sumarlo todo luego.
    if(!(Bytes[i]=='.')){
      Aux[j]=Bytes[i]-'0';          // Convertimos char a int (lo mismo que atoi).
      i++;
      j++;
    }
    else{
      Dot=j;
      i++;
    }
  }

  for(j=0; j<sizeof(Aux); j++){   // Sumamos los valores del vector auxiliar, convirtiendo cada uno a su unidad correspondiente (Decenas, decimas, etc.)
    C+= Aux[j]*pow(10,Dot-1);
    Dot--;
    }
  return C*Sign;
}
