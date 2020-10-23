#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Servo.h>

//Configuracion del Keypad
const byte filas = 4;
const byte columnas = 4;
char keys [filas][columnas]={
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte pinesFilas[filas] = {22,24,26,28};
byte pinesColumnas[columnas] = {30,32,34,36};
Keypad teclado = Keypad(makeKeymap(keys),pinesFilas,pinesColumnas,filas,columnas);
//Cierre de la configuracion del Keypad

//Configuracion de los servos
Servo servoIzquierda;
Servo servoDerecha;
int servo1 = 41;
int servo2 = 43;
//Cierre de la configuracion de los servos

//Configuracion del lcd
LiquidCrystal lcd(23,25,27,29,31,33);
//Cierre de la configuracion del lcd

//Configuracion del modulo Bluetooth
SoftwareSerial Bt(0,1);
//Cierre de la configuracion Bluetooth

//Configuracion del sensor ultrasonico
int trig = 2;
int eco = 3;
//Cierre de la configuracion del sensor ultrasonico

//Configuracion de los botones
int boton1 = 20;
int boton2 = 21;
//Cierre de la configuracion de los botones

//Configuracion del modulo L298N
int in1 = 8;
int in2 = 9;
int ena = 10;
//Cierre de la configuracion del modulo L298N

//Configuracion del buzzer
int buzzer = A0;
//cierre de la configuracion del buzzer

//Configuracion del SW420
int sw420 = 19;
//Cierre de la configuracion del SW420

//Variables de control de metodos de apertura de los servos
int exitoServo1 = 0;
int exitoServo2 = 0;
//Cierre de variables de control de metodos de apertura de los servos

int contador = 3; //Variable que cambia el menu del lcd
int valorServo1 = 0, valorServo2 = 0; //Variables del estado abierto/cerrado de los servos
float duracion = 0; //Variable del tiempo que dura en hacerce el rebote de la sensor ultrasonico
float distancia; //Variable que almacena el calculo de la distancia /58.2
char caracter; //Variable que captura el teclado matricial
int modoServo1 = 2, modoServo2 = 2; //Variables para controlar el modo manual o modo automatico de las compuertas
int menu = 0; //Variable que dicta cual menu mostrar
float compuertaIzquierda = 100, compuertaDerecha = 0; //Variables que determinan la abertura de las compuertas
String caracteres = ""; //Variable que almacena una o varias variables 'caracter' 
int indice = 0; //Variable que indica si ya hay que mandar la data de 'caracteres' al servo especificado
unsigned long tiempoInicial, tiempoPasado; //Variables que miden el tiempo
int lcdOnOff = 0; //Variables que determinan cuando se enciende el lcd mediante el display
int valorBuzzer = 0; //Variable que determina la potencia del buzzer
int valorDistancia; //Variable que controla la distancia del sensor de ultrasonido con calculos matematicos
int valorBuzzerSound; //Variable que determina el sonido del buzzer

void setup() {
  lcd.begin(16,2);
  Serial.begin(9600);
  Bt.begin(38400);
  servoIzquierda.attach(servo1); //Pin del servo Izquierdo
  servoDerecha.attach(servo2); //Pin del servo Derecho
  pinMode(in1, OUTPUT); //Declaracion de pin In1
  pinMode(in2, OUTPUT); //Declaracion de pin In2
  pinMode(ena, OUTPUT); //Declaracion de pin ena 
  pinMode(trig, OUTPUT); //Declaracion de pin trig
  pinMode(eco, INPUT); //Declaracion de pin eco
  pinMode(buzzer, OUTPUT); //Declaracion de pin buzzer
  pinMode(boton1,INPUT); //Declaracion de pin boton1
  pinMode(boton2, INPUT); //Declaracion de pin boton2
  pinMode(sw420, INPUT); //Declaracion de pin sw420  
  pinMode(buzzer, OUTPUT); //Declaracion de pin buzzer
  tiempoInicial = millis(); //inicia el tiempo
  servoIzquierda.write(0); //Inicializa los servos en su posicion inicial
  servoDerecha.write(0); //Inicializa los servos en su posicion inicial
}

void loop() {

  //Configuracion para ver la cantidad de tiempo transacurrida
  tiempoPasado = millis();
  if(tiempoPasado - tiempoInicial >= 1000){
    tiempoInicial = tiempoPasado;
  }
  //Cierre de la configuracion para ver la cantidad de tiempo transcurrida
  
  //Lee la distancia del nivel del agua
  digitalWrite(trig, HIGH);
  delay(500);
  digitalWrite(trig, LOW);
  duracion = pulseIn(eco, HIGH);
  distancia = duracion / 58.2;
  //Cierre de la lectura del nivel del agua
  
  //Lectura del SW420
  Serial.print((11.32 - distancia)*10);
  if(digitalRead(sw420)==HIGH)
  {
      lcd.clear();
      for(int i = 0; i < 5; i++)
      {
        lcd.display();
        lcd.setCursor(0,0);
        lcd.print("Alerta Sismo!!");
        analogWrite(buzzer,1000);
        delay(1000);
        lcd.noDisplay();
        delay(1000);
      }
      analogWrite(buzzer,0);
      lcd.display();
  }
  //Cierre de la lectura del SW420

  //Lectura de los botones
  if(!digitalRead(boton1))
  {
    contador = 0;
    lcd.clear();
  }

  if(!digitalRead(boton2))
  {
    contador = 1;
    lcd.clear();
  }
  //Cierre de la lectura de los botones

  //Modo de los servos
  if(modoServo1 == 0) //Modo automatico servo1
  {
    if(distancia <= 3){
      MoverServo1(0);
    }
    else if(distancia > 3 )
    {
      MoverServo1(80);
    }
  }

  if(modoServo2 == 0) //Modo automaticco servo2
  {
    if(distancia <= 3){
      MoverServo2(100/1.25 + 20);
    }
    else{
      MoverServo2(0);
    }
  }

  if(modoServo1 == 1) //Modo manual servo1
  {
    if(exitoServo1 == 1)
    {
      MoverServo1((100-compuertaIzquierda)/1.25);
    }
  }

  if(modoServo2 == 1) //Modo manual servo2
  {
    if(exitoServo2 == 1)
    {
      MoverServo2(compuertaDerecha/1.25 + 20);
    }
  }
  //Cierre de los modos de los servos

  if((11.32 - distancia)*10 < 0)
  {
     valorDistancia = 0;
  }
  else{
    valorDistancia = (11.32 - distancia)*10;
  }

  //Menu principal
  if(contador == 0 && menu == 0)
  {
    lcd.setCursor(2,0);
    lcd.print("Nivel del agua: ");
    lcd.print(valorDistancia);
    lcd.print("%");
    lcd.setCursor(2,1);
    lcd.print("Servo1: ");
    lcd.print(compuertaIzquierda);
    lcd.print("%");
    lcd.print("      Servo2: ");
    lcd.print(compuertaDerecha);
    lcd.print("%");
    if(tiempoPasado - tiempoInicial >= 500){
        tiempoInicial = tiempoPasado;
        lcd.scrollDisplayLeft();
       }
  }
  //Cierre de menu principal
  
  //Menu de configuracion de los servos
  if(contador == 1 && menu == 0)
  {
    lcd.setCursor(2,0);
    lcd.print("Elija el servo a configurar: ");
    lcd.setCursor(2,1);
    lcd.print("Servo elegido:");
    lcd.setCursor(16,1);
    if(tiempoPasado - tiempoInicial >= 500){
        tiempoInicial = tiempoPasado;
        lcd.scrollDisplayLeft();
       }
    caracter = teclado.getKey();
    
    if(caracter)
    {
      lcd.print(caracter);
      delay(500);
      if(caracter == '1')
      {
        menu = 1;
        lcd.clear();
      }

      if(caracter == '2')
      {
        menu = 2;
        lcd.clear();
      }
    }
  }
  //Cierre de menu de configuracion de los servos

    //Menu del servoIzquierdo
    if(menu == 1)
    {
        lcd.setCursor(2,0);
        lcd.print("1)Automatico   2)Manual");
        lcd.setCursor(2,1);
        lcd.print("Servo1: ");
        if(tiempoPasado - tiempoInicial >= 500){
        tiempoInicial = tiempoPasado;
        lcd.scrollDisplayLeft();
       }
        caracter = teclado.getKey();
        if(caracter)
        {
          lcd.setCursor(8,1);
          lcd.print(caracter);
          if(caracter == '1')
          {
            modoServo1 = 0;
            menu = 0;
            lcd.clear();
          }

          if(caracter == '2')
          {
            menu = 3;
            lcd.clear();
          }
        }
    }
    //Cierre del menu del servoIzquierdo

    //Menu del servoDerecho
    if(menu == 2)
    {
        lcd.setCursor(2,0);
        lcd.print("1)Automatico   2)Manual");
        lcd.setCursor(2,1);
        lcd.print("Servo2: ");
        if(tiempoPasado - tiempoInicial >= 500){
        tiempoInicial = tiempoPasado;
        lcd.scrollDisplayLeft();
       }
        caracter = teclado.getKey();
        if(caracter)
        {
          lcd.setCursor(10,1);
          lcd.print(caracter);
          if(caracter == '1')
          {
            modoServo2 = 0;
            menu = 0;
            lcd.clear();
          }

          if(caracter == '2')
          {
            menu = 4;
            lcd.clear();
            
          }
        }
    }
    //Cierre del menu del servoIzquierdo

    //Menu de configuracion manual del servoIzquierdo
    if(menu == 3)
    {
      modoServo1 = 1;
      lcd.setCursor(2,0);
      lcd.print("Escriba el porcentaje del servo:");
      if(tiempoPasado - tiempoInicial >= 500){
        tiempoInicial = tiempoPasado;
        lcd.scrollDisplayLeft();
       }
      lcd.setCursor(2,1);
      caracter = teclado.getKey();
      if(caracter)
      {
        lcd.clear();
        caracteres = caracteres + caracter;
        lcd.setCursor(3+indice,1);
        lcd.print(caracteres);
        switch(caracter)
        {
          case '*':
            menu = 0;
            contador = 1;
            caracteres = "";
            exitoServo1 = 1;
            indice = 0;
            lcd.clear();
            break;

          case '#':
            menu = 0;
            compuertaIzquierda = 0;
            indice = 0;
            caracteres = "";
            exitoServo1 = 1;
            delay(500);
            break;

          case 'A':
            menu = 0;
            compuertaIzquierda = 100;
            caracteres = "";
            indice = 0;
            exitoServo1 = 1;
            delay(500);
            break;

          case 'B':
            menu = 0;
            compuertaIzquierda = 75;
            caracteres = "";
            exitoServo1 = 1;
            indice = 0;
            delay(500);
            break;

          case 'C':
            menu = 0;
            compuertaIzquierda = 50;
            caracteres = "";
            indice = 0;
            exitoServo1 = 1;
            delay(500);
            break;

          case 'D':
            menu = 0;
            compuertaIzquierda = 25;
            caracteres = "";
            exitoServo1 = 1;
            indice = 0;
            delay(500);
            break;

          default:
            indice++;
            break;
        }
        if(indice == 3 && caracteres.toInt() < 101)
        {
         compuertaIzquierda = caracteres.toInt();
         caracteres = "";
         exitoServo1 = 1;
         menu = 0;
         indice = 0;
         lcd.clear();
        }
        else if(indice == 3 && caracteres.toInt() > 100)
        {
         lcd.clear();
         lcd.print("No se puede exceder de un valor de 100%");
         caracteres = "";
         delay(1000);
         indice = 0;
         lcd.clear();
        }
      }
    }
    //Cierre del menu de configuracion manual del servoIzquierdo

    //Menu de congfiguracion manual del servoDerecho
    if(menu == 4)
    {
      modoServo2 = 1;
      lcd.setCursor(2,0);
      lcd.print("Escriba el porcentaje del servo:");
      if(tiempoPasado - tiempoInicial >= 500){
        tiempoInicial = tiempoPasado;
        lcd.scrollDisplayLeft();
       }
      lcd.setCursor(2,1);
      caracter = teclado.getKey();
      if(caracter)
      {
        lcd.clear();
        caracteres = caracteres + caracter;
        lcd.setCursor(3+indice,1);
        lcd.print(caracteres);
        switch(caracter)
        {
          case '*':
            menu = 0;
            contador = 1;
            caracteres = "";
            indice = 0;
            lcd.clear();
            break;

          case '#':
            menu = 0;
            compuertaDerecha = 0;
            indice = 0;
            caracteres = "";
            exitoServo2 = 1;
            delay(500);
            break;

          case 'A':
            menu = 0;
            compuertaDerecha = 100;
            caracteres = "";
            indice = 0;
            exitoServo2 = 1;
            delay(500);
            break;

          case 'B':
            menu = 0;
            compuertaDerecha = 75;
            caracteres = "";
            exitoServo2 = 1;
            indice = 0;
            delay(500);
            break;

          case 'C':
            menu = 0;
            compuertaDerecha = 50;
            caracteres = "";
            indice = 0;
            exitoServo2 = 1;
            delay(500);
            break;

          case 'D':
            menu = 0;
            compuertaDerecha = 25;
            caracteres = "";
            exitoServo2 = 1;
            indice = 0;
            delay(500);
            break;

          default:
            indice++;
            break;
        }
        if(indice == 3 && caracteres.toInt() < 101)
        {
         compuertaDerecha = caracteres.toInt();
         caracteres = "";
         exitoServo2 = 1;
         menu = 0;
         indice = 0;
         lcd.clear();
        }
        else if(indice == 3 && caracteres.toInt() > 100)
        {
         lcd.clear();
         lcd.print("No se puede exceder de un valor de 100%");
         caracteres = "";
         delay(1000);
         indice = 0;
         lcd.clear();
        }
      }
    }
  }
  //Cierre del menu de configuracion manual del servoDerecho

  //Metodo de apertura del servoIzquierdo
  void MoverServo1(int valor){
    exitoServo1 = 0;
    lcd.clear();
    int i = servoIzquierda.read();
    lcd.setCursor(0,0);
    lcd.print("Compuerta 1!!!");
    if(valor > i){
      for(i; i <= valor; i++){
        servoIzquierda.write(i);
        if(tiempoPasado - tiempoInicial > 1000 && lcdOnOff == 0){
        lcd.noDisplay();
        lcdOnOff = 1;
        tiempoInicial = tiempoPasado;
        }
        if(tiempoPasado - tiempoInicial > 1000 && lcdOnOff == 1){
        lcd.display();
        lcdOnOff = 0;
        tiempoInicial = tiempoPasado;
        }
        delay(100);
       }
    }

    if(valor < i)
    {
      for(i; i >= valor; i--){
        servoIzquierda.write(i);
        if(tiempoPasado - tiempoInicial > 1000 && lcdOnOff == 0){
        lcd.noDisplay();
        lcdOnOff = 1;
        tiempoInicial = tiempoPasado;
        }
        if(tiempoPasado - tiempoInicial > 1000 && lcdOnOff == 1){
        lcd.display();
        lcdOnOff = 0;
        tiempoInicial = tiempoPasado;
        }
        delay(100);
       }
    }
    lcd.clear();
  }
  //Cierre del metodo de apertura del servoIzquierdo

  //Metodo de apertura del servoDerecho
  void MoverServo2(int valor){
    exitoServo2 = 0;
    lcd.clear();
    int j = servoDerecha.read();
    lcd.setCursor(0,0);
    lcd.print("Compuerta 2!!!");
    if(valor > j){
      for(j; j <= valor; j++){
        servoDerecha.write(j);
        if(tiempoPasado - tiempoInicial > 1000 && lcdOnOff == 0){
        lcd.noDisplay();
        lcdOnOff = 1;
        tiempoInicial = tiempoPasado;
        }
        if(tiempoPasado - tiempoInicial > 1000 && lcdOnOff == 1){
        lcd.display();
        lcdOnOff = 0;
        tiempoInicial = tiempoPasado;
        }
        delay(100);
       }
    }

    if(valor < j)
    {
      for(j; j >= valor; j--){
        servoDerecha.write(j);
        if(tiempoPasado - tiempoInicial > 1000 && lcdOnOff == 0){
        lcd.noDisplay();
        lcdOnOff = 1;
        tiempoInicial = tiempoPasado;
        }
        if(tiempoPasado - tiempoInicial > 1000 && lcdOnOff == 1){
        lcd.display();
        lcdOnOff = 0;
        tiempoInicial = tiempoPasado;
        }
        delay(100);
       }
    }
    lcd.clear();
  }
  //Cierre del metodo de apertura del servoDereho
