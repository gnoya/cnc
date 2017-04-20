//
// serial.c / serial.cpp
// A simple serial port writing example
// Written by Ted Burke - last updated 13-2-2013
// Edited for CNC usage by Gabriel Noya - last update 20-04-2017

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

#include <windows.h>
#include <stdio.h>

void fill_array(char *arr, int size, FILE *file);


int main(){
    int i;
    char bytes_to_send[64];
    int bytes_to_receive[1];
    bytes_to_receive[0]=0;
    FILE *coordenadas;
	coordenadas = fopen ( "dog.txt", "r" );

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

    // Set device parameters (38400 baud, 1 start bit,
    // 1 stop bit, no parity)
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (GetCommState(hSerial, &dcbSerialParams) == 0)
    {
        fprintf(stderr, "Error getting device state\n");
        CloseHandle(hSerial);
        return 1;
    }

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
    fputs ("Error al abrir el archivo",stderr);
    exit (0);
    }

    while (feof(coordenadas) == 0) {
        do{
            ReadFile(hSerial, bytes_to_receive, 1, &bytes_read, NULL);
        }while(bytes_to_receive[0]==0);

        fill_array(bytes_to_send, 64, coordenadas);
        printf("%s",bytes_to_send);
        printf("\n");

        fprintf(stderr, "Sending bytes... ");

        if(!WriteFile(hSerial, bytes_to_send, 64, &bytes_written, NULL))
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
    fprintf(stderr, "Closing serial port...");
    if (CloseHandle(hSerial) == 0)
    {
        fprintf(stderr, "Error\n");
        return 1;
    }
    fprintf(stderr, "OK\n");

    // exit normally
    return 0;
}


void fill_array(char *arr, int size, FILE *file){

    char aux[100] ={0};
    int i=0;
    int j=0;

    // Inicializamos el arr en 0.
    for (i=0;i<size;i++){
        arr[i] = 0;
    }
    i=0;

    // Llenara el auxiliar con las coordenadas que empiecen con G0 hasta el final de linea.
    fgets(aux, 100, file);
    while (aux[0]!='G' || aux[1]!='0'){ // if
            fgets(aux, 100, file);
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

            i = i+3;
            j = j+6;
        }
        i++;
        j++;
    }
}
