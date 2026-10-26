/*

--- English ---

This project is a VRIW calculator (Volts, Resistance/Ohms, Current/Amps, Watts): the user chooses two
known quantities via two RadioGroups (row A and row B), enters the four values in NumberBoxes, then
presses one of four buttons to compute the third quantity. Each RadioGroup has four options: id 1 =
Volts, 2 = Ohms, 3 = Amps, 4 = Watts. The selected ids in grupo1 (radiog1) and grupo2 (radiog2) indicate
which two quantities are "inputs" for the calculation. Four NumberBox widgets (inputvolts, inputohms,
inputamps, inputwatts) have callbacks that store getValue() into voltsValue, resistenceValue, ampsValue
and wattsValue. Four TextButtons ("Calc V", "Calc R", "Calc I", "Calc W") call calculateVolts(),
calculateOhms(), calculateAmps() and calculateWatts() respectively. Each calculate function checks the
current radiog1/radiog2 combination: if it matches a valid pair (e.g. Ohms + Amps for Volts), it
computes the result with the corresponding formula, sets the result Label (txtresult) prefix/suffix
(e.g. "Result:", " V") and updates the text with setTextFloat; otherwise it shows "Invalid combination".
The Label thus displays the result with the correct unit. The screen title is "Calc VRIW"; labels "V",
"R", "I", "W" and "A", "B" identify the number boxes and radio rows. Display: ST7796, FT6336U, ESP32S3.
The loop only runs startCustomDraw/finishCustomDraw and delay(2000); all result updates happen in the
button callbacks.

The NumberBox widget is a numeric input that opens an on-screen numpad when touched; the user enters
a number and the callback is called with getValue() returning the float. Here the four NumberBoxes
store volts, ohms, amps and watts. The RadioGroup widget shows a set of radio options; only one can
be selected per group. When the user selects an option, the callback runs and getSelected() returns
the id of the chosen option (here 1=Volts, 2=Ohms, 3=Amps, 4=Watts). The two groups (A and B) indicate
which two quantities are the known inputs. The TextButton widget is a button with a text label; each
of the four buttons calls a calculate function that uses the radio selections and the four stored
values to compute the result. The Label widget displays the result with setTextFloat() or setText();
its prefix and suffix are set in the calculate functions so the unit (V, Ohms, Amps, Watts) is shown
correctly.

The equations used are listed in the block below.

--- Português ---

Este projeto é uma calculadora VRIW (Volts, Resistência/Ohms, Corrente/Amps, Watts): o usuário escolhe
duas grandezas conhecidas por meio de dois RadioGroups (linha A e linha B), informa os quatro valores
nos NumberBoxes e pressiona um dos quatro botões para calcular a terceira grandeza. Cada RadioGroup tem
quatro opções: id 1 = Volts, 2 = Ohms, 3 = Amps, 4 = Watts. Os ids selecionados em grupo1 (radiog1) e
grupo2 (radiog2) indicam quais duas grandezas são "entradas" para o cálculo. Quatro widgets NumberBox
(inputvolts, inputohms, inputamps, inputwatts) possuem callbacks que armazenam getValue() em
voltsValue, resistenceValue, ampsValue e wattsValue. Quatro TextButtons ("Calc V", "Calc R", "Calc I",
"Calc W") chamam calculateVolts(), calculateOhms(), calculateAmps() e calculateWatts() respectivamente.
Cada função de cálculo verifica a combinação atual de radiog1/radiog2: se corresponder a um par válido
(ex.: Ohms + Amps para Volts), calcula o resultado com a fórmula correspondente, define prefix/suffix do
Label de resultado (txtresult) (ex. "Result:", " V") e atualiza o texto com setTextFloat; caso contrário
exibe "Invalid combination". O Label mostra assim o resultado com a unidade correta. O título da tela é
"Calc VRIW"; as etiquetas "V", "R", "I", "W" e "A", "B" identificam as caixas numéricas e as linhas de
rádio. Display: ST7796, FT6336U, ESP32S3. O loop apenas executa startCustomDraw/finishCustomDraw e
delay(2000); todas as atualizações do resultado ocorrem nos callbacks dos botões.

O widget NumberBox é uma entrada numérica que abre um teclado numérico na tela ao ser tocada; o
usuário informa um número e o callback é chamado com getValue() retornando o float. Aqui os quatro
NumberBoxes armazenam volts, ohms, amps e watts. O widget RadioGroup exibe um conjunto de opções de
rádio; apenas uma pode estar selecionada por grupo. Quando o usuário escolhe uma opção, o callback
é executado e getSelected() retorna o id da opção escolhida (aqui 1=Volts, 2=Ohms, 3=Amps, 4=Watts).
Os dois grupos (A e B) indicam quais duas grandezas são as entradas conhecidas. O widget TextButton
é um botão com rótulo de texto; cada um dos quatro botões chama uma função de cálculo que usa as
seleções dos rádios e os quatro valores armazenados para calcular o resultado. O widget Label exibe
o resultado com setTextFloat() ou setText(); seu prefix e suffix são definidos nas funções de cálculo
para que a unidade (V, Ohms, Amps, Watts) seja exibida corretamente.

As equações utilizadas estão listadas no bloco abaixo.

*/

/*

RadioGroup:
- Id 1 = Volts
- Id 2 = Ohms
- Id 3 = Amps
- Id 4 = Watts


Input:
- voltsValue = Volts
- resistenceValue = Ohms
- ampsValue = Amps
- wattsValue = Watts

Equations:
Volts = Ohms * Amps
Volts = Watts / Amps
Volts = sqrt(Watts * Ohms)


Ohms = Volts / Amps
Ohms = Watts / (Amps * Amps)
Ohms = Volts * Volts / Watts

Amps = Volts / Ohms
Amps = Watts / Volts
Amps = sqrt(Watts / Ohms)

Watts = Volts * Amps
Watts = Ohms * Amps * Amps
Watts = Volts * Volts / Ohms

*/

#define ID_VOLTS 1
#define ID_OHMS 2
#define ID_AMPS 3
#define ID_WATTS 4


// Include for plugins of chip 0
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define VSPI FSPI
#endif
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>


    /* Project setup:
    * MCU: ESP32S3
    * Display: ST7796
    * Touch: FT6336U
    */
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 480;
const int DISPLAY_H = 320;
const int DISP_FREQUENCY = 27000000;
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_X1 = 320;
const int TOUCH_MAP_Y1 = 480;
const bool TOUCH_SWAP_XY = true;
const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = true;
const int DISP_MOSI = 11;
const int DISP_MISO = -1;
const int DISP_SCLK = 12;
const int DISP_CS = 10;
const int DISP_DC = 8;
const int DISP_RST = 9;
const int TOUCH_SCL = 13;
const int TOUCH_SDA = 14;
const int TOUCH_INT = 18;
const int TOUCH_RST = 17;
const uint8_t rotationScreen = 1; // This value can be changed depending of orientation of your screen
const bool isIPS = true; // Come display can use this as bigEndian flag

// Prototypes for each screen
void screen0();
void loadWidgets();

// Prototypes for callback functions
void inputvolts_cb();
void inputohms_cb();
void inputamps_cb();
void inputwatts_cb();
void btnvolts_cb();
void btnohms_cb();
void btnamps_cb();
void btnwatts_cb();
void callbackOfGroup_grupo1();
void callbackOfGroup_grupo2();

// Create global SPI object
#if defined(CONFIG_IDF_TARGET_ESP32S3)
SPIClass spi_shared(FSPI);
#else
SPIClass spi_shared(HSPI);
#endif
Arduino_DataBus *bus = nullptr;
Arduino_GFX *tft = nullptr;
DisplayWX myDisplay;
// Create global objects. Constructor is: xPos, yPos and indexScreen
Label txtresult(245, 275, 0);
const uint8_t qtdLabel = 1;
Label *arrayLabel[qtdLabel] = {&txtresult};
NumberBox inputvolts(15, 150, 0);
NumberBox inputohms(135, 150, 0);
NumberBox inputamps(255, 150, 0);
NumberBox inputwatts(370, 150, 0);
const uint8_t qtdNumberbox = 4;
NumberBox *arrayNumberbox[qtdNumberbox] = {&inputvolts, &inputohms, &inputamps, &inputwatts};
float voltsValue = 1; // Global variable that stores the value of the widget inputvolts
float resistenceValue = 1; // Global variable that stores the value of the widget inputohms
float ampsValue = 1; // Global variable that stores the value of the widget inputamps
float wattsValue = 1; // Global variable that stores the value of the widget inputwatts
RadioGroup grupo1(0);
RadioGroup grupo2(0);
const uint8_t qtdRadioGroup = 2;
RadioGroup *arrayRadio[qtdRadioGroup] = {&grupo1, &grupo2};
radio_t radiosOfGroup_grupo1[4] = {{65, 80, CFK_GREY1, 1},{185, 80, CFK_GREY1, 2},{300, 80, CFK_GREY1, 3},{420, 80, CFK_GREY1, 4}};
uint16_t radiog1 = 1; // Global variable that stores the value of the widget grupo1
radio_t radiosOfGroup_grupo2[4] = {{65, 120, CFK_GREY1, 1},{185, 120, CFK_GREY1, 2},{300, 120, CFK_GREY1, 3},{420, 120, CFK_GREY1, 4}};
uint16_t radiog2 = 1; // Global variable that stores the value of the widget grupo2
TextButton btnvolts(20, 205, 0);
TextButton btnohms(145, 205, 0);
TextButton btnamps(265, 205, 0);
TextButton btnwatts(380, 205, 0);
const uint8_t qtdTextButton = 4;
TextButton *arrayTextButton[qtdTextButton] = {&btnvolts, &btnohms, &btnamps, &btnwatts};

void setup(){

    Serial.begin(115200);
    // Start SPI object for display
    spi_shared.begin(DISP_SCLK, DISP_MISO, DISP_MOSI);
    bus = new Arduino_HWSPI(DISP_DC, DISP_CS, DISP_SCLK, DISP_MOSI, DISP_MISO, &spi_shared);
    tft = new Arduino_ST7796(bus, DISP_RST, rotationScreen, isIPS, 320, 480);
    tft->begin(DISP_FREQUENCY);
    myDisplay.setDrawObject(tft); // Reference to object to draw on screen
    // Setup touch
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    myDisplay.startTouchFT6336(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
    myDisplay.enableTouchLog();
    loadWidgets(); // This function is used to setup with widget individualy
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
}

void loop(){

    myDisplay.startCustomDraw();
    //txtresult.setTextFloat(-----, 2); //Use this command to change widget value.
    // Use the variable voltsValue to check value/status of widget inputvolts
    // Use the variable resistenceValue to check value/status of widget inputohms
    // Use the variable ampsValue to check value/status of widget inputamps
    // Use the variable wattsValue to check value/status of widget inputwatts
    // Use the variable radiog1 to check value/status of widget grupo1
    // Use the variable radiog2 to check value/status of widget grupo2
    myDisplay.finishCustomDraw();
    delay(2000);
}

void screen0(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    tft->fillRect(5, 100, 466, 35, CFK_GREY11);
    tft->drawRect(5, 100, 466, 35, CFK_BLACK);
    tft->fillRect(5, 60, 466, 35, CFK_GREY11);
    tft->drawRect(5, 60, 466, 35, CFK_BLACK);
    myDisplay.printText("Calc VRIW", 205, 12, TL_DATUM, CFK_COLOR01, CFK_WHITE, &RobotoRegular10pt7b);
    //This screen has a/an caixaNumero
    //This screen has a/an caixaNumero
    //This screen has a/an caixaNumero
    //This screen has a/an caixaNumero
    myDisplay.printText("V", 60, 37, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.printText("R", 180, 37, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.printText("I", 300, 37, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.printText("W", 410, 37, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    //This screen has a/an textButton
    //This screen has a/an textButton
    //This screen has a/an textButton
    //This screen has a/an textButton
    //This screen has a/an label
    //This screen has a/an radiobutton
    //This screen has a/an radiobutton
    //This screen has a/an radiobutton
    //This screen has a/an radiobutton
    //This screen has a/an radiobutton
    //This screen has a/an radiobutton
    //This screen has a/an radiobutton
    //This screen has a/an radiobutton
    myDisplay.printText("A", 15, 67, TL_DATUM, CFK_BLACK, CFK_GREY11, &RobotoRegular10pt7b);
    myDisplay.printText("B", 15, 107, TL_DATUM, CFK_BLACK, CFK_GREY11, &RobotoRegular10pt7b);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

    LabelConfig configLabel0 = {
            .text = "-----",
            .prefix = "Result: ",
            .suffix = "",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TC_DATUM,
            .fontColor = CFK_BLACK,
            .backgroundColor = CFK_WHITE
        };
    txtresult.setup(configLabel0);
    myDisplay.setLabel(arrayLabel,qtdLabel);


    Numpad::m_backgroundColor = CFK_GREY3;
    Numpad::m_letterColor = CFK_BLACK;
    Numpad::m_keyColor = CFK_GREY13;


    NumberBoxConfig configNumberBox0 = {
            .funcPtr = screen0,
            .callback = inputvolts_cb,
            .font = &RobotoRegular10pt7b,
            .startValue = voltsValue,
            .width = 101,
            .height = 25,
            .letterColor = CFK_GREY1,
            .backgroundColor = CFK_WHITE,
            .decimalPlaces = 2
        };
    inputvolts.setup(configNumberBox0);
    NumberBoxConfig configNumberBox1 = {
            .funcPtr = screen0,
            .callback = inputohms_cb,
            .font = &RobotoRegular10pt7b,
            .startValue = resistenceValue,
            .width = 103,
            .height = 25,
            .letterColor = CFK_GREY1,
            .backgroundColor = CFK_WHITE,
            .decimalPlaces = 2
        };
    inputohms.setup(configNumberBox1);
    NumberBoxConfig configNumberBox2 = {
            .funcPtr = screen0,
            .callback = inputamps_cb,
            .font = &RobotoRegular10pt7b,
            .startValue = ampsValue,
            .width = 103,
            .height = 25,
            .letterColor = CFK_GREY1,
            .backgroundColor = CFK_WHITE,
            .decimalPlaces = 2
        };
    inputamps.setup(configNumberBox2);
    NumberBoxConfig configNumberBox3 = {
            .funcPtr = screen0,
            .callback = inputwatts_cb,
            .font = &RobotoRegular10pt7b,
            .startValue = wattsValue,
            .width = 103,
            .height = 25,
            .letterColor = CFK_GREY1,
            .backgroundColor = CFK_WHITE,
            .decimalPlaces = 2
        };
    inputwatts.setup(configNumberBox3);
    myDisplay.setNumberbox(arrayNumberbox,qtdNumberbox);


    RadioGroupConfig configRadioGroup0 = {
            .buttons = radiosOfGroup_grupo1,
            .callback = callbackOfGroup_grupo1,
            .radius = 10,
            .group = 1,
            .amount = 4,
            .defaultClickedId = 1
        };
    grupo1.setup(configRadioGroup0);
    RadioGroupConfig configRadioGroup1 = {
            .buttons = radiosOfGroup_grupo2,
            .callback = callbackOfGroup_grupo2,
            .radius = 10,
            .group = 2,
            .amount = 4,
            .defaultClickedId = 1
        };
    grupo2.setup(configRadioGroup1);
    myDisplay.setRadioGroup(arrayRadio,qtdRadioGroup);


    TextButtonConfig configTextButton0 = {
            .text = "Calc V",
            .callback = btnvolts_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 87,
            .height = 34,
            .radius = 10,
            .backgroundColor = CFK_COLOR14,
            .textColor = CFK_BLACK
        };

    btnvolts.setup(configTextButton0);
    TextButtonConfig configTextButton1 = {
            .text = "Calc R",
            .callback = btnohms_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 87,
            .height = 34,
            .radius = 10,
            .backgroundColor = CFK_COLOR14,
            .textColor = CFK_BLACK
        };

    btnohms.setup(configTextButton1);
    TextButtonConfig configTextButton2 = {
            .text = "Calc I",
            .callback = btnamps_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 87,
            .height = 34,
            .radius = 10,
            .backgroundColor = CFK_COLOR14,
            .textColor = CFK_BLACK
        };

    btnamps.setup(configTextButton2);
    TextButtonConfig configTextButton3 = {
            .text = "Calc W",
            .callback = btnwatts_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 87,
            .height = 34,
            .radius = 10,
            .backgroundColor = CFK_COLOR14,
            .textColor = CFK_BLACK
        };

    btnwatts.setup(configTextButton3);
    myDisplay.setTextButton(arrayTextButton,qtdTextButton);


}

void calculateVolts(){
    if((radiog1 == ID_OHMS && radiog2 == ID_AMPS) || (radiog1 == ID_AMPS && radiog2 == ID_OHMS)){
        float results = resistenceValue * ampsValue;
        txtresult.setPrefix("Result:");
        txtresult.setSuffix(" V");
        txtresult.setTextFloat(results, 2);
    }else if((radiog1 == ID_OHMS && radiog2 == ID_WATTS) || (radiog1 == ID_WATTS && radiog2 == ID_OHMS)){
        float results = sqrt(wattsValue * resistenceValue);
        txtresult.setPrefix("Result:");
        txtresult.setSuffix(" V");
        txtresult.setTextFloat(results, 2);
    }else if((radiog1 == ID_AMPS && radiog2 == ID_WATTS) || (radiog1 == ID_WATTS && radiog2 == ID_AMPS)){
        float results = wattsValue / ampsValue;
        txtresult.setPrefix("Result:");
        txtresult.setSuffix(" V");
        txtresult.setTextFloat(results, 2);
    }else{
        txtresult.setPrefix("");
        txtresult.setSuffix("");
        txtresult.setText("Invalid combination");
    }
}

void calculateOhms(){
    if((radiog1 == ID_VOLTS && radiog2 == ID_AMPS) || (radiog1 == ID_AMPS && radiog2 == ID_VOLTS)){
        float results = voltsValue / ampsValue;
        txtresult.setPrefix("Result:");
        txtresult.setSuffix(" Ohms");
        txtresult.setTextFloat(results, 2);
    }else if((radiog1 == ID_VOLTS && radiog2 == ID_WATTS) || (radiog1 == ID_WATTS && radiog2 == ID_VOLTS)){
        float results = voltsValue * voltsValue / wattsValue;
        txtresult.setPrefix("Result:");
        txtresult.setSuffix(" Ohms");
        txtresult.setTextFloat(results, 2);
    }else if((radiog1 == ID_AMPS && radiog2 == ID_WATTS) || (radiog1 == ID_WATTS && radiog2 == ID_AMPS)){
        float results = wattsValue / (ampsValue * ampsValue);
        txtresult.setPrefix("Result:");
        txtresult.setSuffix(" Ohms");
        txtresult.setTextFloat(results, 2);
    }else{
        txtresult.setPrefix("");
        txtresult.setSuffix("");
        txtresult.setText("Invalid combination");
    }
}


void calculateAmps(){
    if((radiog1 == ID_VOLTS && radiog2 == ID_OHMS) || (radiog1 == ID_OHMS && radiog2 == ID_VOLTS)){
        float results = voltsValue / resistenceValue;
        txtresult.setPrefix("Result:");
        txtresult.setSuffix(" Amps");
        txtresult.setTextFloat(results, 2);
    }else if((radiog1 == ID_VOLTS && radiog2 == ID_WATTS) || (radiog1 == ID_WATTS && radiog2 == ID_VOLTS)){
        float results = wattsValue / voltsValue;
        txtresult.setPrefix("Result:");
        txtresult.setSuffix(" Amps");
        txtresult.setTextFloat(results, 2);
    }else if((radiog1 == ID_OHMS && radiog2 == ID_WATTS) || (radiog1 == ID_WATTS && radiog2 == ID_OHMS)){
        float results = sqrt(wattsValue / resistenceValue);
        txtresult.setPrefix("Result:");
        txtresult.setSuffix(" Amps");
        txtresult.setTextFloat(results, 2);
    }else{
        txtresult.setPrefix("");
        txtresult.setSuffix("");
        txtresult.setText("Invalid combination");
    }
}

void calculateWatts(){
    if((radiog1 == ID_VOLTS && radiog2 == ID_OHMS) || (radiog1 == ID_OHMS && radiog2 == ID_VOLTS)){
        float results = voltsValue * voltsValue / resistenceValue;
        txtresult.setPrefix("Result:");
        txtresult.setSuffix(" Watts");
        txtresult.setTextFloat(results, 2);
    }
    else if((radiog1 == ID_VOLTS && radiog2 == ID_AMPS) || (radiog1 == ID_AMPS && radiog2 == ID_VOLTS)){
        float results = voltsValue * ampsValue;
        txtresult.setPrefix("Result:");
        txtresult.setSuffix(" Watts");
        txtresult.setTextFloat(results, 2);
    }
    else if((radiog1 == ID_OHMS && radiog2 == ID_AMPS) || (radiog1 == ID_AMPS && radiog2 == ID_OHMS)){
        float results = ampsValue * ampsValue * resistenceValue;
        txtresult.setPrefix("Result:");
        txtresult.setSuffix(" Watts");
        txtresult.setTextFloat(results, 2);
    }
    else{
        txtresult.setPrefix("");
        txtresult.setSuffix("");
        txtresult.setText("Invalid combination");
    }
}



// This function is a callback of this element inputvolts.
// You dont need call it. Make sure it is as short and quick as possible.
void inputvolts_cb(){

    voltsValue = inputvolts.getValue();
}
// This function is a callback of this element inputohms.
// You dont need call it. Make sure it is as short and quick as possible.
void inputohms_cb(){

    resistenceValue = inputohms.getValue();
}
// This function is a callback of this element inputamps.
// You dont need call it. Make sure it is as short and quick as possible.
void inputamps_cb(){

    ampsValue = inputamps.getValue();
}
// This function is a callback of this element inputwatts.
// You dont need call it. Make sure it is as short and quick as possible.
void inputwatts_cb(){

    wattsValue = inputwatts.getValue();
}
// This function is a callback of radio buttons of group 1.
// You dont need call it. Make sure it is as short and quick as possible.
void callbackOfGroup_grupo1(){

    radiog1 = grupo1.getSelected();
}
// This function is a callback of radio buttons of group 2.
// You dont need call it. Make sure it is as short and quick as possible.
void callbackOfGroup_grupo2(){

    radiog2 = grupo2.getSelected();
}
// This function is a callback of this element btnvolts.
// You dont need call it. Make sure it is as short and quick as possible.
void btnvolts_cb(){

    Serial.print("Clicked on: ");Serial.println("btnvolts_cb");
    calculateVolts();
}
// This function is a callback of this element btnohms.
// You dont need call it. Make sure it is as short and quick as possible.
void btnohms_cb(){

    Serial.print("Clicked on: ");Serial.println("btnohms_cb");
    calculateOhms();
}
// This function is a callback of this element btnamps.
// You dont need call it. Make sure it is as short and quick as possible.
void btnamps_cb(){

    Serial.print("Clicked on: ");Serial.println("btnamps_cb");
    calculateAmps();
}
// This function is a callback of this element btnwatts.
// You dont need call it. Make sure it is as short and quick as possible.
void btnwatts_cb(){

    Serial.print("Clicked on: ");Serial.println("btnwatts_cb");
    calculateWatts();
}
