/*
 * Autor: Gabriel Noya
 * Ultima fecha de edicion: 27-04-2017
 * Fin: Proyecto de nuevo ingreso a la agrupacion CITE, de la Universidad Simon Bolivar.
 */


// Posibles To-do:
// Quitar tantas variables globales ó:
// Quitar tantos parametros y usar las variables como globales.

// Notas:
// Las coordenadas se encuentran en micrometros. Esto se hizo para obtener mayor precision y no se perdieran decimales al trabajar con milimetros.
// Las demas observaciones se encuentran en el codigo.

#include <Servo.h>
Servo myservo;

char Bytes[64];

int i=0;
int j=0;

// Coordenadas iniciales del marcador encima de la hoja.
// PrevX,Y,Z se refieren a las coordenadas anteriores a la instruccion que realizaremos.
// X,Y,Z se refieren a las coordenadas ACTUALES durante la instruccion.
// NextX,Y,Z se refieren a las coordenadas objetivas.

float PrevX=70000;
float PrevY=130000;
float PrevZ=5000;
float X=PrevX;
float Y=PrevY;
float Z=PrevZ;
float NextX;
float NextY;
float NextZ;
float G;
float I;
float J;

float AuxX;
float AuxY;

int posNoDraw=90; // Posicion del servo para el cual no dibuja.
int posDraw=130; // Posicion del servo para el cual dibuja.
int Delay0=10;   // Delay para la instruccion G00.
int Delay1=15;  // Delay para la instruccion G01.
int Delay=Delay1;
int Trit[3];

float d=100;            // d= distancia por paso del motor (0.1 mm).

// Asignacion de pines.

int Dirx=12; //4
int Diry=4; //12
int Stepx=13; //7
int Stepy=7; //13
int ServoPin = 11;

void setup() {
  Serial.begin(9600);
  pinMode(Dirx, OUTPUT);
  pinMode(Stepx, OUTPUT);
  pinMode(Diry, OUTPUT);
  pinMode(Stepy, OUTPUT);
  myservo.attach(ServoPin);
  myservo.write(posNoDraw);
}

void loop() {
  i=0;
  // Le enviamos datos al programa en C para decirle que estamos listos para recibir una instruccion.
  while(!Serial.available()){
    Serial.write('a');
    delay(100);
  }
  delay(1);
  
  // Llenamos el arreglo Bytes con la instruccion.
  while(Serial.available()){
      Bytes[i]= (char)Serial.read();
      i++;
  }
  
  // Buscamos dentro del arreglo Bytes cada parametro de la instruccion y lo guardamos en la variable correspondiente.
  G= round(FindCoord('G',Bytes)*1000);
  NextX= round(FindCoord('X',Bytes)*1000);
  NextY= round(FindCoord('Y',Bytes)*1000);
  NextZ= round(FindCoord('Z',Bytes)*1000);
  I= round(FindCoord('I',Bytes)*1000);
  J= round(FindCoord('J',Bytes)*1000);

  AssignTrit(PrevX, PrevY, NextX, NextY, Trit, d);

  // Patch: Si el comando siguiente es solo mover el eje Z, los FindCoord() devolveran 0 y todos estos datos serán 0, por lo tanto toca mover el eje Z.
  if(NextX==0 && NextY==0 && I==0 && J==0){
    MoveZAxis(NextZ,PrevZ);
    
    // Si no hacemos estas dos asignaciones PrevX y NextX al final del loop se volveran 0.
    NextX=PrevX;
    NextY=PrevY;
  }
  
  if(G==0){               // Mueve el brazo de manera rapida pero con el lapiz arriba.
    Delay=Delay0;
    LinearMovement(&X, &Y, NextX, NextY, PrevX, PrevY, Trit, d);
    Delay=Delay1;
  }
  
  else if(G==1000){          // Dibuja, mueve el brazo lentamente.
    LinearMovement(&X, &Y, NextX, NextY, PrevX, PrevY, Trit, d);
  }

  else if(G==2000 || G==3000){          // Realiza movimientos circulares.
    RotateSet(&X, &Y, I, J, NextX, NextY, PrevX, PrevY, Trit, d);
  }
  
  else{
    // Nada
  }

  //Actualizamos las variables previas.
 PrevX=NextX;
 PrevY=NextY;
 PrevZ=NextZ;
 // Para mayor precision de software, cambiamos las coordenadas actuales. El error de esta asignacion es minimo debido a que el paso del motor es muy pequeño.
 X=PrevX;
 Y=PrevY;
}

// Esta funcion se encarga de buscar la coordenada Coordinate en el arreglo donde se encuentra la instruccion. Luego la retorna.
// Esto lo hace buscando caracter por caracter hasta encontrar el deseado, luego separa en partes los digitos encontrados, los convierte de ASCII a decimal y los suma. Si el numero es negativo se multiplica por -1.
float FindCoord(char Coordinate, char Bytes[]){
  int Sign=1;                       // Signo de la coordenada.
  int Dot=2;                        // Posicion del punto decimal. Se inicia en 2 ya que si nunca encuentra punto(Cuando buscamos GO) el numero que retornara sera el que queremos (Ver codigo y asignacion de Dot).
  float C=0;
  int Aux[10];
  memset(Aux,0,sizeof(Aux));
  i=0;
  j=0;
  
  while(!(Bytes[i]==Coordinate)){   // Busca la coordenada en el arreglo.
    if(i>64){
      return 0; // Revisar casos de no repetirse las coordenadas.
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


// Esta funcion asigna a cada eje el respectivo movimiento que realizara dependiendo de las coordenadas actuales y las coordenadas siguientes.
// Trit[0] corresponde al eje X. Trit[1] al eje Y.
// Si el Trit es igual a 2, es que no realizaremos movimiento en el eje en esta instrucción. Esta informacion es util para otras funciones.
// Si el Trit vale 0, la coordenada disminuirá en esta instrucción.
// Si el Trit vale 1, la coordenada aumentará en esta instrucción.
void AssignTrit(float PrevX, float PrevY, float NextX, float NextY, int Trit[], float d){
  
  if(abs(PrevX-NextX)<d){
    Trit[0]= 2; // Para X
  }
  if(PrevX>NextX){
    Trit[0]= 0; // Para X
  }
  if(PrevX<NextX){
    Trit[0]= 1; // Para X
  }
  
  if(abs(PrevY-NextY)<d){
    Trit[1]= 2; // Para Y    
  }  
  if(PrevY>NextY){
    Trit[1]= 0; // Para Y    
  }
  if(PrevY<NextY){
    Trit[1]= 1; // Para Y  
  }
}

// Mueve un paso del motor correspondiente al eje X.
void MoveX(int DirectionX){
  if(DirectionX == 1){
     digitalWrite(Dirx,LOW);
   }
  else if(DirectionX == 0){
   digitalWrite(Dirx,HIGH);
  }
  else{
  }
  digitalWrite(Stepx,HIGH);
  delayMicroseconds(1);
  digitalWrite(Stepx,LOW);
  delay(Delay);
}

// Mueve un paso del motor correspondiente al eje Y.
void MoveY(int DirectionY){
  if(DirectionY == 1){
    digitalWrite(Diry,LOW);
  }
  else if (DirectionY == 0){
    digitalWrite(Diry,HIGH);
  }
  else {
  }
  digitalWrite(Stepy,HIGH);
  delayMicroseconds(1);
  digitalWrite(Stepy,LOW);
  delay(Delay);
}


void MoveZAxis(float NextZ, float PrevZ){
  // Lo definimos cuando tengamos el funcionamiento exacto del servomotor.
  if(NextZ==PrevZ){
  }
  else{
    if(NextZ==-1000){
      myservo.write(posDraw);
    }
    else if(NextZ==5000){
      myservo.write(posNoDraw);
    }
    else{
    }
  }
}

// Esta funcion se encarga de calcular el radio de giro para un movimiento circular, este radio de giro dependen de las variables I y J de la instruccion.
// Los valores I y J son el offset para el centro del circulo. Es decir que si estamos en (1,1) e I=-1 y J=-1, el centro del circulo estara en (0,0). Por lo tanto el radio es 1.
float Ratio(float I, float J){
  float r;
  r=pow(I,2)+pow(J,2);
  r=sqrt(r);
  return r;
}

// Esta funcion se encarga de calcular los valores auxiliares de las coordenadas, es decir, calcula el valor de X y Y suponiendo que se muevan un paso hacia donde deberan hacerlo esta instruccion (en base al sistema de trits).
float Auxiliary(float N, int A[], int n, float d){
  float Aux;
  if(A[n]==0){
    Aux=N-d;
  }
  else if(A[n]==1){
    Aux=N+d;
  }
  else{
    Aux=N;
  }
  return Aux;
}

// Esta funcion calcula la pendiente entre dos puntos dados del eje de coordenadas y la retorna.

float Slope(float A, float B, float C, float D){ // (X1,Y1,X2,Y2)
  float s;
  s= (D-B) / (C-A);
  return s;
}

// Esta funcion se encarga de controlar los movimientos lineales. En esta funcion se calcula la pendiente respectiva a la trayectoria que realizaremos.
// Existen ciertos problemas cuando se calculan las pendientes, por ejemplo cuando la pendiente es infinita, para eso creamos la variable Infinity y Counterinfinity. Seran explicadas en la funcion MoveLinear.

void LinearMovement(float *X, float *Y, float NextX, float NextY, float PrevX, float PrevY, int Trit[], float d){
  float m; // Pendiente de la trayectoria a realizar.
  int Infinity=0;
  int CounterInfinity=0;

  // Si hay un movimiento rectilineo, y no nos moveremos en X (El cambio de PrevX y NextX es menor a la distancia por paso), haremos que la variable Infinity sea 1. Lo usaremos en MoveLinear.
  if(abs(NextX-PrevX)<d){
    Infinity=1;
    m=0;
  }
  else{
    m = Slope(PrevX,PrevY,NextX,NextY);
  }
  // Hasta que nuestra coordenada actual (X,Y) no este lo suficientemente cerca (distancia menor de paso) de la coordenada objetiva (NextX, NextY), seguiremos moviendonos.
  while(!(abs(NextX-*X)<d && abs(NextY-*Y)<d)){
    AuxX=Auxiliary(*X, Trit, 0, d);
    AuxY=Auxiliary(*Y, Trit, 1, d);
    MoveLinear(&*X, &*Y, AuxX, AuxY, NextX, NextY, m, &Infinity, &CounterInfinity, Trit);
  }
}


// Esta funcion se encarga de decidir si vamos a mover un paso en el eje X o en el eje Y.
// Para seguir una trayectoria correcta, debemos analizar los posibles dos movimientos (esto gracias al sistema de trits, sino tendiramos que analizar 4, dos por cada coordenada), entonces debemos analizar cual pendiente se acerca mas a la que debemos realizar.
// Para esto debemos calcular la pendiente si movieramos el eje X y la pendiente si movieramos el eje Y. Luego se comparan y se procede a mover el eje para la que este mas cerca de la real (m).
// Para los casos particulares, como con la pendiente infinita, hay unos condicionales para evitarnos tener numeros infinitos.
// Existen casos para los que las pendientes auxiliares darian infinitas. Tambien existen casos en que falta un solo movimiento para llegar al objetivo pero no se logra por algun motivo. Para estos casos se utilizan los ultimos dos ifs de esta funcion.
// Se agrego una variable llamada Mem, que se utiliza para tomar decisiones cuando ambas pendientes auxiliares tienen el mismo valor. Esto rompera el empate moviendo el eje que NO se movio la ultima vez.
// Mem=1 significa que el ultimo movimiento realizado fue en el eje Y.

void MoveLinear(float *X, float *Y, float AuxX, float AuxY, float NextX, float NextY, float m, int *Infinity, int *CounterInfinity, int Trit[]){
  float m1; // Pendiente si movieramos X.
  float m2; // Pendiente si movieramos Y.
  int Mem=1; // Memoria del ultimo movimiento.
  
  if(*Infinity==1){
    MoveY(Trit[1]);
    *Y= AuxY;
  }
  else if(*CounterInfinity==1){
    MoveX(Trit[0]);
    *X= AuxX;
  }
  else if(Trit[1]==2){ // No nos movemos en Y.
    MoveX(Trit[0]);
    *X= AuxX;
  }
  else{
    // Calculamos las pendientes auxiliares.
    m1= Slope(AuxX, *Y, NextX, NextY);
    m1= abs(m-m1);
    m2= Slope(*X, AuxY, NextX, NextY);
    m2= abs(m-m2);

    if(m1==m2){
      if(Mem==0){
        MoveY(Trit[1]);
        *Y= AuxY;
        Mem=1;
      }
      else{ //Mem==1
        MoveX(Trit[0]);
        *X= AuxX;
        Mem=0;
      }
    }
    else if (m1<m2){
      MoveX(Trit[0]);
      *X= AuxX;
      Mem=0;
    }
    else{ //m2<m1
      MoveY(Trit[1]);
      *Y = AuxY;
      Mem=1;
    }
    
    if(abs(NextX-AuxX)<d && Mem == 0){ // Para este caso la pendiente nos daria infinita tambien.
      *Infinity=1;
    }
    if(abs(NextY-AuxY)<d && Mem == 1){ 
      *CounterInfinity=1;
    }
  }
}

// Esta funcion se encarga de controlar los movimientos circulares. En esta funcion se calcula el radio respectivo a la trayectoria que realizaremos.
// Con las variables I y J podemos conocer el centro de giro (ver explicacion en la funcion Ratio). Este centro de giro estara en la posicion (Xr,Yr).
void RotateSet(float *X, float *Y, float I, float J, float NextX, float NextY, float PrevX, float PrevY, int Trit[],float d){
  float AuxX;
  float AuxY;
  float Xr;
  float Yr;
  float r;

  r=Ratio(I,J);
  
  // (Xr,Yr) Posicion del centro de la circunferencia.
  
  Xr=PrevX+I;
  Yr=PrevY+J;
  
  while(!(abs(NextX-*X)<d && abs(NextY-*Y)<d)){
    AuxX=Auxiliary(*X, Trit, 0, d);
    AuxY=Auxiliary(*Y, Trit, 1, d);
    Rotate(&*X, &*Y, Xr, Yr, AuxX, AuxY, Trit, r);
    // Patch: Existen casos muy raros donde el radio preferible conlleva a movernos mas alla de las coordenadas deseadas. Esto lo interrumpiremos asignando el correspondiente Trit igual a 2.
    if(abs(NextX-*X)<d) Trit[0]=2;
    if(abs(NextY-*Y)<d) Trit[1]=2;
  }
}

// Esta funcion se encarga de decidir si vamos a mover un paso en el eje X o en el eje Y en movimientos circulares.
// Para seguir una trayectoria correcta, debemos analizar los posibles dos movimientos (esto gracias al sistema de trits, sino tendiramos que analizar 4, dos por cada coordenada), entonces debemos analizar cual radio se acerca mas al inicial.
// Para esto debemos calcular la el radio si movieramos el eje X y el radio si movieramos el eje Y. Luego se comparan y se procede a mover el eje para la que este mas cerca del radio original.

void Rotate(float *X, float *Y, float Xr, float Yr, float AuxX, float AuxY, int Trit[], float r){
  float r1;     // Auxiliar de radio si movemos X.
  float r2;     // Auxiliar de radio si movemos Y.

  r1= Ratio((AuxX-Xr),(*Y-Yr));
  r1= abs(r-r1);

  r2= Ratio((*X-Xr),(AuxY-Yr));
  r2= abs(r-r2);

  if(r1<=r2 && !(Trit[0]==2)){
    MoveX(Trit[0]);
    *X=AuxX;
  }
  else if(r1>r2 && !(Trit[1]==2)){
    MoveY(Trit[1]);
    *Y=AuxY;
  }
  else{     // En caso de que r2>r1 y Trit[0]==2.
    if(!(Trit[0]==2)){
        MoveX(Trit[0]);
        *X=AuxX;
    }
    if(!(Trit[1]==2)){
        MoveY(Trit[1]);
        *Y=AuxY;
    }
  }
}




