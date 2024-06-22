#include <TimeLib.h>                                  // Libreria de tiempo
#include <SPI.h>                                      // Libreria para la comunicacion SPI
#include <Wire.h>                                     // Libreria para la comunicacion I2C
#include <Adafruit_GFX.h>                             // Libreria para graficar en la pantalla OLED
#include <Adafruit_SH1106.h>                          // Libreria para el uso de la pantalla OLED SH1106
#include <Ethernet.h>                                 // Libreria para el uso de red


#define OLED_RESET 4

Adafruit_SH1106 display(OLED_RESET);                  // Creacion de pantalla

byte mac[] = {                                        // Esta direccion IP debe ser cambiada obligatoriamente...
0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };                 // ...dependiendo de la subred de su Area Local y es la que...
IPAddress ip{10,37,1,5};                         // ...usara para conectarse por el Navegador.         
IPAddress subnet{255,255,255,0};
IPAddress gateway{10,37,1,250};

EthernetServer server(80);                            // Puerto 80 por defecto para HTTP  

const int pressureInput = A0;                         // Selecciona la entrada analoga que se utiliza del arduino para el sensor de presion
const int pressureZero = 102.4;                       // Lectura de presion en 0 psi
const int pressureMax = 921.6;                        // Lectura maxima de presion en 100 psi
const int pressuretransducermaxPSI = 100;             // Valor maximo de lectura del sensor
const int baudRate = 9600;                            // Constante para comunicacion utilizando serial de pantalla
const int sensorreadDelay = 500;                      // Constante para mantener la informacion en el display por tiempo limitado
const int puertoAnalogos = 0;
float pressureValue = 0;                              // Variable para almacenar el valor proveniente del transductor de presión.

void(* resetFunc)(void) = 0;


void setup() {
  Ethernet.begin(mac, ip, gateway, subnet);           // Inicializa la conexion Ethernet y el servidor
  server.begin();
  
  Serial.begin(9600);                                 // Inicia comunicacion Serial
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);           // Inicia pantalla
}


void loop() {
    /*__________________________________________________________________________________
      ___________Codigo para mostrar informacion desde IP de monitoreo__________________
      __________________________________________________________________________________*/

  EthernetClient cliente = server.available();          // Inicializa cliente como servidor ethernet
  if (cliente) {
    boolean currentLineIsBlank = true;
    while (cliente.connected()) {
      if (cliente.available()) {
        char c = cliente.read();
        if (c == '\n' && currentLineIsBlank) { 
          cliente.println("HTTP/1.1 200 OK");
          cliente.println("Content-Type: text/html");  // Envia el encabezado en codigo HTML estandar
          cliente.println("Connection: close"); 
          cliente.println("Refresh: 10");              // refresca la pagina automaticamente cada x segundos
          cliente.println();
          cliente.println("<!DOCTYPE HTML>"); 
          cliente.println("<html>");
          cliente.println("<HEAD>");
          cliente.println("<TITLE>Ethernet Monitor</TITLE>");
          cliente.println("</HEAD>");
          cliente.println("<BODY>");
          cliente.println("<hr />");
          cliente.println("<H1><center>Arduino Monitoreo de Presurizadores<center></H1>");
          cliente.println("<br />");    
          cliente.println("<br />");

          /*__________Codigo para mostrar valor de entrada analoga de arduino__________*/
          for (int puertoAnalogo = 0; puertoAnalogo < 1; puertoAnalogo++) {
            int lecturaSensor = analogRead(puertoAnalogo);   // Lee los 6 puertos analogos de A0 a A5
            cliente.print("Entrada Analoga ");
            cliente.print(puertoAnalogo + 1);
            cliente.print(lecturaSensor);
            cliente.println("<br />");       
          }

          /*__________Codigo para mostrar valor de entrada analoga como voltaje__________*/
          cliente.println("<br />");
          for (int puertoAnalogo = 0; puertoAnalogo < 1; puertoAnalogo++) {
            float lecturaSensor = analogRead(puertoAnalogo);   // Lee los 6 puertos analogos de A0 a A5
            float volts = (4.5*lecturaSensor/1023.0);
            cliente.print("Voltaje sensor ");
            cliente.print(puertoAnalogo + 1);
            cliente.print(volts);
            cliente.print(" V");
            cliente.println("<br />");       
          }
          
          /*__________Codigo para mostrar valor de entrada analoga en unidades de presion psi__________*/
          cliente.println("<br />");
          cliente.println("<span id='httpsensor'>"); 
          cliente.print("Presion: [");
          //cliente.print(pressureValue, 1);
          cliente.print(pressureValue);
          cliente.print("]");
          cliente.println("</span>");
          cliente.println(" psi");
          cliente.println("<br />");
          cliente.println("<br />");
          cliente.println("<hr />"); 
          cliente.println("Consorcio Canales Nacionales Privados CCNP"); 
          cliente.println("</html>");
          break;
        }
        if (c == '\n') {
           currentLineIsBlank = true;
        } 
        else if (c != '\r') {
           currentLineIsBlank = false;
        }
      }
    }
   delay(15);                                         // Da tiempo al Servidor para que reciba los datos 15ms
   cliente.stop();                                    // cierra la conexion
  }
  
    /*__________________________________________________________________________________
      ___________Codigo para mostrar informacion en display oled sh110__________________
      __________________________________________________________________________________*/

    
    /*__________Codigo para mostrar valor de entrada analoga en unidades de presion psi__________*/
    
    pressureValue = analogRead(pressureInput);        // Lee valor de entrada y asigna variable
    
    pressureValue = ((((pressureValue-pressureZero)*pressuretransducermaxPSI)/(pressureMax-pressureZero))*1.92); 
                                                      // Ecuacion de conversion para leer señal analoga como psi
                                                      
    Serial.print(pressureValue, 1);                   // Imprime valor previo a la linea de serial
    Serial.println("psi");                            // Imprime etiqueta al serial

    if(pressureValue < 0){                            // En caso de recibir valor negativo se muestra en 0 la presion
      pressureValue = 0;
      }

    display.clearDisplay();                           // Limpiamos pantalla
    display.setTextSize(1);                           // Tamaño del texto  
    display.setTextColor(WHITE);                      // Color WHITE para que se vea el texto, el color sera el de la pantalla
    display.setCursor(0,0);                           // Posición del texto en puntos de la pantalla
    display.println("beta sensor");  
    display.print("Presion: ");                                      
    display.print(pressureValue, 1);
    display.print(" psi");                           
    display.println("  ");                            


    /*__________Codigo para mostrar valor de entrada analoga de arduino__________*/
    int lecturaSensor = analogRead(puertoAnalogos);   // Lee el puertos analogo de A0
    display.println("  ");  
    display.print("Entrada Analoga ");
    display.println(puertoAnalogos + 1);
    display.print("A0 es: ");
    display.print(lecturaSensor);
    display.println("  "); 
    
     /*__________Codigo para mostrar valor de entrada analoga como voltaje__________*/
    float voltajeSensor = (4.5*lecturaSensor/1023.0); //4.5 es el voltaje maximo de sensor
    display.println("  "); 
    display.print("Voltaje sensor ");
    display.println(puertoAnalogos + 1);
    display.print("A0 es: ");
    display.print(voltajeSensor);
    display.print(" V");
 
    display.display();                                // Ingresar la información en el display para mostrarla
    delay(sensorreadDelay);                           // Llamado de constante delay

     
     //Instancia de reloj de reset
     time_t t = now();
  
    Serial.println("La hora es: ");
    Serial.println(hour(t));
    Serial.println("Los minutos son: ");
    Serial.println(minute(t));
    Serial.println("Los segundos son: ");
    Serial.println(second(t));

    if(minute()==30){
      resetFunc();
    }
}
