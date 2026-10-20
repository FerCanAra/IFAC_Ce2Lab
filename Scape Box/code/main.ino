#include <Adafruit_GFX.h>         // Core graphics library
#include <Adafruit_ILI9341.h>     // Hardware-specific library
#include <SdFat.h>                // SD card & FAT filesystem library
#include <Adafruit_SPIFlash.h>    // SPI / QSPI flash library
#include <Adafruit_ImageReader.h> // Image-reading functions
#include <Keypad.h>
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

// Declaramos y configuramos el keypad
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {31, 29, 27, 25}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {30, 28, 26, 24}; //connect to the column pinouts of the keypad

Keypad myKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

// Declaramos pines el modulo tft y sd
#define TFT_RST 47
#define TFT_DC 46
#define TFT_CS 48
#define TFT_MOSI 51
#define TFT_MISO 50
#define TFT_CLK 52
#define SD_CS 49

// Declaramos pin buzzer
#define buzzer 8

SdFile file;
SdFat SD;

// Configuramos TFT
Adafruit_ImageReader reader(SD);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
ImageReturnCode stat; // Status from image-reading functions
int32_t   width  = 320, height = 240;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

// Estructura para almacenar las preguntas (nommbre del archivo y respuesta correcta)
struct QuizData {
  String question_file;
  int correct_answer;
  int digits_answer;
  int padlock_asocciated;
};
// Estructura para almacenar las respuestas (duracion e intentos)
struct QuizResult {
  unsigned duration;
  unsigned attempts;
};

// Tamaño del array
const int arraySize = 30;

// Crear el array de estructuras
QuizData quizArray[arraySize];
QuizResult statsArray[arraySize];

//-----------------------------------------------------------------------------------------------------------------------------------------------------//
//                                                                            FUNCIONES
//-----------------------------------------------------------------------------------------------------------------------------------------------------//
// Función encargada de guardar los resultados obtenidos en el juego en la tarjeta SD usando el nombre generado por la funcion Main_menu().
void save_stats (String file_name) {
  String Stats_fileName = file_name + ".CSV";
  Serial.println(Stats_fileName);
  file.open(Stats_fileName.c_str(), FILE_WRITE);
  if (file.isOpen()) {
    for (int i = 0; i < arraySize; i++) {
      file.print(i);
      file.print(",");
      file.print(statsArray[i].duration / 1000);
      file.print('.');
      file.print(statsArray[i].duration % 1000);
      file.print(",");
      file.println(statsArray[i].attempts);
    }
    tft.setCursor(0, 20);
    tft.setTextColor(ILI9341_YELLOW);  tft.setTextSize(2);
    // Close the file
    tft.println("Resultados guardados en SD");
    file.close();
    delay(3000);
  } else {
    tft.println("Error al guardar en SD");
    delay(3000);
  }
}
// Función encargada mostrar pantalla principal con las asignaturas disponibles, una vez seleccionada la asignatura realiza petición de número de grupo.
// (Devuelve un String con el nombre del archivo en el cual se guardarán los resultados del juego: ASIGNATURA_NUMERODEGRUPO)
String Main_menu (int &testSize) {
  String file_name;
  int group_num;

  stat = reader.drawBMP("main_menu.bmp", tft, 0, 0);
  if (stat == 1) {
    tft.writeFillRect(0, 0, width, 10, 0);
    tft.setTextColor(ILI9341_GREEN);  tft.setTextSize(2);
    tft.println("Error al leer el archivo");
    for (;;);
  }

  testSize = 0; // Aseguramos que la variable sea cero antes de realizar el bucle;
  while (testSize == 0) {
    char key = myKeypad.getKey();
    if (key) {
      key_sound();
      tft.fillScreen(0);
      switch (key) {
        case '1':
          testSize = load_test("Auto_test.CSV", quizArray);
          if (testSize != 0) {
            reader.drawBMP("group.bmp", tft, 0, 0);
            group_num = read_keypad(2);
            file_name = "AUTO_" + String(group_num);
          } else {
            reader.drawBMP("main_menu.bmp", tft, 0, 0);
          }
          break;
        case '2':
          testSize = load_test("TCI_test.CSV", quizArray);
          if (testSize != 0) {
            reader.drawBMP("group.bmp", tft, 0, 0);
            group_num = read_keypad(2);
            file_name = "TCI_" + String(group_num);
          } else {
            reader.drawBMP("main_menu.bmp", tft, 0, 0);
          }
          break;
        case '3':
          testSize = load_test("Regulacion_test.CSV", quizArray);
          if (testSize != 0) {
            reader.drawBMP("group.bmp", tft, 0, 0);
            group_num = read_keypad(2);
            file_name = "RI_" + String(group_num);
          } else {
            reader.drawBMP("main_menu.bmp", tft, 0, 0);
          }
          break;
        case '4':
          testSize = load_test("Control_test.CSV", quizArray);
          if (testSize != 0) {
            reader.drawBMP("group.bmp", tft, 0, 0);
            group_num = read_keypad(2);
            file_name = "C_" + String(group_num);
          } else {
            reader.drawBMP("main_menu.bmp", tft, 0, 0);
          }
          break;
        case '5':
          tft.setCursor(0, 0);
          tft.setTextColor(ILI9341_YELLOW);  tft.setTextSize(2);
          tft.println("Extraer");
          break;
      }
    }
  }
  return file_name;
}

//Funcion encargada de lanzar la pregunta correspondiente y almacenar el resulta (tiempo e intentos) en una variable tipo Struct (QuizResult).
QuizResult launch_quiz(int index) {
  // Convierte el objeto String a const char*
  const char* questionFile = quizArray[index].question_file.c_str();
  stat = reader.drawBMP(questionFile, tft, 0, 0);
  reader.printStatus(stat);

  if (stat == 1) {
    tft.setCursor(0, 0);
    tft.setTextColor(ILI9341_YELLOW);  tft.setTextSize(2);
    tft.fillScreen(0);
    tft.println("Error al leer la pregunta");
    tft.println("Compruebe que existe...");
    for (;;);
  }

  bool correct_answer = LOW;
  int selected_answer = 0;
  int value = 0;
  unsigned long  start_time = 0;
  unsigned  duration = 0;
  unsigned attempts = 1;

  run_sound();
  start_time = millis();

  while (!correct_answer) {
    selected_answer = read_keypad(quizArray[index].digits_answer);
    if (selected_answer == quizArray[index].correct_answer) {
      correct_sound();
      duration = (millis() - start_time);
      correct_answer = HIGH;
    } else {
      incorrect_sound();
      attempts++;
    }
  }
  QuizResult result = {duration, attempts};
  return result;
}

// Función encargada de leer el archivo CSV y rellenar el array de struct(QuizData) llamado QuizArray que contiene el nombre del bmp correspondiente a las preguntas, la respuesta correcta y el número de dígitos de la pregunta.
//(Devuelve TRUE si se ha conseguido cargar correctamente)

int load_test(const char *fileName, QuizData *quizArray) {
  Serial.println(fileName);
  // Abrir el archivo en modo lectura
  if (!file.open(fileName, O_READ)) {
    tft.setCursor(0, 0);
    tft.setTextColor(ILI9341_YELLOW);  tft.setTextSize(2);
    tft.fillScreen(0);
    tft.println("Error al abrir el archivo.");
    tft.println("Compruebe que el existe...");
    delay(3000);
    return LOW;
  }
  // Índice para recorrer el array
  int index = 0;

  // Buffer para leer una línea del archivo
  char buffer[100];

  // Leer el contenido del archivo línea por línea
  while (file.available()) {
    // Leer una línea
    file.fgets(buffer, sizeof(buffer));

    // Separar los campos usando el separador ";"
    String line(buffer);
    int separatorIndex1 = line.indexOf(';');
    int separatorIndex2 = line.indexOf(';', separatorIndex1 + 1); // Buscar el segundo ';'
    int separatorIndex3 = line.indexOf(';', separatorIndex2 + 1); // Buscar el tercer ';'

    if (separatorIndex1 != -1 && separatorIndex2 != -1 && separatorIndex3 != -1) {
      // Obtener los valores de los campos
      String questionFile = line.substring(0, separatorIndex1);
      int correctAnswer = line.substring(separatorIndex1 + 1, separatorIndex2).toInt();
      int digitsAnswer = line.substring(separatorIndex2 + 1, separatorIndex3).toInt();
      int padlock = line.substring(separatorIndex3 + 1).toInt();

      // Asignar los valores al array de estructuras
      quizArray[index].question_file = questionFile;
      quizArray[index].correct_answer = correctAnswer;
      quizArray[index].digits_answer = digitsAnswer;
      quizArray[index].padlock_asocciated = padlock;
      Serial.println("Elemento " + String(index) + ": question_file: " + quizArray[index].question_file + ", correct_answer: " + String(quizArray[index].correct_answer) + ", digits_answer: " + String(quizArray[index].digits_answer) + ", padlock: " + String(quizArray[index].padlock_asocciated));
      // Incrementar el índice
      index++;
    }
  }
  // Cerrar el archivo
  file.close();
  return index;
}

void run_sound() {
  tone(buzzer, 2000);
  delay(80);
  noTone(buzzer);
  delay(50);
  tone(buzzer, 3000);
  delay(50);
  noTone(buzzer);
}

void key_sound() {
  tone(buzzer, 3000);
  delay(50);
  noTone(buzzer);
}

void incorrect_sound() {
  tone(buzzer, 800);
  delay(300);
  noTone(buzzer);
}
void correct_sound() {
  tone(buzzer, 1200); // Tono 1
  delay(100);
  noTone(buzzer);
  delay(10);

  tone(buzzer, 1500); // Tono 2
  delay(100);
  noTone(buzzer);
  delay(10);

  tone(buzzer, 1700); // Tono 3
  delay(100);
  noTone(buzzer);
  delay(10); // Pausa entre la secuencia

  tone(buzzer, 2000); // Tono 1
  delay(300);
  noTone(buzzer);
  delay(10);
}


void delete_option_selected() {
  tft.fillCircle(15, 125, 4, 0xFFFF );
  tft.fillCircle(15, 142, 4, 0xFFFF);
  tft.fillCircle(15, 157, 4, 0xFFFF);
  tft.fillCircle(15, 172, 4, 0xFFFF);
}

void draw_option_selected(int answer_value) {

  switch (answer_value) {
    case 1:
      tft.fillCircle(15, 125, 4, 0x0400 );
      tft.fillCircle(15, 142, 4, 0xFFFF);
      tft.fillCircle(15, 157, 4, 0xFFFF);
      tft.fillCircle(15, 172, 4, 0xFFFF);
      break;

    case 2:
      tft.fillCircle(15, 142, 4, 0x0400 );
      tft.fillCircle(15, 125, 4, 0xFFFF);
      tft.fillCircle(15, 157, 4, 0xFFFF);
      tft.fillCircle(15, 172, 4, 0xFFFF);

      break;

    case 3:
      tft.fillCircle(15, 157, 4, 0x0400 );
      tft.fillCircle(15, 125, 4, 0xFFFF);
      tft.fillCircle(15, 142, 4, 0xFFFF);
      tft.fillCircle(15, 172, 4, 0xFFFF);

      break;

    case 4:
      tft.fillCircle(15, 172, 4, 0x0400 );
      tft.fillCircle(15, 125, 4, 0xFFFF);
      tft.fillCircle(15, 142, 4, 0xFFFF);
      tft.fillCircle(15, 157, 4, 0xFFFF);

      break;
  }
}

int read_keypad (int digits) {
  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(5);
  int  X_text = 180 - 20 * digits;
  int  Y_text = 100;

  String numbers = "";
  char key = 0;

  while (true) {
    key = myKeypad.getKey();
    if (key) {
      if (key == 'A') {
        // Se presionó A, retornar el número actual
        key_sound();
        return numbers.toInt();
      } else if (key == 'B') {
        // Se presionó B, borrar el último dígito
        key_sound();
        if (numbers.length() > 0) {
          numbers.remove(numbers.length() - 1);
        }
      } else if (key == 'C') {
        // Se presionó C, borrar todo el número
        key_sound();
        numbers = "";
      } else {
        // Agregar dígitos solo si no se ha alcanzado el límite
        if (digits == 1) {
          if (atoi(&key) > 0 and atoi(&key) < 5) {
            key_sound();
            numbers = key;
          }
        } else {
          if (numbers.length() < digits) {
            key_sound();
            numbers += key;
          }
        }
      }
      Serial.println(numbers);
      if (digits > 1) {
        tft.fillRect(X_text, Y_text, 120, 35, ILI9341_WHITE); //Evitar digitos solapados en caso de borrar
        tft.setCursor(X_text, Y_text);
        tft.println(numbers);
      } else {
        delete_option_selected();
        draw_option_selected(numbers.toInt());
      }
    }
  }
}

void open_padlock(int index) {

}


void setup() {
  pinMode(buzzer, OUTPUT);
  Serial.begin(9600);
  tft.begin(1500000);
  tft.fillScreen(0);
  tft.setRotation(1);
  tft.setCursor(0, 0);
  tft.println(F("Proyecto de gamifizacion - Grupo ARM (TEP-197)"));
  tft.setCursor(0, 15);
  tft.print(F("Inicializando tarjeta SD..."));

  if (!SD.begin(SD_CS, SD_SCK_MHZ(1.5))) {
    tft.setCursor(170, 15);
    tft.setTextColor(ILI9341_RED);
    tft.println(F("SD begin failed"));
    for (;;); //Parar ejecución
  }
  tft.setCursor(170, 15);
  tft.setTextColor(ILI9341_GREEN);
  tft.println(F("OK!"));
  delay(1000);
  tft.fillScreen(ILI9341_WHITE);
}

void loop() {
  
  int index = 0; // Inicializamos el contador de preguntas del test.
  unsigned long total_duration = 0; // Inicializamos a 0 el tiempo total invertido en el test.
  int testSize = 0; // Declaramos el tamaño del test a 0, esperando que la funcion de Main_menu cargue el preguntas del test seleccionado.

  String stats_file_name = Main_menu(testSize); // LLama a la funcion y recibe el nombre del archivo con el que se va a guardar los resultados y testSize se modifica con el numero de preguntas del Test.

  while (index < testSize) {
    statsArray[index] = launch_quiz(index);
    open_padlock(index);
    //String msg = String(statsArray[index].duration) + " milisegundos transcurridos en la pregunta " + String(index) + ". Con un total de intentos de: " + statsArray[index].attempts + ". Total de tiempo en segundos transcurrido en el juego: " + String(total_duration / 1000);
    //Serial.println(msg);
    total_duration = statsArray[index].duration + total_duration;
    index++;
  }

  // Muestra el tiempo trancurrido en la realización del test completo
  tft.fillScreen(0);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_YELLOW);  tft.setTextSize(2);
  tft.print(F("Tiempo transcurrido: "));
  tft.println(total_duration / 1000);

  // Guarda en la SD los resultados en el archivo con nombre 'stats_file_name'
  save_stats (stats_file_name);
}
